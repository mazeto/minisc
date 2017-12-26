
## MISC - Minimal Instruction Set Computing

    .3v   data
      | ||||||||
      ##########
      # miscpu #
      ##########
      | ||||||||
    gnd   addr

The MISC architecture is a fictional Turing Machine, that ships with an emulator written in C. A MISC CPU have:

- 8 data pins.
- 8 address pins.
- one pin for .3 Volts input.
- ground pin. 

It can address up to 256 bytes, with no bank switching. The MISC CPU don't have I/O ports, but it can interface with external devices via shared RAM. Although thread-locking wasn't implemented yet.

### Instruction Layout

    o    s t
    0000.0000

    o = opcode
    t = target
    s = source

The following table shows the encoding of the high nibble for each instruction.

    add sub mul div  |  0 1 2 3
    mod inc dec lur  |  4 5 6 7
    and  or xor cmp  |  8 9 A B
    biz bis biq big  |  C D E F

### Registers

    op rg pc st

    op = instruction
    rg = accumulator, register
    pc = program counter
    st = status

The MISC architecture gives you full access to the status and program counter registers, both as source and target. You can manually clear the status registers, jump by loading the address on the program counter,

The following table shows how the instruction is encoded for the target and source lower nibble.

    t\s  op rg pc st
    op | 0  1  2  3
    rg | 4  5  6  7
    pc | 8  9  A  B
    st | C  D  E  F


### Status bits
    zer  sml  eql  gtr
    neg  car  skp? int?

The skp flag tells the MISC cpu that the next byte should not be interpreted as a instruction, but as data. The 'int' status flag sets the OP register to 0xf0, making the CPU jump to the last "page". But I'm not sure if they're really necessary yet. So let they be placeholders.

Here's a list of all the operators and which bits they may set on the status register. The values marked with a question mark need to be checked.

    add -> zero  | equal | carry
    sub -> zero  | equal | negative
    mul -> zero  | equal | carry
    div -> equal
    mod -> zero
    inc -> zero (overflow or carry)
    dec -> zero (underflow or negative)
    lur -> none
    and -> zero | equal | greater? | smaller?
    or  -> zero | equal | greater? | smaller?
    xor -> zero | equal? | greater? | smaller?
    cmp -> smaller | equal | greater
    biz -> none
    bis -> none
    biq -> none
    big -> none

### Macros

The MISC CPU don't have a jmp instruction, But who needs it? you can load the desired address directly into the PC register.

    jmp $AD = ldr PC
              $AD

