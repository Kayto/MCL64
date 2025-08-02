For the original code and credit visit [https://github.com/MicroCoreLabs/Projects/tree/master](https://github.com/MicroCoreLabs/Projects/tree/master)

# MCL64 Revision 4 Changes Documentation

**Date:** July 30, 2025
**Original Code Modified by:** kayto@github.com
**Base:** Revision 3 (12/10/2021)

Revision 4 represents my refactoring of the MCL64 codebase with the aim to gain better understanding of compatibility related to real C64 carts, specifically EF3, Action Replay and Supersnapshot freezer cartridges. I am not there yet, but steadily improving seems like a result.

##### Compatibility Notes

- Please note that your situation may vary depending on your hardware and this only represents my apples with apples comparison. Try the original code first as YMMV!


| Cartridge Type | REV4 Status | Original MCL64 REV3 Status |
|----------------|-------------|-------------|
| Diagnostic 4in1 Multi Cart | Works on boot, physical reset fails (1) | Works on boot, physical reset fails (1) |
| Diagnostic Cartridge | Intermittent (2) | Working |
| Dead Test Cart | Not working (8 flashes) | Not working (8 flashes) |
| EF3 KERNAL Replacement | Intermittent | Intermittent (better?) |
| EF3 Freezer | Working RR,SS,AR | RR, AR Working.   SS Not Working |
| Standard Cartridges (SimonsBASIC) | Working | Working |
| Easyflash Cartridge | Working | Working |
| Super Snapshot 5 Cartridge (SS) | Working | Not Working |
| Magic Desk Cartridge | Working | Working |
| Action Replay v6.0 (AR) | Working | Not working |
| EF3 Turrican | Working | Working (minor sound corruption)
| EF3 Prince of Persia | Not Working | Not Working |


* (1) this may be normal behaviour on real hardware.
* (2) TBC - is my board a bit flakey!

### Potential Issues

* All things timings!
* Is my board a bit flakey/inconsistent?

## Major Changes from Revision 3 and general ramblings!

### 1\. ACCELERATION REMOVAL

**Motivation:** To be honest I only removed the acceleration features in Revision 3 to ensure that I didn't get ROM and RAM emulation mixed up with using the real hardware. Additionally to shorten the code for better handling. I initially isolated the ROMS into seperate files which helped but then went this step further. I may introduce this back in, but for now the concentration is on accuracy of the emulation.

**Changes Made:**

* Stripped out all acceleration features and UART control mechanisms
* Removed all speedup modes and fast execution paths
* Eliminated internal memory caching and acceleration lookup tables
* All memory access now goes through the real C64 hardware bus exclusively
* Removed acceleration control commands and status reporting

### 2\. CODE ORGANIZATION

**Motivation:** The monolithic code structure was becoming difficult to maintain and debug with my ageing brain. Separating opcodes helped readability.

**Changes Made:**

* Separated all 6502/6510 opcodes into `opcodes.h` header file
* Created `opcode_dispatch.h` for the instruction dispatch table
* Reorganized code structure and commented.


### 3\. COMPLETE 6510 PORT REGISTER IMPLEMENTATION

**Motivation:** I assume that Cartridges require complete and accurate 6510 port register behavior. The original Revision 3 code handles this but I entered a rabbit hole of coding, to try and ensure this was accurate.

**Implementation:**

#### Memory Banking Control (Bits 0-2)

* **Bit 0 (LORAM):** Controls BASIC ROM banking at $A000-$BFFF
    * 0 = BASIC ROM visible, 1 = RAM visible
    * Drives physical PIN\_P0 to C64 PLA
* **Bit 1 (HIRAM):** Controls KERNAL ROM banking at $E000-$FFFF
    * 0 = KERNAL ROM visible, 1 = RAM visible
    * Drives physical PIN\_P1 to C64 PLA
* **Bit 2 (CHAREN):** Controls Character ROM/I/O banking at $D000-$DFFF
    * 0 = Character ROM visible, 1 = I/O space visible
    * Drives physical PIN\_P2 to C64 PLA

#### Cassette Control Implementation (Bits 3-7)

* **Bit 3:** Cassette data output line (software controlled)
* **Bit 4:** Cassette sense input (always reads as 1 - simulated)
* **Bit 5:** Cassette motor control (software controlled)
* **Bit 6:** Cassette write enable line (software controlled)
* **Bit 7:** Cassette sense input (software controlled)

#### Technical Details

* added power on state initialisation of the register.
* incorporated the full port register (including always-set and cassette bits).
* maintaining port state more explicitly through a port value.
* data direction register - not really, but initialisation assumes always as an output.

### 4\. ENHANCED RESET SEQUENCE

**Motivation:** I must admit this was also a rabbit hole that I am not sure I have emerged from. In an attempt to debug why physical hardware resets were failing to boot cartridge ROMS, I ended up fiddling with the revision 3 reset code to make it "cycle exact". I am not sure part of my minor tweak to Phase 3 adds anything really needed over the original. So I am really stretching the term "enhanced".

**7-Phase Reset Implementation:**

#### Phase 1: Hardware Reset Synchronization

* Wait for RESET line deassertion (active low signal)
* Ensures reset sequence doesn't start during hardware reset

#### Phase 2: Bus State Initialization

* Set R/W to READ mode for safe bus state
* Tri-state data bus outputs to prevent contention
* Initialize control signals for safe operation

#### Phase 3: CPU Register Initialization

* Status register set to $34 (I=1, bit5=1, others=0)
* A/X/Y/SP initialization

#### Phase 4: 6510 Port Register Setup

* Configure PLA control pins 
* Set LORAM/HIRAM/CHAREN to 1 (ROM disabled, cartridge space enabled)
* Initialize internal port register to $37

#### Phase 5: Dummy Read Cycles

* Perform reads that mimic real 6510 reset behavior:
    * Read from current PC (undefined address)
    * Read from PC+1 (undefined address)
    * Read from stack pointer address
    * Read from SP-1 and SP-2
* These cycles are visible on the bus and have assumed that some cartridges depend on them?

#### Phase 6: Reset Vector Fetch

* Read reset vector from $FFFC (low byte) and $FFFD (high byte)
* Initialize program counter with vector address
* Standard 6502/6510 reset vector handling

#### Phase 7: First Instruction Setup

* Set SYNC signal high to indicate opcode fetch
* Begin reading first instruction at reset vector address
* Transition to normal execution mode

**Note:** This revision prioritizes hardware compatibility over performance. All changes were made with the goal of achieving maximum compatibility with the specific cartridges I had problems with.
