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
   get the next word instead. Same for target
                       t\s  op rg pc st
                       op | 0  1  2  3
                       rg | 4  5  6  7
                       pc | 8  9  A  B
                       st | C  D  E  F
                       0 0000
                       1 0001
                       2 0010
                       3 0011
                       4 0100
                       5 0101
                       6 0110
                       7 0111
                       8 1000
                       9 1001
                       a 1010
                       b 1011
                       c 1100
                       d 1101
                       e 1110
                       f 1111
*/
#define SOURCE (      OP & S_MASK    ? \
                 &reg[OP & S_MASK]   : \
                 &ram[PC + 1]          )
#define TARGET (       OP & T_MSK2     ?\
                 &reg[(OP & T_MSK2)/4] :\
                  &ram[PC + 1]          )

/* enums for the goto jump table */
enum { nul=0x00, add=0x10, sub=0x20, mul=0x30, /* 0 1 2 3 */
       div=0x40, mod=0x50, inc=0x60, dec=0x70, /* 4 5 6 7 */
       and=0x80,  or=0x90, xor=0xa0, cmp=0xb0, /* 8 9 a b */
       beq=0xc0, biz=0xd0, lur=0xe0, ret=0xf0  /* c d e f */
};

/* Status flag masks */
enum {
    zr=0x01, /* zero */
    lt=0x02, /* less than */
    eq=0x04, /* equal */
    gt=0x08, /* greater than */
    cr=0x10, /* carry (overflow) */
    ng=0x20, /* negative */
    _x=0x40, /* unused... */
    _y=0x80  /* unused... */
};

int main(int argc, char ** argv){    
    u8 reg[4] = {0, 0, 0, 0};
    u8 *s=0x00, *t=0x00, ram[256];
    s32 c;
    FILE *fp;
    struct timespec nap;

    /* jump table for gotos */
    void * jmps[16] = {&&nul, &&add, &&sub, &&mul,
                       &&div, &&mod, &&inc, &&dec,
                       &&and, &&or,  &&xor, &&cmp,
                       &&beq, &&biz, &&lur, &&ret
    };

    /* exits if we don't have arguments */
    if(argc < 2) return 1;

    /* exits if we fail reading the file */
    fp = fopen(argv[1], "r");
    if (!fp) return 2;

    /* zeros the ram */
    PC=0;do{ram[PC]=0;PC++;}while(PC);

    /**  125.000.000 for   8hz
     **    3.906.250 for 256hz
    nap.tv_nsec = (long) 3906250;
    **/
    nap.tv_sec = 0;
    nap.tv_nsec = (long) 125000000;

    /* reads file to ram */
    while((c = fgetc(fp)) != EOF){
        printf("%02x ", c);
        ram[PC] = c;
        PC++;
        if(!(PC%16)) puts("");
        /*nanosleep(&nap, NULL);*/
    };  puts("");
    PC=255;
    goto cycle;

    dump_ram:
        PC=0;
        do{
            printf("%2x ", ram[PC]);
            PC++;
            if(!(PC%16))puts("");
        }while(PC != 255);

    cycle:
        PC++;
        OP = ram[PC];
        s = SOURCE;
        t = TARGET;
        nanosleep(&nap, NULL);
        printf("OP=%02x, RG=%02x, PC=%02x, ST=%02x, %02x.%02x ",
                OP,     RG,     PC,     ST,         *s,*t);
        /* dynamic jump */
        goto *(jmps[(OP&O_MASK)>>4]);

    add:
        puts("add'ing");
        *t += *s;
        goto cycle;

    sub:
        puts("sub'bing");
        *t -= *s;
        goto cycle;

    mul:
        puts("mul'ing");
        *t *= *s;
        goto cycle;

    div:
        puts("div'ing");
        *t /= *s;
        goto cycle;

    mod:
        puts("mod'ing");
        *t %= *s;
        goto cycle;

    inc:
        puts("inc'ing");
        (*t)++;
        goto cycle;

    dec:
        puts("dec'ing");
        (*t)--;
        goto cycle;

    and:
        puts("and'ing");
        *t &= *s;
        goto cycle;

    or:
        puts("or'ing");
        *t |= *s;
        goto cycle;

    xor:
        puts("xor'ing");
        *t ^= *s;
        goto cycle;

    lur: /* load/unload registers (LOAD/STORE) */
        puts("lur'ing");
        *t = *s;
        goto cycle;

    cmp:
        puts("cmp'ring");
        (SOURCE == TARGET) ? (ST |= eq) : (ST ^= eq);
        goto cycle;

    biz:
        puts("biz'ing");
        goto cycle;
    nul:
        puts("nul'ling");
        goto cycle;
    beq:
        puts("beq'ing");
        goto cycle;

    ret:
        puts("ret'urning");
        return 0;
    goto dump_ram;
}
