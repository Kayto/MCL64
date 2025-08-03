
//  File Name   :  MCL64_final_noacc.ino
//  Used on     :  Teensy 4.1 as drop-in 6510 CPU replacement
//  Author      :  Ted Fried, MicroCore Labs
//  Creation    :  3/12/2021
//
//   Description:
//   ============
//   
//  MOS 6510 emulator with direct hardware bus interface for Commodore 64.
//  This version focuses on maximum cartridge compatibility with cycle-accurate
//  timing and complete 6510 port register emulation.
//
//------------------------------------------------------------------------
//
// Modification History:
// =====================
//
// Revision 1 3/12/2021
// Initial revision
//
// Revision 2 11/28/2021
// Improved undocumented opcodes
//
// Revision 3 12/10/2021
// Made optimiations for acceleration and UART control
//
// Revision 4 08/03/2025 by Kayto@github.com
// Refactored code to split out the following files:
// - opcodes.cpp/h
// - opcode_dispatch.cpp/h
// - basic_rom.c/h
// - kernal_rom.c/h
// 
// ========================================================================
// Revision 5 WIP by kayto@github.com
// focus on hardware compatibility -EF3 and freezer cartridge support.
// See Revision 5 readme.md for detailed documentation of developing changes.
// ========================================================================
//
//------------------------------------------------------------------------
//
// Copyright (c) 2021 Ted Fried
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//------------------------------------------------------------------------

#include <stdint.h>
#include "opcodes.h"
#include "opcode_dispatch.h"

// ============================================================================
// TEENSY 4.1 PIN ASSIGNMENTS - C64 Bus Interface Mapping
// ============================================================================
// This section maps Teensy 4.1 GPIO pins to C64 CPU socket signals
// The mapping is designed for level-shifted/buffered interface boards

// CONTROL SIGNALS
#define PIN_CLK0            24  // PHI2 clock input from C64 (1MHz system clock)
#define PIN_READY_n         26  // READY input (active low) - allows wait states
#define PIN_IRQ             25  // IRQ input (maskable interrupt request)
#define PIN_NMI             41  // NMI input (non-maskable interrupt)
#define PIN_RESET           40  // RESET input (system reset signal)
#define PIN_RDWR_n          12  // R/W output (read/write control to bus)

// 6510 PORT REGISTER OUTPUTS (Memory Banking Control)
// These pins drive the C64 PLA (Programmable Logic Array) for memory mapping
#define PIN_P0              22  // LORAM - Controls BASIC ROM banking
#define PIN_P1              13  // HIRAM - Controls KERNAL ROM banking
#define PIN_P2              39  // CHAREN - Controls Character ROM/I/O banking

// ADDRESS BUS OUTPUTS (16-bit address to C64 motherboard)
// Address bits are spread across multiple Teensy GPIO ports for optimal routing                 
#define PIN_ADDR0           27  // Address bit 0 (LSB)
#define PIN_ADDR1           38  // Address bit 1
#define PIN_ADDR2           28  // Address bit 2
#define PIN_ADDR3           37  // Address bit 3
#define PIN_ADDR4           29  // Address bit 4
#define PIN_ADDR5           36  // Address bit 5
#define PIN_ADDR6           30  // Address bit 6
#define PIN_ADDR7           35  // Address bit 7
#define PIN_ADDR8           31  // Address bit 8
#define PIN_ADDR9           34  // Address bit 9
#define PIN_ADDR10          32  // Address bit 10
#define PIN_ADDR11          33  // Address bit 11
#define PIN_ADDR12          1   // Address bit 12
#define PIN_ADDR13          0   // Address bit 13
#define PIN_ADDR14          2   // Address bit 14
#define PIN_ADDR15          23  // Address bit 15 (MSB)

// DATA BUS INPUTS (8-bit data from C64 to Teensy)
// These pins read data from the C64 bus during read cycles        
#define PIN_DATAIN0         14  // Data bus bit 0 (LSB)
#define PIN_DATAIN1         15  // Data bus bit 1
#define PIN_DATAIN2         16  // Data bus bit 2
#define PIN_DATAIN3         17  // Data bus bit 3
#define PIN_DATAIN4         18  // Data bus bit 4
#define PIN_DATAIN5         19  // Data bus bit 5
#define PIN_DATAIN6         20  // Data bus bit 6
#define PIN_DATAIN7         21  // Data bus bit 7 (MSB)

// DATA BUS OUTPUTS (8-bit data from Teensy to C64)
// These pins drive data onto the C64 bus during write cycles        
#define PIN_DATAOUT0        11  // Data output bit 0 (LSB)
#define PIN_DATAOUT1        10  // Data output bit 1
#define PIN_DATAOUT2        9   // Data output bit 2
#define PIN_DATAOUT3        8   // Data output bit 3
#define PIN_DATAOUT4        7   // Data output bit 4
#define PIN_DATAOUT5        6   // Data output bit 5
#define PIN_DATAOUT6        5   // Data output bit 6
#define PIN_DATAOUT7        4   // Data output bit 7 (MSB)
#define PIN_DATAOUT_OE_n    3   // Data bus output enable (active low tri-state control) 


// ============================================================================
// MEMORY MAPPING DEFINITIONS - C64 Address Space Layout
// ============================================================================
// These definitions help identify which memory regions are being accessed
// The C64 has a complex memory map with overlapping ROM/RAM/I/O regions

// Memory page classification macros (for potential future use)
// Page numbers refer to 256-byte pages ($0000-$00FF = page 0, etc.)
#define Page_128_159  ( (current_address >= 0x8000) && (current_address <= 0x9FFF) ) ? 0x1 : 0x0  // Pages 128-159: BASIC ROM area
#define Page_160_191  ( (current_address >= 0xA000) && (current_address <= 0xBFFF) ) ? 0x1 : 0x0  // Pages 160-191: RAM/ROM area  
#define Page_224_255  ( (current_address >= 0xE000) && (current_address <= 0xFFFF) ) ? 0x1 : 0x0  // Pages 224-255: KERNAL ROM area

