//
//
//  File Name   :  MCL64.c
//  Used on     :  
//  Author      :  Ted Fried, MicroCore Labs
//  Creation    :  3/12/2021
//
//   Description:
//   ============
//   
//  MOS 6510 emulator with bus interface.
//
// The acceleration modes can be changed via the UART from the host.
// Entering a 0,1,2,3 will change the acceleration mode to this value
// and it will be echoed back to the host.
//
// Entering mode 2 or 4 could result in video corruption, but the CPU will still 
// be running.  When returning to mode-0 or mode-1 the video should return to normal.
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
// ========================================================================
// Revision 4 08/03/2025 by Kayto@github.com
// Refactored code to split out the following files:
// - opcodes.cpp/h
// - opcode_dispatch.cpp/h
// - basic_rom.c/h
// - kernal_rom.c/h
// - addressing_modes.cpp/h
// - hardware_config.cpp/h
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
#include "basic_rom.h"
#include "kernal_rom.h"
#include "opcodes.h"
#include "opcode_dispatch.h"
#include "addressing_modes.h"
#include "hardware_config.h"

// Memory page and banking macros 
#define Page_128_159  ( (current_address >= 0x8000) && (current_address <= 0x9FFF) ) ? 0x1 : 0x0 
#define Page_160_191  ( (current_address >= 0xA000) && (current_address <= 0xBFFF) ) ? 0x1 : 0x0 
#define Page_224_255  ( (current_address >= 0xE000) && (current_address <= 0xFFFF) ) ? 0x1 : 0x0 

#define bank_mode  ( 0x18 | (current_p&0x7)  )

// 6502 stack always in Page 1
#define register_sp_fixed  (0x0100 | register_sp)

// This eliminates most of the superfluous fetches and writes of the cycle-accurate 6502
#define SPEEDUP 0


// CPU register for direct reads of the GPIOs 
uint8_t   current_p=0x7;        
uint8_t   register_flags=0x34; 
uint8_t   next_instruction;
uint8_t   internal_memory_range=0;
uint8_t   nmi_n_old=1;
uint8_t   register_a=0;
uint8_t   register_x=0;
uint8_t   register_y=0;
uint8_t   register_sp=0xFF;
uint8_t   direct_datain=0;
uint8_t   direct_reset=0;
uint8_t   direct_ready_n=0;
uint8_t   direct_irq=0;
uint8_t   direct_nmi=0;
uint8_t   assert_sync=0;
uint8_t   global_temp=0;
uint8_t   last_access_internal_RAM=0;
uint8_t   ea_data=0;
uint8_t   mode=0;

uint16_t  register_pc=0;
uint16_t  current_address=0;
uint16_t  effective_address=0;

uint8_t   internal_RAM[65536];
extern const uint8_t BASIC_ROM[0x2000];
extern const uint8_t KERNAL_ROM[0x2000];

int       incomingByte;   

// Setup Teensy 4.1 IO's
//
void setup() {
  setup_teensy_pins();

  digitalWriteFast(PIN_P0, 0x1 ); 
  digitalWriteFast(PIN_P1, 0x1 ); 
  digitalWriteFast(PIN_P2, 0x1 ); 

  Serial.begin(9600);  
}


// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------
//
// Begin 6502 Bus Interface Unit 
//
// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------


// ----------------------------------------------------------
// Address range check
//  Return: 0x0 - All exernal memory accesses
//          0x1 - Reads and writes are cycle accurate using internal memory with writes passing through to motherboard
//          0x2 - Reads accelerated using internal memory and writes are cycle accurate and pass through to motherboard
//          0x3 - All read and write accesses use accelerated internal memory 
// ----------------------------------------------------------
inline uint8_t internal_address_check(uint16_t local_address) {


  if ( (local_address > 0x0001 ) && (local_address <= 0x03FF) ) return mode;            //   Zero-Page up to video 
  if ( (local_address >= 0x0400) && (local_address <= 0x07FF) && mode>1) return 0x1;    //   C64 Video Memory 
  if ( (local_address >= 0x0800) && (local_address <= 0x7FFF) ) return mode;            //   C64 RAM 
  if ( (local_address >= 0x8000) && (local_address <= 0x9FFF) ) return mode;            //   C64 CART_LOW & RAM 
  if ( (local_address >= 0xA000) && (local_address <= 0xBFFF) ) return mode;            //   C64 BASIC ROM & RAM
  if ( (local_address >= 0xC000) && (local_address <= 0xCFFF) ) return mode;            //   C64 RAM
//if ( (local_address >= 0xD000) && (local_address <= 0xDFFF) ) return 0x0;             //   C64 I/O
  if ( (local_address >= 0xE000) && (local_address <= 0xE4FF) ) return mode;            //   C64 KERNAL ROM  
  if ( (local_address >= 0xE500) && (local_address <= 0xFF7F) && mode>1) return 0x1;    //   C64 KERNAL ROM  
  if ( (local_address >= 0xFF80) && (local_address <= 0xFFFF) ) return mode;            //   C64 KERNAL ROM 
 
  return 0x0;
} 


