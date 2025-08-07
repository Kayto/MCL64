# Ted Fried's MCL64 MicroCore Labs Projects (Kayto Revisions)

For the original code and credit visit [https://github.com/MicroCoreLabs/Projects/tree/master](https://github.com/MicroCoreLabs/Projects/tree/master)

Ted Fried's blog which has some details on these projects: [www.MicroCoreLabs.Wordpress.com](www.MicroCoreLabs.Wordpress.com)

Ted Fried's YouTube Channel with some videos of the stuff in action: [www.youtube.com/channel/UC9B3TaEUon-araO2j7tp9jg/videos](www.youtube.com/channel/UC9B3TaEUon-araO2j7tp9jg/videos)

Boards:

MCL64 - MOS 6510 emulator which runs on a Teensy 4.1 and can be used as a drop-in replacement in the Commodore 64

# MCL64 Kayto Revisions
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