// Memory banking mode calculation
// Combines the fixed bits (0x18) with the lower 3 bits of the 6510 port register
// This value would be used by PLA logic for memory mapping decisions
#define bank_mode  ( 0x18 | (current_p&0x7)  )

// Stack pointer address calculation
// The 6502/6510 stack is always located in page 1 ($0100-$01FF)
// This macro converts the 8-bit stack pointer to a full 16-bit address
#define register_sp_fixed  (0x0100 | register_sp)

// ============================================================================
// GLOBAL VARIABLES - 6510 CPU State and Bus Interface
// ============================================================================

// 6510 Port Register ($0001) - Controls memory banking and cassette
// Bit 0: LORAM - Controls BASIC ROM visibility (0=visible, 1=hidden)
// Bit 1: HIRAM - Controls KERNAL ROM visibility (0=visible, 1=hidden) 
// Bit 2: CHAREN - Controls CHAR ROM/I/O visibility (0=CHAR ROM, 1=I/O)
// Bit 3: Cassette data output (software controlled)
// Bit 4: Cassette sense (always reads as 1 - we simulate this)
// Bit 5: Cassette motor control (software controlled)
// Bit 6: Cassette write enable (software controlled)  
// Bit 7: Cassette sense input (software controlled)
uint8_t   current_p=0x37;        // Default: all control bits high for cartridge compatibility

// 6510 CPU Registers
uint8_t   register_flags=0x34;   // Status flags: I=1 (IRQ disabled), bit5=1 (always set)
uint8_t   register_a=0;          // Accumulator register
uint8_t   register_x=0;          // X index register  
uint8_t   register_y=0;          // Y index register
uint8_t   register_sp=0xFD;      // Stack pointer (reset value)
uint16_t  register_pc=0;         // Program counter

// Instruction Processing State
uint8_t   next_instruction;      // Current opcode being executed
uint8_t   assert_sync=0;         // SYNC signal state (high during opcode fetch)
uint8_t   ea_data=0;             // Data fetched from effective address
uint16_t  current_address=0;     // Address currently on the bus
uint16_t  effective_address=0;   // Calculated effective address for instruction

// Bus Interface State - Direct GPIO Readings
uint8_t   direct_datain=0;       // Data bus input (sampled from GPIO)
uint8_t   direct_reset=0;        // RESET line state
uint8_t   direct_ready_n=0;      // READY line state (active low)
uint8_t   direct_irq=0;          // IRQ line state  
uint8_t   direct_nmi=0;          // NMI line state
uint8_t   nmi_n_old=1;           // Previous NMI state for edge detection

// Utility Variables
uint8_t   internal_memory_range=0; // Memory range classification flag
uint8_t   global_temp=0;         // General purpose temporary variable

// ------------------------------------------------------------------------------


// ============================================================================
// SETUP FUNCTION - Initialize Teensy 4.1 Hardware Interface
// ============================================================================
// Configure all GPIO pins for 6510 CPU bus interface to C64 motherboard
void setup() {
  // ----------------------------------------
  // INPUT PINS - Signals from C64 to Teensy
  // ----------------------------------------
  pinMode(PIN_CLK0,        INPUT);   // PHI2 clock from C64 (pin 24)
  pinMode(PIN_RESET,       INPUT);   // RESET signal from C64 (pin 40) 
  pinMode(PIN_READY_n,     INPUT);   // READY signal for wait states (pin 26)
  pinMode(PIN_IRQ,         INPUT);   // IRQ interrupt request (pin 25)
  pinMode(PIN_NMI,         INPUT);   // NMI non-maskable interrupt (pin 41)
  
  // Data bus input pins (pins 14-21)
  pinMode(PIN_DATAIN0,     INPUT);   // Data bus bit 0
  pinMode(PIN_DATAIN1,     INPUT);   // Data bus bit 1
  pinMode(PIN_DATAIN2,     INPUT);   // Data bus bit 2
  pinMode(PIN_DATAIN3,     INPUT);   // Data bus bit 3
  pinMode(PIN_DATAIN4,     INPUT);   // Data bus bit 4
  pinMode(PIN_DATAIN5,     INPUT);   // Data bus bit 5
  pinMode(PIN_DATAIN6,     INPUT);   // Data bus bit 6
  pinMode(PIN_DATAIN7,     INPUT);   // Data bus bit 7
  
  // ----------------------------------------  
  // OUTPUT PINS - Signals from Teensy to C64
  // ----------------------------------------
  pinMode(PIN_RDWR_n,      OUTPUT);  // Read/Write control (pin 12)
  
  // 6510 port register control pins (memory banking)
  pinMode(PIN_P0,          OUTPUT);  // LORAM - BASIC ROM control (pin 22)
  pinMode(PIN_P1,          OUTPUT);  // HIRAM - KERNAL ROM control (pin 13)
  pinMode(PIN_P2,          OUTPUT);  // CHAREN - CHAR ROM/I/O control (pin 39)
  
  // Address bus output pins (pins 0-2, 23, 27-38)                               
  pinMode(PIN_ADDR0,       OUTPUT);  // Address bit 0 (pin 27)
  pinMode(PIN_ADDR1,       OUTPUT);  // Address bit 1 (pin 38)
  pinMode(PIN_ADDR2,       OUTPUT);  // Address bit 2 (pin 28)
  pinMode(PIN_ADDR3,       OUTPUT);  // Address bit 3 (pin 37)
  pinMode(PIN_ADDR4,       OUTPUT);  // Address bit 4 (pin 29)
  pinMode(PIN_ADDR5,       OUTPUT);  // Address bit 5 (pin 36)
  pinMode(PIN_ADDR6,       OUTPUT);  // Address bit 6 (pin 30)
  pinMode(PIN_ADDR7,       OUTPUT);  // Address bit 7 (pin 35)
  pinMode(PIN_ADDR8,       OUTPUT);  // Address bit 8 (pin 31)
  pinMode(PIN_ADDR9,       OUTPUT);  // Address bit 9 (pin 34)
  pinMode(PIN_ADDR10,      OUTPUT);  // Address bit 10 (pin 32)
  pinMode(PIN_ADDR11,      OUTPUT);  // Address bit 11 (pin 33)
  pinMode(PIN_ADDR12,      OUTPUT);  // Address bit 12 (pin 1)
  pinMode(PIN_ADDR13,      OUTPUT);  // Address bit 13 (pin 0)
  pinMode(PIN_ADDR14,      OUTPUT);  // Address bit 14 (pin 2)
  pinMode(PIN_ADDR15,      OUTPUT);  // Address bit 15 (pin 23)
  
  // Data bus output pins (pins 4-11)                              
  pinMode(PIN_DATAOUT0,    OUTPUT);  // Data output bit 0 (pin 11)
  pinMode(PIN_DATAOUT1,    OUTPUT);  // Data output bit 1 (pin 10)
  pinMode(PIN_DATAOUT2,    OUTPUT);  // Data output bit 2 (pin 9)
  pinMode(PIN_DATAOUT3,    OUTPUT);  // Data output bit 3 (pin 8)
  pinMode(PIN_DATAOUT4,    OUTPUT);  // Data output bit 4 (pin 7)
  pinMode(PIN_DATAOUT5,    OUTPUT);  // Data output bit 5 (pin 6)
  pinMode(PIN_DATAOUT6,    OUTPUT);  // Data output bit 6 (pin 5)
  pinMode(PIN_DATAOUT7,    OUTPUT);  // Data output bit 7 (pin 4)
  pinMode(PIN_DATAOUT_OE_n, OUTPUT); // Data bus output enable (pin 3)

  // ----------------------------------------
  // INITIALIZE OUTPUT PIN STATES
  // ----------------------------------------
  digitalWriteFast(PIN_DATAOUT_OE_n,  0x1 ); // Tri-state data bus (disabled)
  digitalWriteFast(PIN_RDWR_n,        0x1 ); // Set to read mode initially
  
  // Set 6510 port pins for maximum cartridge compatibility
  digitalWriteFast(PIN_P0, 0x1 );            // LORAM = 1 (BASIC ROM off)
  digitalWriteFast(PIN_P1, 0x1 );            // HIRAM = 1 (KERNAL ROM off) 
  digitalWriteFast(PIN_P2, 0x1 );            // CHAREN = 1 (I/O space visible)
  
  // Initialize 6510 port register with proper default value
  // $37 = 00110111 binary = bits 5-7 high (cassette), bits 3-4 high (always), bits 0-2 high (banking)
  current_p = 0x37;  // All bits set to default 6510 power-on state for EF3 compatibility
}


// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------
//
// Begin 6502 Bus Interface Unit 
//
// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------

// ============================================================================
// 6502/6510 BUS INTERFACE UNIT - Hardware Timing and GPIO Control
// ============================================================================
// This section implements the low-level bus interface between the Teensy 4.1
// and the C64 motherboard. It handles PHI2 synchronization, address/data bus
// control, and GPIO register manipulation for maximum performance.

// -------------------------------------------------
// PHI2 Clock Synchronization - Rising Edge Wait with Signal Sampling
// -------------------------------------------------
// This function waits for the PHI2 clock rising edge and simultaneously samples
// all input signals (data bus, control lines) using direct GPIO register access
// for maximum speed and deterministic timing.
inline void wait_for_CLK_rising_edge() {
  register uint32_t GPIO6_data=0;      // Main GPIO register snapshot
  register uint32_t GPIO6_data_d1=0;   // Alternative sampling method (currently unused)
  uint32_t   d10, d2, d3, d4, d5, d76; // Data bus bit extraction variables

    // Wait for PHI2 to go LOW (falling edge completion)
    while (((GPIO6_DR >> 12) & 0x1)!=0) {}            // Teensy 4.1 Pin-24  GPIO6_DR[12]     CLK
    
    // Wait for PHI2 rising edge and capture GPIO state at the transition
    // This method works for most C64 operations (VIC-20 and Apple-II+ non-DRAM ranges)
    while (((GPIO6_DR >> 12) & 0x1)==0) {GPIO6_data=GPIO6_DR;}
    
    // Alternative sampling method for systems requiring precise data setup timing
    // Currently commented out - would be needed for Apple-II+ DRAM read operations
    //do {  GPIO6_data_d1=GPIO6_DR;   } while (((GPIO6_data_d1 >> 12) & 0x1)==0);
    //GPIO6_data=GPIO6_data_d1;
    
    // Extract data bus bits from GPIO6 register
    // Data bus pins are scattered across GPIO6 register bits for optimal PCB routing
    d10             = (GPIO6_data&0x000C0000) >> 18;  // Teensy 4.1 Pin-14/15  GPIO6_DR[19:18]  D1:D0
    d2              = (GPIO6_data&0x00800000) >> 21;  // Teensy 4.1 Pin-16     GPIO6_DR[23]     D2
    d3              = (GPIO6_data&0x00400000) >> 19;  // Teensy 4.1 Pin-17     GPIO6_DR[22]     D3
    d4              = (GPIO6_data&0x00020000) >> 13;  // Teensy 4.1 Pin-18     GPIO6_DR[17]     D4
    d5              = (GPIO6_data&0x00010000) >> 11;  // Teensy 4.1 Pin-19     GPIO6_DR[16]     D5
    d76             = (GPIO6_data&0x0C000000) >> 20;  // Teensy 4.1 Pin-20/21  GPIO6_DR[27:26]  D7:D6
    
    // Extract control signal states from GPIO6 register
    direct_irq      = (GPIO6_data&0x00002000) >> 13;  // Teensy 4.1 Pin-25     GPIO6_DR[13]     IRQ
    direct_ready_n  = (GPIO6_data&0x40000000) >> 30;  // Teensy 4.1 Pin-26     GPIO6_DR[30]     READY
    direct_reset    = (GPIO6_data&0x00100000) >> 20;  // Teensy 4.1 Pin-40     GPIO6_DR[20]     RESET
    direct_nmi      = (GPIO6_data&0x00200000) >> 21;  // Teensy 4.1 Pin-41     GPIO6_DR[21]     NMI
    
    // Reconstruct complete 8-bit data bus value from extracted bits
    direct_datain = d76 | d5 | d4 | d3 | d2 | d10;
    
    return; 
}


