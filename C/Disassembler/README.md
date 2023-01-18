# Disassembler

Usage: `disassemble <start address> <length> <adl mode>`

An eZ80 disassembler for MOS

### Parameters

Parameters can be specified in hexadecimal or decimal. Prefix the number with an & for hexadecimal.

- `start address`: Where to start the disassembly from
- `length`: Number of bytes to disassemble
- `adl mode`: Disassembly mode (optional, defaults to 1)

### ADL mode

When set to 1 it will default to disassembling 24-bit Z80 code. If set to 0, it will disassemble within a 64K segment, and will display any address references relative to that segment.

### Display

The disassembly is spread across 4 columns:

- Address
- Opcode bytes: The instruction in hexadecimal
- Opcode chars: The opcode bytes displayed in ASCII, handy to check if you are disassembling data
- Mnemonics The disassembled instruction

All values are displayed in hexadecimal

### Considerations

The disassembler correctly decodes all Z80 and eZ80 instructions, yet is missing the addressing modes offered by the eZ80, to be added in a future release.

### Compiling

- The paths in the link files (Debug.linkcmd and Release.linkcmd) need to be modified to reflect where the tools are located on your hard drive before this will compile.