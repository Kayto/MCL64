For the original code and credit visit [https://github.com/MicroCoreLabs/Projects/tree/master](https://github.com/MicroCoreLabs/Projects/tree/master)

# MCL64 Revision Changes Documentation

## Revision 4
**Date:** August 04, 2025
**Original Code Modified by:** kayto@github.com
**Base:** Revision 3 (12/10/2021)

Revision 4 represents my refactoring of the MCL64 codebase to simplify ongoing development.

```
// Revision 4 08/04/2025 by Kayto@github.com
// Refactored code to split out the following files:
// - opcodes.h
// - opcode_dispatch.h
// - basic_rom.h/cpp
// - kernal_rom.h/cpp
// - addressing_modes.h/cpp
// - hardware_config.h/cpp
//
```

### Major Changes from Revision 3

**CODE ORGANISATION**

**Motivation:** The monolithic code structure was becoming difficult to maintain and debug with my ageing brain. Separating opcodes, ROMs, addressing modes, and hardware configuration helped readability and modularity.

**Changes Made:**

* Separated all 6502/6510 opcodes into `opcodes.h` header file
* Created `opcode_dispatch.h` for the instruction dispatch table
* Separated all ROMs into `.h` and `.cpp` header files:
  - `basic_rom.h/cpp` - Commodore BASIC ROM data
  - `kernal_rom.h/cpp` - Commodore KERNAL ROM data
* Extracted addressing modes into separate module:
  - `addressing_modes.h/cpp` - All 6502 addressing mode functions
* Created hardware configuration module:
  - `hardware_config.h/cpp` - Teensy 4.1 pin definitions and GPIO setup
* Maintained all 6502 emulation logic, memory banking, and performance-critical code in main MCL64.ino

**Files Structure:**
```
MCL64.ino              - Main 6502 emulation, banking, bus interface
opcodes.h              - 6502/6510 instruction implementations
opcode_dispatch.h      - Instruction dispatch table
basic_rom.h/cpp        - Commodore BASIC ROM
kernal_rom.h/cpp       - Commodore KERNAL ROM  
addressing_modes.h/cpp - 6502 addressing mode functions
hardware_config.h/cpp  - Teensy 4.1 pin assignments and setup
```

### Technical Notes

**Preserved in Main File:**
- All memory banking macros (Page_128_159, etc.)
- 6502 stack handling (register_sp_fixed)
- SPEEDUP timing control
- All CPU state variables and registers
- Performance-critical GPIO operations
- Serial communication setup

**Modularized Components:**
- Pin assignments moved to hardware_config.h
- GPIO initialization moved to hardware_config.cpp
- Addressing mode functions extracted to addressing_modes.cpp
- ROM data separated into dedicated modules