// -------------------------------------------------
// Wait for the CLK1 rising edge and sample signals
// -------------------------------------------------
inline void wait_for_CLK_rising_edge() {
  register uint32_t GPIO6_data=0;
  register uint32_t GPIO6_data_d1=0;
  uint32_t   d10, d2, d3, d4, d5, d76;

    while (((GPIO6_DR >> 12) & 0x1)!=0) {}            // Teensy 4.1 Pin-24  GPIO6_DR[12]     CLK
    
    while (((GPIO6_DR >> 12) & 0x1)==0) {GPIO6_data=GPIO6_DR;}                  // This method is ok for VIC-20 and Apple-II+ non-DRAM ranges 
    
    //do {  GPIO6_data_d1=GPIO6_DR;   } while (((GPIO6_data_d1 >> 12) & 0x1)==0);   // This method needed to support Apple-II+ DRAM read data setup time
    //GPIO6_data=GPIO6_data_d1;
    
    d10             = (GPIO6_data&0x000C0000) >> 18;  // Teensy 4.1 Pin-14  GPIO6_DR[19:18]  D1:D0
    d2              = (GPIO6_data&0x00800000) >> 21;  // Teensy 4.1 Pin-16  GPIO6_DR[23]     D2
    d3              = (GPIO6_data&0x00400000) >> 19;  // Teensy 4.1 Pin-17  GPIO6_DR[22]     D3
    d4              = (GPIO6_data&0x00020000) >> 13;  // Teensy 4.1 Pin-18  GPIO6_DR[17]     D4
    d5              = (GPIO6_data&0x00010000) >> 11;  // Teensy 4.1 Pin-19  GPIO6_DR[16]     D5
    d76             = (GPIO6_data&0x0C000000) >> 20;  // Teensy 4.1 Pin-20  GPIO6_DR[27:26]  D7:D6
    
    direct_irq      = (GPIO6_data&0x00002000) >> 13;  // Teensy 4.1 Pin-25  GPIO6_DR[13]     IRQ
    direct_ready_n  = (GPIO6_data&0x40000000) >> 30;  // Teensy 4.1 Pin-26  GPIO6_DR[30]     READY
    direct_reset    = (GPIO6_data&0x00100000) >> 20;  // Teensy 4.1 Pin-40  GPIO6_DR[20]     RESET
    direct_nmi      = (GPIO6_data&0x00200000) >> 21;  // Teensy 4.1 Pin-41  GPIO6_DR[21]     NMI
    
    direct_datain = d76 | d5 | d4 | d3 | d2 | d10;
    
    return; 
}


// -------------------------------------------------
// Wait for the CLK1 falling edge 
// -------------------------------------------------
inline void wait_for_CLK_falling_edge() {

  while (((GPIO6_DR >> 12) & 0x1)==0) {}   // Teensy 4.1 Pin-24  GPIO6_DR[12]  CLK
  while (((GPIO6_DR >> 12) & 0x1)!=0) {}
  return; 
}


