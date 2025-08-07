#ifndef ADDRESSING_MODES_H
#define ADDRESSING_MODES_H

#include <stdint.h>

// Acceleration configuration (must match main file)
#ifndef ENABLE_ACCELERATION
#define ENABLE_ACCELERATION 0
#endif

// Define SPEEDUP (matching main file)
#define SPEEDUP 0

// External variables needed by addressing modes - with correct types
extern uint16_t register_pc, effective_address;
extern uint8_t register_x, register_y, ea_data;

// Function declarations for addressing modes
uint8_t Fetch_Immediate();
uint8_t Fetch_ZeroPage();
uint8_t Fetch_ZeroPage_X();
uint8_t Fetch_ZeroPage_Y();
uint16_t Calculate_Absolute();
uint8_t Fetch_Absolute();
uint8_t Fetch_Absolute_X(uint8_t page_cross_check);
uint8_t Fetch_Absolute_Y(uint8_t page_cross_check);
uint8_t Fetch_Indexed_Indirect_X();
uint8_t Fetch_Indexed_Indirect_Y(uint8_t page_cross_check);

void Write_ZeroPage(uint8_t local_data);
void Write_Absolute(uint8_t local_data);
void Write_ZeroPage_X(uint8_t local_data);
void Write_ZeroPage_Y(uint8_t local_data);
void Write_Absolute_X(uint8_t local_data);
void Write_Absolute_Y(uint8_t local_data);
void Write_Indexed_Indirect_X(uint8_t local_data);
void Write_Indexed_Indirect_Y(uint8_t local_data);

void Double_WriteBack(uint8_t local_data);

#endif // ADDRESSING_MODES_H