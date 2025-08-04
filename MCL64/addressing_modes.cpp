#include "addressing_modes.h"

// External functions we need
extern uint8_t read_byte(uint16_t local_address);
extern void write_byte(uint16_t local_address, uint8_t local_write_data);

// External variables - fix the types to match the main file
extern uint16_t register_pc, effective_address;
extern uint8_t register_x, register_y, ea_data;

// -------------------------------------------------
// Addressing Modes
// -------------------------------------------------
uint8_t Fetch_Immediate()  {
    register_pc++;
    ea_data = read_byte(register_pc);
    return ea_data;
}

uint8_t Fetch_ZeroPage()  { 
    effective_address = Fetch_Immediate(); 
    ea_data = read_byte(effective_address);
    return ea_data;
}

uint8_t Fetch_ZeroPage_X()  {   
    uint16_t bal;
    bal = Fetch_Immediate();
    if (SPEEDUP==0) read_byte(register_pc+1); 
    effective_address = (0x00FF & (bal + register_x));
    ea_data = read_byte(effective_address);
    return ea_data;
}

uint8_t Fetch_ZeroPage_Y()  {   
    uint16_t bal;
    bal = Fetch_Immediate();
    if (SPEEDUP==0) read_byte(register_pc+1); 
    effective_address = (0x00FF & (bal + register_y)); 
    ea_data = read_byte(effective_address);
    return ea_data;
}

uint16_t Calculate_Absolute()  { 
    uint16_t adl, adh;

    adl = Fetch_Immediate();
    adh = Fetch_Immediate()<<8;
    effective_address = adl + adh;  
    return effective_address;
}

uint8_t Fetch_Absolute()  { 
    uint16_t adl, adh;

    adl = Fetch_Immediate();
    adh = Fetch_Immediate()<<8;
    effective_address = adl + adh;  
    ea_data = read_byte(effective_address);
    return ea_data;
}

uint8_t Fetch_Absolute_X(uint8_t page_cross_check)  {
    uint16_t bal, bah;
    
    bal = Fetch_Immediate();
    bah = Fetch_Immediate()<<8;
    effective_address = bah + bal + register_x;
    ea_data = read_byte(effective_address );
    
    if (  (SPEEDUP==0) && page_cross_check==1 && (  (0xFF00&effective_address) != (0xFF00&bah) ) ) {  
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
    
    if ( (SPEEDUP==0)  && page_cross_check==1 && (  (0xFF00&effective_address) != (0xFF00&bah) ) ) {  
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
    
    if ( (SPEEDUP==0) && page_cross_check==1 && ((0xFF00&effective_address) != (0xFF00&bah)) ) {  
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
    if (SPEEDUP==0) read_byte(effective_address);
    write_byte( (0x00FF&(effective_address + register_x)) , local_data );
    return;
}

void Write_ZeroPage_Y(uint8_t local_data)  {
    effective_address = Fetch_Immediate();
    if (SPEEDUP==0) read_byte(effective_address);
    write_byte( (0x00FF&(effective_address + register_y)) , local_data );
    return;
}

void Write_Absolute_X(uint8_t local_data)  {
    uint16_t bal,bah;

    bal = Fetch_Immediate();
    bah = Fetch_Immediate()<<8;
    effective_address = bal + bah + register_x; 
    if (SPEEDUP==0) read_byte(effective_address);
    write_byte(effective_address , local_data );  
    return;
}

void Write_Absolute_Y(uint8_t local_data)  {
    uint16_t bal,bah;

    bal = Fetch_Immediate();
    bah = Fetch_Immediate()<<8;
    effective_address = bal + bah + register_y;
    read_byte(effective_address);

    if (SPEEDUP==0) {
        if ( (0xFF00&effective_address) != (0xFF00&bah) ) { 
        read_byte(effective_address);
        }
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
    if (SPEEDUP==0) read_byte(effective_address);
    write_byte(effective_address , local_data );
    return;
}

void Double_WriteBack(uint8_t local_data)  {  
    if (SPEEDUP==0)  write_byte(effective_address , ea_data);
    write_byte(effective_address , local_data);
    return;
}