#include "hardware_config.h"
#include <Arduino.h>

// Setup Teensy 4.1 IO's
void setup_teensy_pins(void) {
  
  pinMode(PIN_CLK0,        INPUT);  
  pinMode(PIN_RESET,       INPUT);  
  pinMode(PIN_READY_n,     INPUT);  
  pinMode(PIN_IRQ,         INPUT);  
  pinMode(PIN_NMI,         INPUT);  
  pinMode(PIN_RDWR_n,      OUTPUT); 
  pinMode(PIN_P0,          OUTPUT); 
  pinMode(PIN_P1,          OUTPUT); 
  pinMode(PIN_P2,          OUTPUT); 
  
  pinMode(PIN_ADDR0,       OUTPUT); 
  pinMode(PIN_ADDR1,       OUTPUT); 
  pinMode(PIN_ADDR2,       OUTPUT); 
  pinMode(PIN_ADDR3,       OUTPUT); 
  pinMode(PIN_ADDR4,       OUTPUT); 
  pinMode(PIN_ADDR5,       OUTPUT); 
  pinMode(PIN_ADDR6,       OUTPUT); 
  pinMode(PIN_ADDR7,       OUTPUT);
  pinMode(PIN_ADDR8,       OUTPUT); 
  pinMode(PIN_ADDR9,       OUTPUT); 
  pinMode(PIN_ADDR10,      OUTPUT); 
  pinMode(PIN_ADDR11,      OUTPUT); 
  pinMode(PIN_ADDR12,      OUTPUT); 
  pinMode(PIN_ADDR13,      OUTPUT); 
  pinMode(PIN_ADDR14,      OUTPUT); 
  pinMode(PIN_ADDR15,      OUTPUT);  
  
  pinMode(PIN_DATAIN0,     INPUT); 
  pinMode(PIN_DATAIN1,     INPUT); 
  pinMode(PIN_DATAIN2,     INPUT); 
  pinMode(PIN_DATAIN3,     INPUT); 
  pinMode(PIN_DATAIN4,     INPUT); 
  pinMode(PIN_DATAIN5,     INPUT); 
  pinMode(PIN_DATAIN6,     INPUT); 
  pinMode(PIN_DATAIN7,     INPUT);
    
  pinMode(PIN_DATAOUT0,    OUTPUT); 
  pinMode(PIN_DATAOUT1,    OUTPUT); 
  pinMode(PIN_DATAOUT2,    OUTPUT); 
  pinMode(PIN_DATAOUT3,    OUTPUT); 
  pinMode(PIN_DATAOUT4,    OUTPUT); 
  pinMode(PIN_DATAOUT5,    OUTPUT); 
  pinMode(PIN_DATAOUT6,    OUTPUT); 
  pinMode(PIN_DATAOUT7,    OUTPUT);
  pinMode(PIN_DATAOUT_OE_n,  OUTPUT); 




}