// -------------------------------------------------
// Drive the 6502 Address pins
// -------------------------------------------------
inline void send_address(uint32_t local_address) {
  register uint32_t writeback_data=0;
  
    writeback_data = (0x6DFFFFF3 & GPIO6_DR);   // Read in current GPIOx register value and clear the bits we intend to update
    writeback_data = writeback_data | (local_address & 0x8000)<<10 ;  // 6502_Address[15]   TEENSY_PIN23   GPIO6_DR[25]
    writeback_data = writeback_data | (local_address & 0x2000)>>10 ;  // 6502_Address[13]   TEENSY_PIN0    GPIO6_DR[3]
    writeback_data = writeback_data | (local_address & 0x1000)>>10 ;  // 6502_Address[12]   TEENSY_PIN1    GPIO6_DR[2]
    writeback_data = writeback_data | (local_address & 0x0002)<<27 ;  // 6502_Address[1]    TEENSY_PIN38   GPIO6_DR[28]
    GPIO6_DR       = writeback_data | (local_address & 0x0001)<<31 ;  // 6502_Address[0]    TEENSY_PIN27   GPIO6_DR[31]
    
    writeback_data = (0xCFF3EFFF & GPIO7_DR);   // Read in current GPIOx register value and clear the bits we intend to update
    writeback_data = writeback_data | (local_address & 0x0400)<<2  ;  // 6502_Address[10]   TEENSY_PIN32   GPIO7_DR[12]
    writeback_data = writeback_data | (local_address & 0x0200)<<20 ;  // 6502_Address[9]    TEENSY_PIN34   GPIO7_DR[29]
    writeback_data = writeback_data | (local_address & 0x0080)<<21 ;  // 6502_Address[7]    TEENSY_PIN35   GPIO7_DR[28]
    writeback_data = writeback_data | (local_address & 0x0020)<<13 ;  // 6502_Address[5]    TEENSY_PIN36   GPIO7_DR[18]
    GPIO7_DR       = writeback_data | (local_address & 0x0008)<<16 ;  // 6502_Address[3]    TEENSY_PIN37   GPIO7_DR[19]
                
    writeback_data = (0xFF3BFFFF & GPIO8_DR);   // Read in current GPIOx register value and clear the bits we intend to update
    writeback_data = writeback_data | (local_address & 0x0100)<<14 ;  // 6502_Address[8]    TEENSY_PIN31   GPIO8_DR[22]
    writeback_data = writeback_data | (local_address & 0x0040)<<17 ;  // 6502_Address[6]    TEENSY_PIN30   GPIO8_DR[23]
    GPIO8_DR       = writeback_data | (local_address & 0x0004)<<16 ;  // 6502_Address[2]    TEENSY_PIN28   GPIO8_DR[18]
    
    writeback_data = (0x7FFFFF6F & GPIO9_DR);   // Read in current GPIOx register value and clear the bits we intend to update
    writeback_data = writeback_data | (local_address & 0x4000)>>10 ;  // 6502_Address[14]   TEENSY_PIN2    GPIO9_DR[4]
    writeback_data = writeback_data | (local_address & 0x0800)>>4  ;  // 6502_Address[11]   TEENSY_PIN33   GPIO9_DR[7]
    GPIO9_DR       = writeback_data | (local_address & 0x0010)<<27 ;  // 6502_Address[4]    TEENSY_PIN29   GPIO9_DR[31]
    
    return;
}

        
// -------------------------------------------------
// Send the address for a read cyle
// -------------------------------------------------
inline void start_read(uint32_t local_address) {
  
  current_address = local_address; 
   
    if (internal_address_check(current_address)>0x1)  { 
      //last_access_internal_RAM=1;
    }

    else 
    {
       if (last_access_internal_RAM==1) wait_for_CLK_rising_edge();
       last_access_internal_RAM=0;

        digitalWriteFast(PIN_RDWR_n,  0x1);
        send_address(local_address);
     }
    return;
}



// -------------------------------------------------
// Fetch data from the correct Bank
// -------------------------------------------------
inline uint8_t fetch_byte_from_bank() {
                     
    if (Page_160_191==0x1)  {  if   ((bank_mode&0x3)==0x3)                                  {  return BASIC_ROM[current_address & 0x1FFF];        }
                               else                                                         {  return internal_RAM[current_address];              }  }
    
    if (Page_224_255==0x1)  {  if ( (bank_mode&0x2)==0x2)                                   {  return KERNAL_ROM[current_address & 0x1FFF];       }
                               else                                                         {  return internal_RAM[current_address];              }  }
    
     return internal_RAM[current_address];
}
  

