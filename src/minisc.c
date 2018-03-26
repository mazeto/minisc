#include <stdio.h>
#include <inttypes.h>
#include <time.h>

#define u8  uint8_t
#define s32 int32_t

#define O_MASK 0xF0  /* 1111.0000 */
#define S_MASK 0x03  /* 0000.0011 */
#define T_MASK 0x0C  /* 0000.1100 */
#define T_MSK2 0x0F  /* 0000.1111 */

#define OP (reg[0])
#define RG (reg[1])
#define PC (reg[2])
#define ST (reg[3])

/* If the source is the source, then
   get the next word instead. Same for target */
#define SOURCE (       OP & S_MASK    ? \
                  &reg[OP & S_MASK]   : \
                  &ram[PC + 1]          )
#define TARGET (       OP & T_MSK2     ?\
                 &reg[(OP & T_MSK2)/4] :\
                  &ram[PC + 1]          )

/* enums for the goto jump table */
enum { add, sub, mul, div, /* 0 1 2 3 */
       mod, inc, dec, lur, /* 4 5 6 7 */
       and,  or, xor, cmp, /* 8 9 a b */
       biz, bis, biq, big  /* c d e f */
};

/* Status flag masks */
enum {
    zr=0x01, /* zero            */
    lt=0x02, /* less than       */
    eq=0x04, /* equal           */
    gt=0x08, /* greater than    */
    cr=0x10, /* carry, overflow */
    ng=0x20, /* negative        */
    _x=0x40, /* unused...       */
    _y=0x80  /* unused...       */
};

int main(int argc, char ** argv){    
    u8 reg[4] = {0, 0, 0, 0};
    u8 *s=0, *t=0, _t=0, ram[256];
    s32 c;
    FILE *fp;
    struct timespec nap;
    /**  125.000.000 for   8hz
     **    3.906.250 for 256hz
    nap.tv_nsec = (long) 3906250;
    **/
    nap.tv_sec = 0;
    nap.tv_nsec = (long) 125000000;

    /* jump table for gotos */
    void * jmps[16] = {&&add, &&sub, &&mul, &&div,
                       &&mod, &&inc, &&dec, &&lur,
                       &&and, &&or,  &&xor, &&cmp,
                       &&biz, &&bis, &&biq, &&big
    };

    /* exits if we don't have arguments */
    if(argc < 2) return 1;

    /* exits if we fail reading the file */
    fp = fopen(argv[1], "r");
    if (!fp) return 2;

    /* zeros the ram */
    PC=0;do{ram[PC]=0;PC++;}while(PC);

    /* reads file to ram */
    while((c = fgetc(fp)) != EOF){
        ram[PC] = c;
        printf("%02x ", ram[PC]);
        PC++;
        if(!(PC%16))puts("");
        /*nanosleep(&nap, NULL);*/
    };  puts("");

    PC=255;
    cycle:
        PC++;
        OP = ram[PC];
        s = SOURCE;
        t = TARGET;
        _t = *t; /* set the latch target register */
        printf("OP=%02x, RG=%02x, PC=%02x, ST=%02x, s=%02x, t=%02x, ",
                    OP,      RG,      PC,      ST,   *s,  *t);
        nanosleep(&nap, NULL);
        /* dynamic jump */
        goto *(jmps[(OP&O_MASK)>>4]);

    add:
        puts("add'ing");
        *t += *s;
        if(!*t)      ST |= zr;
        if(*t == _t) ST |= eq;
        if(*t < _t)  ST |= cr;
        goto cycle;

    sub:
        puts("sub'ing");
        *t -= *s;
        if (!*t)      ST |= zr;
        if (*t == _t) ST |= eq;
        if(*t > _t)   ST |= ng;
        goto cycle;

    mul:
        puts("mul'ing");
        *t *= *s;
        if (!*t)     ST |= zr;
        if (*t < _t) ST |= cr;
        goto cycle;

    div:
        puts("div'ing");
        /* avoid dividing by zero */
        if (*s != 0) *t /= *s;
        else ST |= zr;
        goto cycle;

    mod:
        puts("mod'ing");
        /* avoid dividing by zero */
        if (*s != 0) *t %= *s;
        else ST |= zr; /* ? */
        goto cycle;

    inc:
        puts("inc'ing");
        (*t)++;
        if (!*t)     ST |= cr;
        if (*t == 0) ST |= (zr & cr);
        goto cycle;

    dec:
        puts("dec'ing");
        (*t)--;
        if (!*t)       ST |= zr;
        if (*t == 255) ST |= (zr & ng);
        goto cycle;

    lur: /* load/unload registers (LOAD/STORE) */
        puts("lur'ing");
        *t = *s;
        goto cycle;

    and:
        puts("and'ing");
        *t &= *s;
        if (!*t) ST |= zr;
        if (*t < _t) ST |= lt;
        if (*t > _t) ST |= gt;
        if (*t == _t) ST |= eq;
        else ST ^= eq;
        goto cycle;

    or:
        puts(" or'ing");
        *t |= *s;
        if (!*t) ST |= zr;
        if (*t < _t) ST |= lt;
        if (*t > _t) ST |= gt;
        if (*t == _t) ST |= eq;
        else ST ^= eq;
        goto cycle;

    xor:
        puts("xor'ing");
        *t ^= *s;
        if (!*t) ST |= zr;
        if (*t < _t) ST |= lt;
        if (*t > _t) ST |= gt;
        if (*t == _t) ST |= eq;
        else ST ^= eq;
        goto cycle;

    cmp:
        puts("cmp'ing");
        if (*t < *s) ST |= lt;
        if (*t > *s) ST |= gt;
        if (*t == *s) ST |= eq;
        else ST ^= eq;
        goto cycle;

    biz:
        puts("biz'ing");
        /* OP+1 to get the next word,
         * -1 'cause the 1st thing the cycle does
         * is increment the OP */
        if (ST & zr) OP = ram[OP+1]-1;
        goto cycle;

    bis: /* branch if smaller */
        puts("bis'ing");
        if (ST & lt) OP = ram[OP+1]-1;
        goto cycle;

    biq: /* branch if eQual */
        puts("biq'ing");
        if (ST & eq) OP = ram[OP+1]-1;
        goto cycle;

    big:
        puts("big'ing");
        if (ST & gt) OP = ram[OP+1]-1;
        goto cycle;

    return 0; /* unreachable */
}
