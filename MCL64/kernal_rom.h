//
// kernal_rom.h - Commodore 64 KERNAL ROM Image  
// ROM size: 8KB (0x2000 bytes) mapped to $E000-$FFFF
//

#ifndef KERNAL_ROM_H
#define KERNAL_ROM_H

#include <stdint.h>

// KERNAL ROM array declaration
extern const uint8_t KERNAL_ROM[0x2000];

#endif // KERNAL_ROM_H