// -------------------------------------------------
// On the rising CLK edge, read in the data
// -------------------------------------------------
inline uint8_t finish_read_byte() {  
  
  if (internal_address_check(current_address)>0x1)  {
    last_access_internal_RAM=1;
    return fetch_byte_from_bank();   
    }         
    else 
    {
       if (last_access_internal_RAM==1) wait_for_CLK_rising_edge();
       last_access_internal_RAM=0;
       
       do {  wait_for_CLK_rising_edge();  }  while (direct_ready_n == 0x1);  // Delay a clock cycle until ready is active 
                      
       if (internal_address_check(current_address)>0x0)  {  return fetch_byte_from_bank();   }
       else                                              {  if (current_address==0x1) return (current_p|0x10); else return direct_datain;                  }
    }
}
  


// -------------------------------------------------
// Full read cycle with address and data read in
// -------------------------------------------------
inline uint8_t read_byte(uint16_t local_address) {  
  
  current_address = local_address;
  
  if (internal_address_check(local_address)>0x1)  {
    last_access_internal_RAM=1;
        return fetch_byte_from_bank(); 
    }
    else 
    {
       if (last_access_internal_RAM==1) wait_for_CLK_rising_edge();
       last_access_internal_RAM=0;
       
       start_read(local_address);
       do {  wait_for_CLK_rising_edge();  }  while (direct_ready_n == 0x1);  // Delay a clock cycle until ready is active 

       if (internal_address_check(current_address)>0x0)  {  return fetch_byte_from_bank();  }
       else                                              {  if (current_address==0x1) return (current_p|0x10); else return direct_datain;                  }
     }
} 


// -------------------------------------------------
// Full write cycle with address and data written
// -------------------------------------------------
inline void write_byte(uint16_t local_address , uint8_t local_write_data) {
  
  // Internal RAM
  //
    if (internal_address_check(local_address)>0x2)  {
    last_access_internal_RAM=1;
    internal_RAM[local_address] = local_write_data;
      //if ( (Page_128_159==0x1)  && ( (EXROM==1 && GAME==0) || ( EXROM==0 && ((bank_mode&0x3)==0x3) ) )) {  } else internal_RAM[local_address] = local_write_data; 
  }
  else 
  {
       if (last_access_internal_RAM==1) wait_for_CLK_rising_edge();
       last_access_internal_RAM=0;
       internal_RAM[local_address] = local_write_data;
      //if ( (Page_128_159==0x1)  && ( (EXROM==1 && GAME==0) || ( EXROM==0 && ((bank_mode&0x3)==0x3) ) )) {  } else internal_RAM[local_address] = local_write_data; 
     
       digitalWriteFast(PIN_RDWR_n,  0x0);
       send_address(local_address);

       
     // Drive the data bus pins from the Teensy to the bus driver which is inactive
     //
       digitalWriteFast(PIN_DATAOUT0,  (local_write_data & 0x01)    );
       digitalWriteFast(PIN_DATAOUT1,  (local_write_data & 0x02)>>1 ); 
       digitalWriteFast(PIN_DATAOUT2,  (local_write_data & 0x04)>>2 ); 
       digitalWriteFast(PIN_DATAOUT3,  (local_write_data & 0x08)>>3 ); 
       digitalWriteFast(PIN_DATAOUT4,  (local_write_data & 0x10)>>4 ); 
       digitalWriteFast(PIN_DATAOUT5,  (local_write_data & 0x20)>>5 ); 
       digitalWriteFast(PIN_DATAOUT6,  (local_write_data & 0x40)>>6 ); 
       digitalWriteFast(PIN_DATAOUT7,  (local_write_data & 0x80)>>7 ); 
     
       if (local_address==0x1) {  
       current_p = local_write_data;
       digitalWriteFast(PIN_P0,  (local_write_data & 0x01) ); 
       digitalWriteFast(PIN_P1,  (local_write_data & 0x02) >> 1 ); 
       digitalWriteFast(PIN_P2,  (local_write_data & 0x04) >> 2 ); 
     }
     
       
       // During the second CLK phase, enable the data bus output drivers
       //
       wait_for_CLK_falling_edge();
       digitalWriteFast(PIN_DATAOUT_OE_n,  0x0 ); 
       
       wait_for_CLK_rising_edge();
       digitalWriteFast(PIN_DATAOUT_OE_n,  0x1 );   
  }            
   return;
}

  
// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------
//
// End 6502 Bus Interface Unit
//
// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------


