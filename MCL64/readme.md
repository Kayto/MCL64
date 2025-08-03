For the original code and credit visit [https://github.com/MicroCoreLabs/Projects/tree/master](https://github.com/MicroCoreLabs/Projects/tree/master)

# MCL64 Revision Changes Documentation

## Revision 4
**Date:** August 03, 2025
**Original Code Modified by:** kayto@github.com
**Base:** Revision 3 (12/10/2021)

Revision 4 represents my refactoring of the MCL64 codebase to simplify ongoing development.

### Major Changes from Revision 3

**CODE ORGANISATION**

**Motivation:** The monolithic code structure was becoming difficult to maintain and debug with my ageing brain. Separating opcodes and ROMS helped readability.

**Changes Made:**

* Separated all 6502/6510 opcodes into `opcodes.h` header file
* Created `opcode_dispatch.h` for the instruction dispatch table
* Seperated all ROMS into `.h`and `.cpp` header files

```
// Revision 4 08/03/2025 by Kayto@github.com
// Refactored code to split out the following files:
// - opcodes.cpp/h
// - opcode\_dispatch.cpp/h
// - basic\_rom.c/h
// - kernal\_rom.c/h
//
```