// -------------------------------------------------
// PHI2 Clock Synchronization - Falling Edge Wait
// -------------------------------------------------
// This function waits for the PHI2 clock falling edge
// Used during write cycles to ensure proper data bus timing
inline void wait_for_CLK_falling_edge() {
  // Wait for PHI2 to go HIGH (if currently low)
  while (((GPIO6_DR >> 12) & 0x1)==0) {}   // Teensy 4.1 Pin-24  GPIO6_DR[12]  CLK
  // Wait for PHI2 to go LOW (falling edge)
  while (((GPIO6_DR >> 12) & 0x1)!=0) {}
  return; 
}


// -------------------------------------------------
// Address Bus Output - High-Speed GPIO Register Manipulation
// -------------------------------------------------
// This function outputs a 16-bit address to the C64 bus using direct GPIO
// register writes for maximum speed. Address bits are distributed across
// multiple GPIO ports (GPIO6, GPIO7, GPIO8, GPIO9) for optimal PCB routing.
inline void send_address(uint32_t local_address) {
  register uint32_t writeback_data=0;  // Temporary register for GPIO manipulation
  
    // GPIO6 Register - Address bits 15, 13, 12, 1, 0
    // Read current register, clear target bits, set new values
    writeback_data = (0x6DFFFFF3 & GPIO6_DR);   // Read and mask out bits we'll modify
    writeback_data = writeback_data | (local_address & 0x8000)<<10 ;  // A15: TEENSY_PIN23 → GPIO6_DR[25]
    writeback_data = writeback_data | (local_address & 0x2000)>>10 ;  // A13: TEENSY_PIN0  → GPIO6_DR[3]
    writeback_data = writeback_data | (local_address & 0x1000)>>10 ;  // A12: TEENSY_PIN1  → GPIO6_DR[2]
    writeback_data = writeback_data | (local_address & 0x0002)<<27 ;  // A1:  TEENSY_PIN38 → GPIO6_DR[28]
    GPIO6_DR       = writeback_data | (local_address & 0x0001)<<31 ;  // A0:  TEENSY_PIN27 → GPIO6_DR[31]
    
    // GPIO7 Register - Address bits 10, 9, 7, 5, 3
    writeback_data = (0xCFF3EFFF & GPIO7_DR);   // Read and mask out bits we'll modify
    writeback_data = writeback_data | (local_address & 0x0400)<<2  ;  // A10: TEENSY_PIN32 → GPIO7_DR[12]
    writeback_data = writeback_data | (local_address & 0x0200)<<20 ;  // A9:  TEENSY_PIN34 → GPIO7_DR[29]
    writeback_data = writeback_data | (local_address & 0x0080)<<21 ;  // A7:  TEENSY_PIN35 → GPIO7_DR[28]
    writeback_data = writeback_data | (local_address & 0x0020)<<13 ;  // A5:  TEENSY_PIN36 → GPIO7_DR[18]
    GPIO7_DR       = writeback_data | (local_address & 0x0008)<<16 ;  // A3:  TEENSY_PIN37 → GPIO7_DR[19]
                
    // GPIO8 Register - Address bits 8, 6, 2
    writeback_data = (0xFF3BFFFF & GPIO8_DR);   // Read and mask out bits we'll modify
    writeback_data = writeback_data | (local_address & 0x0100)<<14 ;  // A8:  TEENSY_PIN31 → GPIO8_DR[22]
    writeback_data = writeback_data | (local_address & 0x0040)<<17 ;  // A6:  TEENSY_PIN30 → GPIO8_DR[23]
    GPIO8_DR       = writeback_data | (local_address & 0x0004)<<16 ;  // A2:  TEENSY_PIN28 → GPIO8_DR[18]
    
    // GPIO9 Register - Address bits 14, 11, 4
    writeback_data = (0x7FFFFF6F & GPIO9_DR);   // Read and mask out bits we'll modify
    writeback_data = writeback_data | (local_address & 0x4000)>>10 ;  // A14: TEENSY_PIN2  → GPIO9_DR[4]
    writeback_data = writeback_data | (local_address & 0x0800)>>4  ;  // A11: TEENSY_PIN33 → GPIO9_DR[7]
    GPIO9_DR       = writeback_data | (local_address & 0x0010)<<27 ;  // A4:  TEENSY_PIN29 → GPIO9_DR[31]
    
    return;
}

        
// -------------------------------------------------
// Initialize Read Cycle - Address Setup Phase
// -------------------------------------------------
// This function begins a memory read cycle by setting up the address bus
// and R/W control signal. The actual data read occurs later with finish_read_byte()
inline void start_read(uint32_t local_address) {
  
  current_address = local_address;         // Store address for 6510 port register handling
  digitalWriteFast(PIN_RDWR_n,  0x1);      // Set R/W to READ mode (HIGH = read)
  send_address(local_address);             // Output address to C64 bus
  return;
}

// -------------------------------------------------
// Complete a read cycle and return the data
// -------------------------------------------------
// This function completes a read cycle that was started with start_read()
// It handles the special case of reading the 6510 port register at $0001
inline uint8_t finish_read_byte() {  
  
  // Wait for PHI2 rising edge and READY signal
  // READY allows external devices to insert wait states
  do {  wait_for_CLK_rising_edge();  }  while (direct_ready_n == 0x1);  // Delay a clock cycle until ready is active 
                      
  // Handle 6510 port register read at address $0001
  // The 6510 port register is internal to the CPU, not on the external bus
  // Bit 4 is always read as 1 (cassette sense line - we simulate this)
  if (current_address==0x1) return (current_p|0x10); 
  else return direct_datain;  // Return data from external bus for all other addresses
}
  