void push(uint8_t push_data) {    
    write_byte(register_sp_fixed, push_data); 
    register_sp = register_sp - 1;
    return;
}


uint8_t pop() {
    uint8_t temp=0;
    register_sp = register_sp + 1;
    temp = read_byte(register_sp_fixed);
    return temp;
}
    
  
void Calc_Flags_NEGATIVE_ZERO(uint8_t local_data) {
    
    if (0x80&local_data)   register_flags = register_flags | 0x80;              // Set the N flag
    else                   register_flags = register_flags & 0x7F;              // Clear the N flag
    
    if (local_data==0)     register_flags = register_flags | 0x02;              // Set the Z flag
    else                   register_flags = register_flags & 0xFD;              // Clear the Z flag 
    
    return;
}


uint16_t Sign_Extend16(uint16_t reg_data)  {
    if ((reg_data&0x0080)== 0x0080)   { return (reg_data | 0xFF00); } 
    else                              { return (reg_data & 0x00FF); }  
}


void Begin_Fetch_Next_Opcode()  {
    register_pc++;
    assert_sync=1;
    start_read(register_pc);
    return;
}

// -------------------------------------------------
// Reset sequence for the 6502
// -------------------------------------------------
void reset_sequence() {
    uint16_t temp1, temp2;
       
    while (digitalReadFast(PIN_RESET)!=0) {}                        // Stay here until RESET deasserts
            
                
    digitalWriteFast(PIN_RDWR_n,  0x1);         
    digitalWriteFast(PIN_DATAOUT_OE_n,  0x1 );  
    
    digitalWriteFast(PIN_P0, 0x1 ); 
    digitalWriteFast(PIN_P1, 0x1 ); 
    digitalWriteFast(PIN_P2, 0x1 );           
            
            
    temp1 = read_byte(register_pc);                                 // Address ??
    temp1 = read_byte(register_pc+1);                               // Address ?? + 1
    temp1 = read_byte(register_sp_fixed);                           // Address SP
    temp1 = read_byte(register_sp_fixed-1);                         // Address SP - 1
    temp1 = read_byte(register_sp_fixed-2);                         // Address SP - 2
                
    temp1 = read_byte(0xFFFC);                                      // Fetch Vector PCL
    temp2 = read_byte(0xFFFD);                                      // Fetch Vector PCH
                
    register_flags = 0x34;                                          // Set the I and B flags
            
    register_pc = (temp2<<8) | temp1;    
    assert_sync=1;  
    start_read(register_pc);                                        // Fetch first opcode at vector PCH,PCL
    
    
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

// --------------------------------------------------------------------------------------------------


// -------------------------------------------------
//
// Main loop
//
// -------------------------------------------------
 void loop() {
  
  uint16_t local_counter=0;
  
  // Give Teensy 4.1 a moment
  delay (50);
  wait_for_CLK_rising_edge();
  wait_for_CLK_rising_edge();
  wait_for_CLK_rising_edge();

  reset_sequence();


  while (1) {
      
      if (direct_reset==1) reset_sequence();
      
      
      // Set Acceleration using UART receive characters
      // Send the numbers 0,1,2,3 from the host through a serial terminal to the MCL65+
      // for acceleration modes 0,1,2,3
      //
      local_counter++;
      if (local_counter==8000){
        if (Serial.available() ) { 
          incomingByte = Serial.read();   
          switch (incomingByte){
            case 48: mode=0;  Serial.println("M0"); break;
            case 49: mode=1;  Serial.println("M1"); break;
            case 50: mode=2;  Serial.println("M2"); break;
            case 51: mode=3;  Serial.println("M3"); break;
          }
        }
      }    
    
      // Poll for NMI and IRQ
      //
      if (nmi_n_old==0 && direct_nmi==1)        nmi_handler();          
      if (direct_irq==0x1  && (flag_i)==0x0)    irq_handler(0x0);   
      nmi_n_old = direct_nmi;                                        

    
      next_instruction = finish_read_byte();  
      assert_sync=0;
      
      // Change this line:
      execute_opcode(next_instruction);

    } 
}
