For the original code and credit visit [https://github.com/MicroCoreLabs/Projects/tree/master](https://github.com/MicroCoreLabs/Projects/tree/master)

# MCL64 Revision Changes Documentation

## Revision 4
**Date:** August 04, 2025  
**Updated:** August 07, 2025
**Original Code Modified by:** kayto@github.com
**Base:** Revision 3 (12/10/2021)

Revision 4 represents my refactoring of the MCL64 codebase to simplify ongoing development and add compile-time acceleration control.

```
// Revision 4 08/03/2025 by Kayto@github.com
// Refactored code to split out the following files:
// - opcodes.cpp/h
// - opcode_dispatch.cpp/h
// - basic_rom.c/h
// - kernal_rom.c/h
// - addressing_modes.cpp/h
// - hardware_config.cpp/h
// 
// Updated 08/07/2025 Added compile-time acceleration control:
// - ENABLE_ACCELERATION define allows complete removal of acceleration code
// - All acceleration-related code wrapped in #if ENABLE_ACCELERATION blocks
// - When set to 0, forces cycle-accurate mode and removes internal RAM array
// - resulting in improved cartridge compatibility
// 
```
 Compatibility related to real C64 carts, specifically EF3, Action Replay and Supersnapshot freezer cartridges is improved by compile time removal of acceleration.

### Compatibility Notes

- Please note that your situation may vary depending on your hardware and this only represents my comparison. 
- Try the original code first!


| Cartridge Type | REV4 Status | Original MCL64 REV3 Status |
|----------------|-------------|-------------|
| Diagnostic 4in1 Multi Cart | Works on boot, boot after physical reset fails (1) | Works on boot, boot after physical reset fails (1) |
| Diagnostic Cartridge | Working (2) | Working |
| Dead Test Cart | Working (2) | **Not working (8 flashes)** |
| EF3 KERNAL Replacement | Working (2) | Working (2) |
| EF3 Freezer | Working RR,SS,AR | RR, AR Working.   **SS Not Working** |
| Standard Cartridges (SimonsBASIC) | Working | Working |
| Easyflash Cartridge | Working | Working |
| Super Snapshot 5 Cartridge (SS) | Working | **Not Working** |
| Magic Desk Cartridge | Working | Working |
| Action Replay v6.0 (AR) | Working | **Not working** |
| EF3 Turrican | Working | Working **(minor sound/graphic corruption)** |
| EF3 Prince of Persia | **Not Working** | **Not Working** |


* (1) this may be normal behaviour on real hardware.
* (2) TBC - possibly intermittent - is my board a bit flakey!
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

**COMPILE-TIME ACCELERATION CONTROL** *(Added August 07, 2025)*

**Motivation:** Improvement of cycle accuracy and compatibility, especially when using cartridges. 

**Changes Made:**

* Added `ENABLE_ACCELERATION` compile-time define in `MCL64.ino`
* When set to `1`: Full acceleration features available (default behavior)
* When set to `0`: All acceleration code removed from compilation
* Conditional compilation of 64KB internal RAM array
* Conditional compilation of UART mode switching
* All memory access functions fall back to cycle-accurate external operations

**Disabling Acceleration:**

* **Improved Cycle Accuracy:** Eliminates timing variations introduced by internal memory caching?


**Usage:**
```cpp
// For maximum performance (default):
#define ENABLE_ACCELERATION 1

// For maximum compatibility and cycle accuracy:
#define ENABLE_ACCELERATION 0
```

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