// -------------------------------------------------
// Complete read cycle with address setup and data return
// -------------------------------------------------
// This function performs a complete memory read cycle:
// 1. Sets up the address on the bus
// 2. Waits for PHI2 and READY
// 3. Returns the data, handling 6510 port register specially
inline uint8_t read_byte(uint16_t local_address) {  
  
  current_address = local_address;
  start_read(local_address);
  do {  wait_for_CLK_rising_edge();  }  while (direct_ready_n == 0x1);  // Delay a clock cycle until ready is active 

  // Special handling for 6510 port register at $0001
  // This register is internal to the CPU and controls memory banking and cassette
  if (current_address==0x1) return (current_p|0x10); 
  else return direct_datain;                  
} 


// -------------------------------------------------
// Complete write cycle with address and data
// -------------------------------------------------
// This function performs a complete memory write cycle:
// 1. Sets up address and R/W signal
// 2. Prepares data on output pins
// 3. Handles 6510 port register writes specially
// 4. Enables data bus drivers during PHI2 low
// 5. Completes the write cycle
inline void write_byte(uint16_t local_address , uint8_t local_write_data) {
  
  digitalWriteFast(PIN_RDWR_n,  0x0);  // Set R/W to WRITE mode (LOW = write)
  send_address(local_address);
       
  // Set up data on output pins (bus drivers are still disabled)
  // This prepares the data before enabling the drivers
  digitalWriteFast(PIN_DATAOUT0,  (local_write_data & 0x01)    );
  digitalWriteFast(PIN_DATAOUT1,  (local_write_data & 0x02)>>1 ); 
  digitalWriteFast(PIN_DATAOUT2,  (local_write_data & 0x04)>>2 ); 
  digitalWriteFast(PIN_DATAOUT3,  (local_write_data & 0x08)>>3 ); 
  digitalWriteFast(PIN_DATAOUT4,  (local_write_data & 0x10)>>4 ); 
  digitalWriteFast(PIN_DATAOUT5,  (local_write_data & 0x20)>>5 ); 
  digitalWriteFast(PIN_DATAOUT6,  (local_write_data & 0x40)>>6 ); 
  digitalWriteFast(PIN_DATAOUT7,  (local_write_data & 0x80)>>7 ); 
     
  // Special handling for 6510 port register at $0001
  // Writing to this address updates both the internal register and drives physical pins
  if (local_address==0x1) {  
    current_p = local_write_data;  // Update internal port register
    // Drive the memory banking control pins (bits 0-2 control PLA)
    digitalWriteFast(PIN_P0,  (local_write_data & 0x01) );       // LORAM (bit 0)
    digitalWriteFast(PIN_P1,  (local_write_data & 0x02) >> 1 );  // HIRAM (bit 1) 
    digitalWriteFast(PIN_P2,  (local_write_data & 0x04) >> 2 );  // CHAREN (bit 2)
    // Note: Bits 3-7 are cassette control and don't drive physical pins
  }
     
  // Complete the write cycle with proper PHI2 timing
  // Enable data bus drivers during PHI2 low phase, disable during high phase
  wait_for_CLK_falling_edge();        // Wait for PHI2 to go low
  digitalWriteFast(PIN_DATAOUT_OE_n,  0x0 );  // Enable data bus outputs (LOW = enabled)
       
  wait_for_CLK_rising_edge();         // Wait for PHI2 to go high  
  digitalWriteFast(PIN_DATAOUT_OE_n,  0x1 );  // Disable data bus outputs (HIGH = tri-state)
            
  return;
}
 
// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------
//
// End 6502 Bus Interface Unit
//
// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------


// ============================================================================
// CPU UTILITY FUNCTIONS - Stack Operations and Flag Management
// ============================================================================

// -------------------------------------------------
// Stack Push Operation
// -------------------------------------------------
// Pushes a byte onto the 6502/6510 stack and decrements stack pointer
// The stack grows downward from $01FF toward $0100
void push(uint8_t push_data) {    
    write_byte(register_sp_fixed, push_data);  // Write data to current stack location
    register_sp = register_sp - 1;             // Decrement stack pointer (stack grows down)
    return;
}

// -------------------------------------------------
// Stack Pop Operation  
// -------------------------------------------------
// Increments stack pointer and reads a byte from the 6502/6510 stack
// Returns the popped value
uint8_t pop() {
    uint8_t temp=0;
    register_sp = register_sp + 1;        // Increment stack pointer first
    temp = read_byte(register_sp_fixed);  // Read data from new stack location
    return temp;
}

// -------------------------------------------------
// CPU Status Flag Calculation - Negative and Zero Flags
// -------------------------------------------------
// Updates the N (negative) and Z (zero) flags based on operation result
// This is called after most arithmetic and logic operations
void Calc_Flags_NEGATIVE_ZERO(uint8_t local_data) {
    
    // Set/clear NEGATIVE flag (bit 7) based on MSB of result
    if (0x80&local_data)   register_flags = register_flags | 0x80;  // Set N flag (result is negative)
    else                   register_flags = register_flags & 0x7F;  // Clear N flag (result is positive)
    
    // Set/clear ZERO flag (bit 1) based on result value
    if (local_data==0)     register_flags = register_flags | 0x02;  // Set Z flag (result is zero)
    else                   register_flags = register_flags & 0xFD;  // Clear Z flag (result is non-zero)
    
    return;
}

