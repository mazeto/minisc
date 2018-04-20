#include <stdio.h>
#include <inttypes.h>
#include <time.h>

#define u8  uint8_t
#define s32 int32_t

#define O_MASK 0xF0  /* 1111.0000 */
#define S_MASK 0x03  /* 0000.0011 */
#define T_MASK 0x0C  /* 0000.1100 */
#define T_MSK2 0x0F  /* 0000.1111 */

#define OP (reg[0]) /* OP code */
#define RG (reg[1]) /* Register (accumulator) */
#define PC (reg[2]) /* Program Counter */
#define ST (reg[3]) /* Status */

/* If source == source, get the next word.
   Same goes for target */
#define SOURCE (       OP & S_MASK    ? \
                  &reg[OP & S_MASK]   : \
                  &ram[PC + 1]          )
#define TARGET (       OP & T_MSK2     ?\
                 &reg[(OP & T_MSK2)/4] :\
                  &ram[PC + 1]          )

#ifdef GCC
  /* enums for the goto jump table */
  enum { add, sub, mul, div, /* 0 1 2 3 */
         mod, inc, dec, lur, /* 4 5 6 7 */
         and,  or, xor, cmp, /* 8 9 a b */
         biz, bis, biq, big  /* c d e f */
  };
#endif

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

  #ifndef GCC
    u8 _op=0;
  #endif

  s32 c;
  FILE *fp;
  struct timespec nap;
  /**  125.000.000 for   8hz
   **    3.906.250 for 256hz
  nap.tv_nsec = (long) 3906250;
  **/
  nap.tv_sec = 0;
  nap.tv_nsec = (long) 125000000;

  #ifdef GCC
    /* jump table for gotos */
    void * jmps[16] = {
      &&add, &&sub, &&mul, &&div,
      &&mod, &&inc, &&dec, &&lur,
      &&and, &&or,  &&xor, &&cmp,
      &&biz, &&bis, &&biq, &&big
    };
  #endif

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
  }; puts("");

  PC=255;
  cycle: /* CPU cycle */
    PC++;
    OP = ram[PC];
    s = SOURCE;
    t = TARGET;
    _t = *t; /* set the latch target register */
    printf("OP=%02x, RG=%02x, PC=%02x, "
           "ST=%02x, s=%02x, t=%02x, ",
            OP,      RG,      PC,
            ST,     *s,     *t);
    nanosleep(&nap, NULL);

    #ifdef GCC
      /* gcc dynamic jump */
      goto *(jmps[(OP&O_MASK)>>4]);
    #endif

    #ifndef GCC
      _op = (OP&O_MASK)>>4;

      /* ternary operator binary search */
      _op < 8 ? ({goto lt8;}) : ({goto ge8;});
      lt8:
        _op < 4 ? ({goto lt4;}) : ({goto ge4;});
        lt4:
          _op < 2 ? ({goto lt2;}) : ({goto ge2;});
          lt2:
            _op ? ({goto sub;}) : ({goto add;});
          ge2:
            _op == 2 ? ({goto mul;}) : ({goto div;});
        ge4:
          _op < 6 ? ({goto lt6;}) : ({goto ge6;});
          lt6:
            _op == 4 ? ({goto mod;}) : ({goto inc;});
          ge6:
            _op == 6 ? ({goto dec;}) : ({goto lur;});
      ge8:
        _op < 12 ? ({goto ltc;}) : ({goto gec;});
        ltc:
          _op < 10 ? ({goto lta;}) : ({goto gea;});
          lta:
            _op == 8 ? ({goto and;}) : ({goto or;});
          gea:
            _op == 10 ? ({goto xor;}) : ({goto cmp;});
        gec:
          _op < 14 ? ({goto lte;}) : ({goto gee;});
          lte:
            _op == 12 ? ({goto biz;}) : ({goto bis;});
          gee:
            _op == 14 ? ({goto biq;}) : ({goto big;});
    #endif

  add: /* add */
    puts("add'ing");
    *t += *s;
    if(!*t)      ST |= zr;
    if(*t == _t) ST |= eq;
    if(*t < _t)  ST |= cr;
    goto cycle;

  sub: /* subtract */
    puts("sub'ing");
    *t -= *s;
    if (!*t)      ST |= zr;
    if (*t == _t) ST |= eq;
    if(*t > _t)   ST |= ng;
    goto cycle;

  mul: /* multiply */
    puts("mul'ing");
    *t *= *s;
    if (!*t)     ST |= zr;
    if (*t < _t) ST |= cr;
    goto cycle;

  div: /* divide */
    puts("div'ing");
    /* avoid dividing by zero */
    if (*s != 0) *t /= *s;
    else ST |= zr;
    goto cycle;

  mod: /* modulus */
    puts("mod'ing");
    /* avoid dividing by zero */
    if (*s != 0) *t %= *s;
    else ST |= zr; /* ? */
    goto cycle;

  inc: /* increment */
    puts("inc'ing");
    (*t)++;
    if (!*t)     ST |= cr;
    if (*t == 0) ST |= (zr & cr);
    goto cycle;

  dec: /* decrement */
    puts("dec'ing");
    (*t)--;
    if (!*t)       ST |= zr;
    if (*t == 255) ST |= (zr & ng);
    goto cycle;

  lur: /* load/unload registers (LOAD/STORE) */
    puts("lur'ing");
    *t = *s;
    goto cycle;

  and: /* and */
    puts("and'ing");
    *t &= *s;
    if (!*t) ST |= zr;
    if (*t < _t) ST |= lt;
    if (*t > _t) ST |= gt;
    if (*t == _t) ST |= eq;
    else ST ^= eq;
    goto cycle;

  or: /* or */
    puts(" or'ing");
    *t |= *s;
    if (!*t) ST |= zr;
    if (*t < _t) ST |= lt;
    if (*t > _t) ST |= gt;
    if (*t == _t) ST |= eq;
    else ST ^= eq;
    goto cycle;

  xor: /* exclusive or */
    puts("xor'ing");
    *t ^= *s;
    if (!*t) ST |= zr;
    if (*t < _t) ST |= lt;
    if (*t > _t) ST |= gt;
    if (*t == _t) ST |= eq;
    else ST ^= eq;
    goto cycle;

  cmp: /* compare */
    puts("cmp'ing");
    if (*t < *s) ST |= lt;
    if (*t > *s) ST |= gt;
    if (*t == *s) ST |= eq;
    else ST ^= eq;
    goto cycle;

  biz: /* branch if zero */
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

  big: /* branch if greater */
    puts("big'ing");
    if (ST & gt) OP = ram[OP+1]-1;
    goto cycle;

  return 0; /* unreachable */
}
