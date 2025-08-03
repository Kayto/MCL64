//
// basic_rom.h - Commodore 64 BASIC ROM Image
// ROM size: 8KB (0x2000 bytes) mapped to $A000-$BFFF
//

#ifndef BASIC_ROM_H
#define BASIC_ROM_H

#include <stdint.h>

// BASIC ROM array declaration
extern const uint8_t BASIC_ROM[0x2000];
#endif // BASIC_ROM_H