// -------------------------------------------------
// 16-bit Sign Extension Utility
// -------------------------------------------------
// Extends an 8-bit signed value to 16-bit for address calculations
// Used in relative addressing modes (branches) and signed arithmetic
uint16_t Sign_Extend16(uint16_t reg_data)  {
    if ((reg_data&0x0080)== 0x0080)   { return (reg_data | 0xFF00); }  // Negative: extend with 1s
    else                              { return (reg_data & 0x00FF); }  // Positive: extend with 0s
}

// -------------------------------------------------
// Next Instruction Fetch Setup
// -------------------------------------------------
// Increments PC and begins fetching the next opcode
// Sets SYNC signal to indicate opcode fetch cycle
void Begin_Fetch_Next_Opcode()  {
    register_pc++;                   // Move to next instruction
    assert_sync=1;                   // Assert SYNC signal (opcode fetch)
    start_read(register_pc);         // Begin reading next opcode
    return;
}

// ============================================================================
// 6502/6510 ADDRESSING MODES - Memory Access Patterns
// ============================================================================
// These functions implement the various addressing modes of the 6502/6510 CPU
// Each mode calculates effective addresses and fetches data according to the
// specific instruction encoding and timing requirements.
//
// NOTE: This is the STRIPPED VERSION with NO ACCELERATION FEATURES
// All memory accesses go through the real C64 hardware bus for maximum compatibility

// -------------------------------------------------
// Immediate Addressing Mode
// -------------------------------------------------
// Data is the next byte after the opcode (PC+1)
// Used for: LDA #$20, CMP #$40, etc.
uint8_t Fetch_Immediate()  {
    register_pc++;                           // Move PC to operand byte
    ea_data = read_byte(register_pc);        // Read immediate value
    return ea_data;
}

// -------------------------------------------------  
// Zero Page Addressing Mode
// -------------------------------------------------
// Address is in zero page ($0000-$00FF), specified by single byte operand
// Used for: LDA $20, STA $80, etc.
uint8_t Fetch_ZeroPage()  { 
    effective_address = Fetch_Immediate();   // Get zero page address (8-bit)
    ea_data = read_byte(effective_address);  // Read data from zero page
    return ea_data;
}

// -------------------------------------------------
// Zero Page,X Addressing Mode  
// -------------------------------------------------
// Zero page address + X register, wraps within zero page
// Used for: LDA $20,X, STA $80,X, etc.
uint8_t Fetch_ZeroPage_X()  {   
    uint16_t bal;
    bal = Fetch_Immediate();                 // Get base zero page address
    read_byte(register_pc+1);                // Extra read cycle (hardware timing)
    effective_address = (0x00FF & (bal + register_x)); // Add X, wrap in zero page
    ea_data = read_byte(effective_address);  // Read final data
    return ea_data;
}

// -------------------------------------------------
// Zero Page,Y Addressing Mode
// -------------------------------------------------  
// Zero page address + Y register, wraps within zero page
// Used for: LDX $20,Y, STX $80,Y, etc.
uint8_t Fetch_ZeroPage_Y()  {   
    uint16_t bal;
    bal = Fetch_Immediate();                 // Get base zero page address
    read_byte(register_pc+1);                // Extra read cycle (hardware timing)
    effective_address = (0x00FF & (bal + register_y)); // Add Y, wrap in zero page
    ea_data = read_byte(effective_address);  // Read final data
    return ea_data;
}

// -------------------------------------------------
// Absolute Addressing Mode - Address Calculation Only
// -------------------------------------------------
// Calculates 16-bit absolute address from two-byte operand (little-endian)
// Used internally by other addressing modes
uint16_t Calculate_Absolute()  { 
    uint16_t adl, adh;
    
    adl = Fetch_Immediate();                 // Get address low byte
    adh = Fetch_Immediate()<<8;              // Get address high byte  
    effective_address = adl + adh;           // Combine to 16-bit address
    return effective_address;
}

// -------------------------------------------------
// Absolute Addressing Mode - With Data Fetch
// -------------------------------------------------  
// 16-bit address specified by two-byte operand, then fetch data
// Used for: LDA $1234, STA $4000, etc.
uint8_t Fetch_Absolute()  { 
    uint16_t adl, adh;
    
    adl = Fetch_Immediate();                 // Get address low byte
    adh = Fetch_Immediate()<<8;              // Get address high byte
    effective_address = adl + adh;           // Combine to 16-bit address
    ea_data = read_byte(effective_address);  // Read data from absolute address
    return ea_data;
}

uint8_t Fetch_Absolute_X(uint8_t page_cross_check)  {
    uint16_t bal, bah;
    
    bal = Fetch_Immediate();
    bah = Fetch_Immediate()<<8;
    effective_address = bah + bal + register_x;
    ea_data = read_byte(effective_address );
    
    if (page_cross_check==1 && (  (0xFF00&effective_address) != (0xFF00&bah) ) ) {  
        ea_data = read_byte(effective_address ); 
    }
    return ea_data;
}

uint8_t Fetch_Absolute_Y(uint8_t page_cross_check)  {
    uint16_t bal, bah;
    
    bal = Fetch_Immediate();
    bah = Fetch_Immediate()<<8;
    effective_address = bah + bal + register_y;
    ea_data = read_byte(effective_address );
    
    if (page_cross_check==1 && (  (0xFF00&effective_address) != (0xFF00&bah) ) ) {  
        ea_data = read_byte(effective_address ); 
    } 
    return ea_data;
}

uint8_t Fetch_Indexed_Indirect_X()  { 
    uint16_t bal;
    uint16_t adl, adh;
    
    bal = Fetch_Immediate() + register_x;
    read_byte(bal);
    adl = read_byte(0xFF&bal);
    adh = read_byte(0xFF&(bal+1)) << 8;
    effective_address = adh + adl ;
    ea_data = read_byte(effective_address);
    return ea_data;
}

