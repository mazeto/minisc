
## MISC - Minimal Instruction Set Computing

    0.3v   data
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

    nul add sub mul  |  0 1 2 3
    div mod inc dec  |  4 5 6 7
    and or  xor cmp  |  8 9 A B
    beq biz lur ...  |  C D E F

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
    zer sml eql gtr
    neg car skp int

The skp flag tells the MISC cpu that the next byte should not be interpreted as a instruction, but as data.

Giving each state its own bit on the status register is a waste of space, because 1 byte can hold up to 256 different combinations of values and no operation would turn on all the logic bits on (negative, carry, zero, smaller, equal and greater), so the MISC CPU stores the status of the last operation on the higher nibble of the status register and gives the programmer the lower nibble to use as storage, which can store up to 16 different values!  But nothing prevents you from using the whole status register as storage.

Here's a list of all the operators and which status they may set on the status high nibble. The values marked with a question mark need to be checked.

    nul -> none
    add -> equal | carry
    sub -> equal | negative
    mul -> equal | carry
    div -> equal
    mod -> zero
    inc -> zero (overflow or carry)
    dec -> zero (underflow or negative)
    and -> zero | equal | greater? | smaller?
    or  -> zero | equal | greater? | smaller?
    xor -> zero | equal? | greater? | smaller?
    lur -> none
    cmp -> ...
    biz
    beq

### Macros

The MISC CPU don't have a jmp instruction, But who needs it? you can load the desired address directly into the PC register.

    jmp $AD = ldr PC
              $AD