uint8_t Fetch_Indexed_Indirect_Y(uint8_t page_cross_check)  {
    uint16_t ial, bah, bal;
    
    ial = Fetch_Immediate();
    bal = read_byte(0xFF&ial);
    bah = read_byte(0xFF&(ial+1)) << 8;
    
    effective_address = bah + bal + register_y;
    ea_data = read_byte(effective_address);
    
    if (page_cross_check==1 && ((0xFF00&effective_address) != (0xFF00&bah)) ) {  
        ea_data = read_byte(effective_address); 
    }
    return ea_data;
}


void Write_ZeroPage(uint8_t local_data)  {
    effective_address = Fetch_Immediate();
    write_byte(effective_address , local_data);
    return;
}

void Write_Absolute(uint8_t local_data)  {
    effective_address = Fetch_Immediate();
    effective_address = (Fetch_Immediate() << 8) + effective_address;
    write_byte(effective_address , local_data );
    return;
}

void Write_ZeroPage_X(uint8_t local_data)  {
    effective_address = Fetch_Immediate();
    read_byte(effective_address);
    write_byte( (0x00FF&(effective_address + register_x)) , local_data );
    return;
}

void Write_ZeroPage_Y(uint8_t local_data)  {
    effective_address = Fetch_Immediate();
    read_byte(effective_address);
    write_byte( (0x00FF&(effective_address + register_y)) , local_data );
    return;
}

void Write_Absolute_X(uint8_t local_data)  {
    uint16_t bal,bah;

    bal = Fetch_Immediate();
    bah = Fetch_Immediate()<<8;
    effective_address = bal + bah + register_x; 
    read_byte(effective_address);
    write_byte(effective_address , local_data );  
    return;
}

void Write_Absolute_Y(uint8_t local_data)  {
    uint16_t bal,bah;

    bal = Fetch_Immediate();
    bah = Fetch_Immediate()<<8;
    effective_address = bal + bah + register_y;
    read_byte(effective_address);

    if ( (0xFF00&effective_address) != (0xFF00&bah) ) { 
        read_byte(effective_address);
    }
    write_byte(effective_address , local_data );  
    return;
}

void Write_Indexed_Indirect_X(uint8_t local_data)  {
    uint16_t bal;
    uint16_t adl, adh;

    bal = Fetch_Immediate();
    read_byte(bal);
    adl = read_byte(0xFF&(bal+register_x));
    adh = read_byte(0xFF&(bal+register_x+1)) << 8;
    effective_address = adh + adl;
    write_byte(effective_address , local_data );
    return;
}

void Write_Indexed_Indirect_Y(uint8_t local_data)  {  
    uint16_t ial;
    uint16_t bal, bah;

    ial = Fetch_Immediate();
    bal = read_byte(ial);
    bah = read_byte(ial+1)<<8;
    effective_address = bah + bal + register_y;
    read_byte(effective_address);
    write_byte(effective_address , local_data );
    return;
}

void Double_WriteBack(uint8_t local_data)  {  
    write_byte(effective_address , ea_data);
    write_byte(effective_address , local_data);
    return;
}


// -------------------------------------------------
// Reset sequence for the 6510 CPU
// -------------------------------------------------
// This function implements the complete 6510 reset sequence including:
// 1. Hardware reset line synchronization
// 2. Bus state initialization (R/W, data bus tri-state)
// 3. CPU register initialization (flags only, A/X/Y/SP commented for testing)
// 4. 6510 port register setup for memory banking and cassette control
// 5. Dummy reads that mimic real 6510 internal reset operations
// 6. Reset vector fetch and PC initialization
// 7. First instruction fetch setup
void reset_sequence() {
    uint16_t temp1, temp2;
    
    // PHASE 1: Wait for hardware RESET line to be deasserted
    // C64 RESET is active LOW, so we wait while it's LOW (==0)
    // When RESET goes HIGH (!= 0), the reset sequence can begin
    while (digitalReadFast(PIN_RESET)!=0) {}                        // Stay here until RESET deasserts
    
    // PHASE 2: Initialize bus control signals for safe operation
    // Ensure the CPU is in read mode and data bus outputs are disabled
    // This prevents bus contention during the reset sequence
    digitalWriteFast(PIN_RDWR_n, 0x1);           // Set R/W to READ mode (HIGH = read)
    digitalWriteFast(PIN_DATAOUT_OE_n, 0x1);     // Tri-state data bus outputs (HIGH = disabled)
    
    // PHASE 3: Initialize CPU registers to 6510 reset state
    // Note: A, X, Y, SP initialization commented out for EF3 compatibility testing
    // The 6510 datasheet specifies these reset values:
    register_a = 0x00;        // Accumulator cleared
    register_x = 0x00;        // X register cleared  
    register_y = 0x00;        // Y register cleared
    register_sp = 0xFD;       // Stack pointer set to $FD (standard 6510 reset value)
    register_flags = 0x34;    // Status flags: I=1 (IRQ disabled), bit5=1 (always set), others=0
    
    // PHASE 4: Configure 6510 port register for maximum cartridge compatibility
    // The 6510 has an internal 8-bit port at address $0001 that controls:
    // Bits 0-2: Memory banking (LORAM/HIRAM/CHAREN) - drive physical PLA pins
    // Bits 3-4: Always read as 1 (unused, cassette sense - not implemented)
    // Bits 5-7: Cassette control (motor, write, sense) - software only
    digitalWriteFast(PIN_P0, 0x1);  // LORAM = 1 (BASIC ROM off, allows cartridge at $8000-$9FFF)
    digitalWriteFast(PIN_P1, 0x1);  // HIRAM = 1 (KERNAL ROM off, allows cartridge at $E000-$FFFF)
    digitalWriteFast(PIN_P2, 0x1);  // CHAREN = 1 (Character ROM off, I/O visible at $D000-$DFFF)
    current_p = 0x37;               // Internal port register = $37 (bits 5-7=1, bits 3-4=1, bits 0-2=111)
    
    // PHASE 5: Perform dummy reads that simulate real 6510 reset sequence behavior
    // The real 6510 CPU performs several internal read cycles during reset
    // These reads are visible on the bus and some cartridges may depend on this timing
    // We replicate this behavior for maximum hardware compatibility
    temp1 = read_byte(register_pc);             // Read from current PC (random/undefined address)
    temp1 = read_byte(register_pc+1);           // Read from PC+1 (random/undefined address)
    temp1 = read_byte(register_sp_fixed);       // Read from stack pointer address ($01FD typically)
    temp1 = read_byte(register_sp_fixed-1);     // Read from SP-1 ($01FC typically)
    temp1 = read_byte(register_sp_fixed-2);     // Read from SP-2 ($01FB typically)
    
    // PHASE 6: Fetch the reset vector and initialize program counter
    // The 6510 always reads the reset vector from $FFFC/$FFFD after reset
    // This tells the CPU where to start executing code
    temp1 = read_byte(0xFFFC);      // Get low byte of reset vector
    temp2 = read_byte(0xFFFD);      // Get high byte of reset vector
    register_pc = (temp2 << 8) | temp1;  // Combine into 16-bit PC address

    // PHASE 7: Begin instruction execution at reset vector address
    // Set up the CPU to fetch the first instruction from the reset vector location
    assert_sync = 1;                // Mark that we're starting a new instruction (SYNC high)
    start_read(register_pc);        // Begin reading first opcode at reset address
    return;
}


// -------------------------------------------------
// NMI Interrupt Processing
// -------------------------------------------------
void nmi_handler() {
    uint16_t temp1, temp2;
    wait_for_CLK_rising_edge();                                     // Begin processing on next CLK edge
    
    register_flags = register_flags | 0x20;                         // Set the flag[5]          
    register_flags = register_flags & 0xEF;                         // Clear the B flag     
    
    read_byte(register_pc+1);                                       // Fetch PC+1 (Discard)
    push(register_pc>>8);                                           // Push PCH
    push(register_pc);                                              // Push PCL
    push(register_flags);                                           // Push P
    temp1 = read_byte(0xFFFA);                                      // Fetch Vector PCL
    temp2 = read_byte(0xFFFB);                                      // Fetch Vector PCH
                
    register_flags = register_flags | 0x34;                         // Set the I flag and restore the B flag

    register_pc = (temp2<<8) | temp1;           
    assert_sync=1;
    start_read(register_pc);                                        // Fetch first opcode at vector PCH,PCL
    
    return;
}
    

// -------------------------------------------------
// BRK & IRQ Interrupt Processing
// -------------------------------------------------
void irq_handler(uint8_t opcode_is_brk) {
    uint16_t temp1, temp2;
    
    wait_for_CLK_rising_edge();                                     // Begin processing on next CLK edge
                        
    register_flags = register_flags | 0x20;                         // Set the flag[5]          
    if (opcode_is_brk==1) register_flags = register_flags | 0x10;   // Set the B flag
    else                  register_flags = register_flags & 0xEF;   // Clear the B flag
    
    read_byte(register_pc+1);                                       // Fetch PC+1 (Discard)
    push(register_pc>>8);                                           // Push PCH
    push(register_pc);                                              // Push PCL
    push(register_flags);                                           // Push P
    temp1 = read_byte(0xFFFE);                                      // Fetch Vector PCL
    temp2 = read_byte(0xFFFF);                                      // Fetch Vector PCH
                
    register_flags = register_flags | 0x34;                         // Set the I flag and restore the B flag
                
    register_pc = (temp2<<8) | temp1;           
    assert_sync=1;
    start_read(register_pc);                                        // Fetch first opcode at vector PCH,PCL
    
    return;
}

// Define flag macros for easier access
#define flag_i    ((register_flags & 0x04) >> 2)

// ============================================================================
// MAIN EXECUTION LOOP - 6510 CPU Emulation
// ============================================================================
// This is the main CPU execution loop that:
// 1. Performs initial reset sequence
// 2. Continuously fetches and executes instructions
// 3. Handles hardware interrupts (NMI/IRQ)
// 4. Monitors for hardware reset
void loop() {
  
  // Give the Teensy and C64 hardware time to stabilize
  //delay (50);
  //wait_for_CLK_rising_edge();   // Sync to PHI2 clock
  //wait_for_CLK_rising_edge();   // Additional sync cycles  
  //wait_for_CLK_rising_edge();   // for stable operation

  // Perform the complete 6510 reset sequence
  reset_sequence();

  // ============================================================================
  // MAIN CPU EXECUTION LOOP
  // ============================================================================
  while (1) {
      
      // Check for hardware RESET assertion
      // If RESET goes low, immediately restart the reset sequence
      if (direct_reset==1) reset_sequence();
    
    // ========================================================================
    // INTERRUPT PROCESSING - NMI and IRQ Handling
    // ========================================================================
    if (assert_sync) {
        // NMI (Non-Maskable Interrupt) - Edge-triggered detection
        // NMI is triggered on the falling edge (high-to-low transition)
        // We detect this by comparing current state with previous state
        if ((nmi_n_old == 0) && (direct_nmi == 1)) {
            nmi_handler();
        }
    }

    // IRQ (Interrupt Request) - Level-sensitive, maskable by I flag
    // IRQ is active when line is HIGH and I flag in status register is CLEAR
    if ((direct_irq == 0x1) && (flag_i == 0x0)) {
        irq_handler(0x0);  // 0x0 = not a BRK instruction
    }

    // Update NMI state for next cycle's edge detection
    nmi_n_old = direct_nmi;                     

    // ========================================================================
    // INSTRUCTION FETCH AND EXECUTION
    // ========================================================================
    // Complete the instruction fetch that was started by reset_sequence() or previous instruction
    next_instruction = finish_read_byte();   // Get the opcode from the bus
    assert_sync=0;                          // Clear SYNC - no longer fetching opcode
      
    // Execute the fetched opcode using the dispatch table
    // This calls the appropriate function for the specific 6510 instruction
    execute_opcode(next_instruction);

    } 
}
