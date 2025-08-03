// ============================================================================
// MCL64 - Opcode Dispatch Table
// ----------------------------------------------------------------------------
// This file contains the main opcode switch statement to keep the main
// file more readable and maintainable.
// ============================================================================

#ifndef OPCODE_DISPATCH_H
#define OPCODE_DISPATCH_H

// Function to execute the opcode based on the instruction byte
inline void execute_opcode(uint8_t instruction) {
    switch (instruction) {
        case 0x00:   irq_handler(0x1); break;  // BRK - Break
        case 0x01:   opcode_0x01();    break;  // OR - Indexed Indirect X
        case 0x02:   opcode_0x02();    break;  // JAM
        case 0x03:   opcode_0x03();    break;  // SLO - Indexed Indirect X
        case 0x04:   opcode_0x04();    break;  // NOP - ZeroPage
        case 0x05:   opcode_0x05();    break;  // OR ZeroPage
        case 0x06:   opcode_0x06();    break;  // ASL A - Arithmetic Shift Left - ZeroPage
        case 0x07:   opcode_0x07();    break;  // SLO - ZeroPage
        case 0x08:   opcode_0x08();    break;  // PHP - Push processor status to the stack
        case 0x09:   opcode_0x09();    break;  // OR - Immediate
        case 0x0A:   opcode_0x0A();    break;  // ASL A
        case 0x0B:   opcode_0x0B();    break;  // ANC - Immediate
        case 0x0C:   opcode_0x0C();    break;  // NOP - Absolute
        case 0x0D:   opcode_0x0D();    break;  // OR - Absolute
        case 0x0E:   opcode_0x0E();    break;  // ASL A - Arithmetic Shift Left - Absolute
        case 0x0F:   opcode_0x0F();    break;  // SLO - Absolute
        case 0x10:   opcode_0x10();    break;  // BNE - Branch on Zero Clear
        case 0x11:   opcode_0x11();    break;  // OR Indirect Indexed  Y
        case 0x12:   opcode_0x12();    break;  // JAM
        case 0x13:   opcode_0x13();    break;  // Indirect Indexed  Y
        case 0x14:   opcode_0x14();    break;  // NOP - ZeroPage , X
        case 0x15:   opcode_0x15();    break;  // OR - ZeroPage,X
        case 0x16:   opcode_0x16();    break;  // ASL A - Arithmetic Shift Left - ZeroPage , X
        case 0x17:   opcode_0x17();    break;  // SLO - ZeroPage , X
        case 0x18:   opcode_0x18();    break;  // CLC
        case 0x19:   opcode_0x19();    break;  // OR - Absolute,Y
        case 0x1A:   opcode_0xEA();    break;  // NOP
        case 0x1B:   opcode_0x1B();    break;  // SLO - Absolute , Y
        case 0x1C:   opcode_0x1C();    break;  // NOP - Absolute , X
        case 0x1D:   opcode_0x1D();    break;  // OR - Absolute,X
        case 0x1E:   opcode_0x1E();    break;  // ASL A - Arithmetic Shift Left - Absolute , X
        case 0x1F:   opcode_0x1F();    break;  // SLO - Absolute , X
        case 0x20:   opcode_0x20();    break;  // JSR - Jump to Subroutine
        case 0x21:   opcode_0x21();    break;  // AND - Indexed Indirect
        case 0x22:   opcode_0x22();    break;  // JAM
        case 0x23:   opcode_0x23();    break;  // RLA - Indexed Indirect X
        case 0x24:   opcode_0x24();    break;  // BIT - ZeroPage
        case 0x25:   opcode_0x25();    break;  // AND - ZeroPage
        case 0x26:   opcode_0x26();    break;  // ROL - Rotate Left - ZeroPage
        case 0x27:   opcode_0x27();    break;  // RLA - ZeroPage
        case 0x28:   opcode_0x28();    break;  // PLP - Pop processor status from the stack
        case 0x29:   opcode_0x29();    break;  // AND - Immediate
        case 0x2A:   opcode_0x2A();    break;  // ROL A
        case 0x2B:   opcode_0x2B();    break;  // ANC - Immediate
        case 0x2C:   opcode_0x2C();    break;  // BIT - Absolute
        case 0x2D:   opcode_0x2D();    break;  // AND - Absolute
        case 0x2E:   opcode_0x2E();    break;  // ROL - Rotate Left - Absolute
        case 0x2F:   opcode_0x2F();    break;  // RLA - Absolute
        case 0x30:   opcode_0x30();    break;  // BMI - Branch on Minus (N Flag Set)
        case 0x31:   opcode_0x31();    break;  // AND - Indirect Indexed
        case 0x32:   opcode_0x32();    break;  // JAM
        case 0x33:   opcode_0x33();    break;  // RLA - Indirect Indexed  Y
        case 0x34:   opcode_0x34();    break;  // NOP - ZeroPage , X
        case 0x35:   opcode_0x35();    break;  // AND - ZeroPage,X
        case 0x36:   opcode_0x36();    break;  // ROL - Rotate Left - ZeroPage , X
        case 0x37:   opcode_0x37();    break;  // RLA - ZeroPage , X
        case 0x38:   opcode_0x38();    break;  // SEC
        case 0x39:   opcode_0x39();    break;  // AND - Absolute,Y
        case 0x3A:   opcode_0xEA();    break;  // NOP
        case 0x3B:   opcode_0x3B();    break;  // RLA - Absolute , Y
        case 0x3C:   opcode_0x3C();    break;  // NOP - Absolute , X
        case 0x3D:   opcode_0x3D();    break;  // AND - Absolute,X
        case 0x3E:   opcode_0x3E();    break;  // ROL - Rotate Left - Absolute , X
        case 0x3F:   opcode_0x3F();    break;  // RLA - Absolute , X
        case 0x40:   opcode_0x40();    break;  // RTI - Return from Interrupt
        case 0x41:   opcode_0x41();    break;  // EOR - Indexed Indirect X
        case 0x42:   opcode_0x42();    break;  // JAM
        case 0x43:   opcode_0x43();    break;  // SRE - Indexed Indirect X
        case 0x44:   opcode_0x44();    break;  // NOP - ZeroPage
        case 0x45:   opcode_0x45();    break;  // EOR - ZeroPage
        case 0x46:   opcode_0x46();    break;  // LSR - Logical Shift Right - ZeroPage
        case 0x47:   opcode_0x47();    break;  // SRE - ZeroPage
        case 0x48:   opcode_0x48();    break;  // PHA - Push Accumulator to the stack
        case 0x49:   opcode_0x49();    break;  // EOR - Immediate
        case 0x4A:   opcode_0x4A();    break;  // LSR A
        case 0x4B:   opcode_0x4B();    break;  // ALR - Immediate
        case 0x4C:   opcode_0x4C();    break;  // JMP - Jump Absolute
        case 0x4D:   opcode_0x4D();    break;  // EOR - Absolute
        case 0x4E:   opcode_0x4E();    break;  // LSR - Logical Shift Right - Absolute
        case 0x4F:   opcode_0x4F();    break;  // SRE - Absolute
        case 0x50:   opcode_0x50();    break;  // BVC - Branch on Overflow Clear
        case 0x51:   opcode_0x51();    break;  // EOR - Indirect Indexed  Y
        case 0x52:   opcode_0x52();    break;  // JAM
        case 0x53:   opcode_0x53();    break;  // SRE - Indirect Indexed  Y
        case 0x54:   opcode_0x54();    break;  // NOP - ZeroPage , X
        case 0x55:   opcode_0x55();    break;  // EOR - ZeroPage,X
        case 0x56:   opcode_0x56();    break;  // LSR - Logical Shift Right - ZeroPage , X
        case 0x57:   opcode_0x57();    break;  // SRE - ZeroPage , X
        case 0x58:   opcode_0x58();    break;  // CLI
        case 0x59:   opcode_0x59();    break;  // EOR - Absolute,Y
        case 0x5A:   opcode_0xEA();    break;  // NOP
        case 0x5B:   opcode_0x5B();    break;  // RE - Absolute , Y
        case 0x5C:   opcode_0x5C();    break;  // NOP - Absolute , X
        case 0x5D:   opcode_0x5D();    break;  // EOR - Absolute,X
        case 0x5E:   opcode_0x5E();    break;  // LSR - Logical Shift Right - Absolute , X
        case 0x5F:   opcode_0x5F();    break;  // SRE - Absolute , X
        case 0x60:   opcode_0x60();    break;  // RTS - Return from Subroutine
        case 0x61:   opcode_0x61();    break;  // ADC - Indexed Indirect X
        case 0x62:   opcode_0x62();    break;  // JAM
        case 0x63:   opcode_0x63();    break;  // RRA - Indexed Indirect X
        case 0x64:   opcode_0x64();    break;  // NOP - ZeroPage
        case 0x65:   opcode_0x65();    break;  // ADC - ZeroPage
        case 0x66:   opcode_0x66();    break;  // ROR - Rotate Right - ZeroPage
        case 0x67:   opcode_0x67();    break;  // RRA - ZeroPage
        case 0x68:   opcode_0x68();    break;  // PLA - Pop Accumulator from the stack
        case 0x69:   opcode_0x69();    break;  // ADC - Immediate
        case 0x6A:   opcode_0x6A();    break;  // ROR A
        case 0x6B:   opcode_0x6B();    break;  // ARR - Immediate
        case 0x6C:   opcode_0x6C();    break;  // JMP - Jump Indirect
        case 0x6D:   opcode_0x6D();    break;  // ADC - Absolute
        case 0x6E:   opcode_0x6E();    break;  // ROR - Rotate Right - Absolute
        case 0x6F:   opcode_0x6F();    break;  // RRA - Absolute
        case 0x70:   opcode_0x70();    break;  // BVS - Branch on Overflow Set
        case 0x71:   opcode_0x71();    break;  // ADC - Indirect Indexed  Y
        case 0x72:   opcode_0x72();    break;  // JAM
        case 0x73:   opcode_0x73();    break;  // RRA - Indirect Indexed  Y
        case 0x74:   opcode_0x74();    break;  // NOP - ZeroPage , X
        case 0x75:   opcode_0x75();    break;  // ADC - ZeroPage , X
        case 0x76:   opcode_0x76();    break;  // ROR - Rotate Right - ZeroPage , X
        case 0x77:   opcode_0x77();    break;  // RRA - ZeroPage , X
        case 0x78:   opcode_0x78();    break;  // SEI
        case 0x79:   opcode_0x79();    break;  // ADC - Absolute , Y
        case 0x7A:   opcode_0xEA();    break;  // NOP
        case 0x7B:   opcode_0x7B();    break;  // RRA - Absolute , Y
        case 0x7C:   opcode_0x7C();    break;  // NOP - Absolute , X
        case 0x7D:   opcode_0x7D();    break;  // ADC - Absolute , X
        case 0x7E:   opcode_0x7E();    break;  // ROR - Rotate Right - Absolute , X
        case 0x7F:   opcode_0x7F();    break;  // RRA - Absolute , X
        case 0x80:   opcode_0x80();    break;  // NOP - Immediate
        case 0x81:   opcode_0x81();    break;  // STA - Indexed Indirect X
        case 0x82:   opcode_0x82();    break;  // NOP - Immediate
        case 0x83:   opcode_0x83();    break;  // SAX - Indexed Indirect X
        case 0x84:   opcode_0x84();    break;  // STY - ZeroPage
        case 0x85:   opcode_0x85();    break;  // STA - ZeroPage
        case 0x86:   opcode_0x86();    break;  // STX - ZeroPage
        case 0x87:   opcode_0x87();    break;  // SAX - ZeroPage
        case 0x88:   opcode_0x88();    break;  // DEY
        case 0x89:   opcode_0x89();    break;  // NOP - Immediate
        case 0x8A:   opcode_0x8A();    break;  // TXA
        case 0x8B:   opcode_0x8B();    break;  // ANE - Immediate
        case 0x8C:   opcode_0x8C();    break;  // STY - Absolute
        case 0x8D:   opcode_0x8D();    break;  // STA - Absolute
        case 0x8E:   opcode_0x8E();    break;  // STX - Absolute
        case 0x8F:   opcode_0x8F();    break;  // SAX - Absolute
        case 0x90:   opcode_0x90();    break;  // BCC - Branch on Carry Clear
        case 0x91:   opcode_0x91();    break;  // STA - Indirect Indexed  Y
        case 0x92:   opcode_0x92();    break;  // JAM
        case 0x93:   opcode_0x93();    break;  // SHA - ZeroPage , Y
        case 0x94:   opcode_0x94();    break;  // STY - ZeroPage , X
        case 0x95:   opcode_0x95();    break;  // STA - ZeroPage , X
        case 0x96:   opcode_0x96();    break;  // STX - ZeroPage , Y
        case 0x97:   opcode_0x97();    break;  // SAX - ZeroPage , Y
        case 0x98:   opcode_0x98();    break;  // TYA
        case 0x99:   opcode_0x99();    break;  // STA - Absolute , Y
        case 0x9A:   opcode_0x9A();    break;  // TXS
        case 0x9B:   opcode_0x9B();    break;  // TAS - Absolute , Y 
        case 0x9C:   opcode_0x9C();    break;  // SHY - Absolute , X
        case 0x9D:   opcode_0x9D();    break;  // STA - Absolute , X
        case 0x9E:   opcode_0x9E();    break;  // SHX - Absolute , Y
        case 0x9F:   opcode_0x9F();    break;  // SHA - Absolute , Y
        case 0xA0:   opcode_0xA0();    break;  // LDY - Immediate
        case 0xA1:   opcode_0xA1();    break;  // LDA - Indexed Indirect X
        case 0xA2:   opcode_0xA2();    break;  // LDX - Immediate
        case 0xA3:   opcode_0xA3();    break;  // LAX - Indexed Indirect X
        case 0xA4:   opcode_0xA4();    break;  // LDY - ZeroPage
        case 0xA5:   opcode_0xA5();    break;  // LDA - ZeroPage
        case 0xA6:   opcode_0xA6();    break;  // LDX - ZeroPage
        case 0xA7:   opcode_0xA7();    break;  // LAX - ZeroPage
        case 0xA8:   opcode_0xA8();    break;  // TAY
        case 0xA9:   opcode_0xA9();    break;  // LDA - Immediate
        case 0xAA:   opcode_0xAA();    break;  // TAX
        case 0xAB:   opcode_0xAB();    break;  // LAX - Immediate
        case 0xAC:   opcode_0xAC();    break;  // LDY - Absolute
        case 0xAD:   opcode_0xAD();    break;  // LDA - Absolute
        case 0xAE:   opcode_0xAE();    break;  // LDX - Absolute
        case 0xAF:   opcode_0xAF();    break;  // LAX - Absolute
        case 0xB0:   opcode_0xB0();    break;  // BCS - Branch on Carry Set
        case 0xB1:   opcode_0xB1();    break;  // LDA - Indirect Indexed  Y
        case 0xB2:   opcode_0xB2();    break;  // JAM
        case 0xB3:   opcode_0xB3();    break;  // LAX - Indirect Indexed  Y
        case 0xB4:   opcode_0xB4();    break;  // LDY - ZeroPage , X
        case 0xB5:   opcode_0xB5();    break;  // LDA - ZeroPage , X
        case 0xB6:   opcode_0xB6();    break;  // LDX - ZeroPage , Y
        case 0xB7:   opcode_0xB7();    break;  // LAX - ZeroPage , Y
        case 0xB8:   opcode_0xB8();    break;  // CLV
        case 0xB9:   opcode_0xB9();    break;  // LDA - Absolute , Y
        case 0xBA:   opcode_0xBA();    break;  // TSX
        case 0xBB:   opcode_0xBB();    break;  // LAS - Absolute , Y
        case 0xBC:   opcode_0xBC();    break;  // LDY - Absolute , X
        case 0xBD:   opcode_0xBD();    break;  // LDA - Absolute , X
        case 0xBE:   opcode_0xBE();    break;  // LDX - Absolute , Y
        case 0xBF:   opcode_0xBF();    break;  // LAX - Absolute , Y
        case 0xC0:   opcode_0xC0();    break;  // CPY - Immediate
        case 0xC1:   opcode_0xC1();    break;  // CMP - Indexed Indirect X
        case 0xC2:   opcode_0xC2();    break;  // NOP - Immediate
        case 0xC3:   opcode_0xC3();    break;  // DCP - Indexed Indirect X
        case 0xC4:   opcode_0xC4();    break;  // CPY - ZeroPage
        case 0xC5:   opcode_0xC5();    break;  // CMP - ZeroPage
        case 0xC6:   opcode_0xC6();    break;  // DEC - ZeroPage
        case 0xC7:   opcode_0xC7();    break;  // DCP - ZeroPage
        case 0xC8:   opcode_0xC8();    break;  // INY
        case 0xC9:   opcode_0xC9();    break;  // CMP - Immediate
        case 0xCA:   opcode_0xCA();    break;  // DEX
        case 0xCB:   opcode_0xCB();    break;  // SBX - Immediate
        case 0xCC:   opcode_0xCC();    break;  // CPY - Absolute
        case 0xCD:   opcode_0xCD();    break;  // CMP - Absolute
        case 0xCE:   opcode_0xCE();    break;  // DEC - Absolute
        case 0xCF:   opcode_0xCF();    break;  // DCP - Absolute
        case 0xD0:   opcode_0xD0();    break;  // BNE - Branch on Zero Clear
        case 0xD1:   opcode_0xD1();    break;  // CMP - Indirect Indexed  Y
        case 0xD2:   opcode_0xD2();    break;  // JAM
        case 0xD3:   opcode_0xD3();    break;  // DCP - Indirect Indexed  Y
        case 0xD4:   opcode_0xD4();    break;  // NOP - ZeroPage , X
        case 0xD5:   opcode_0xD5();    break;  // CMP - ZeroPage , X
        case 0xD6:   opcode_0xD6();    break;  // DEC - ZeroPage , X
        case 0xD7:   opcode_0xD7();    break;  // DCP - ZeroPage , X
        case 0xD8:   opcode_0xD8();    break;  // CLD
        case 0xD9:   opcode_0xD9();    break;  // CMP - Absolute , Y
        case 0xDA:   opcode_0xEA();    break;  // NOP
        case 0xDB:   opcode_0xDB();    break;  // DCP - Absolute , Y
        case 0xDC:   opcode_0xDC();    break;  // NOP - Absolute , X
        case 0xDD:   opcode_0xDD();    break;  // CMP - Absolute , X
        case 0xDE:   opcode_0xDE();    break;  // DEC - Absolute , X
        case 0xDF:   opcode_0xDF();    break;  // DCP - Absolute , X
        case 0xE0:   opcode_0xE0();    break;  // CPX - Immediate
        case 0xE1:   opcode_0xE1();    break;  // SBC - Indexed Indirect X
        case 0xE2:   opcode_0xE2();    break;  // NOP - Immediate
        case 0xE3:   opcode_0xE3();    break;  // ISC - Indexed Indirect X
        case 0xE4:   opcode_0xE4();    break;  // CPX - ZeroPage
        case 0xE5:   opcode_0xE5();    break;  // SBC - ZeroPage
        case 0xE6:   opcode_0xE6();    break;  // INC - ZeroPage
        case 0xE7:   opcode_0xE7();    break;  // ISC - ZeroPage
        case 0xE8:   opcode_0xE8();    break;  // INX
        case 0xE9:   opcode_0xE9();    break;  // SBC - Immediate
        case 0xEA:   opcode_0xEA();    break;  // NOP
        case 0xEB:   opcode_0xE9();    break;  // SBC - Immediate
        case 0xEC:   opcode_0xEC();    break;  // CPX - Absolute
        case 0xED:   opcode_0xED();    break;  // SBC - Absolute
        case 0xEE:   opcode_0xEE();    break;  // INC - Absolute
        case 0xEF:   opcode_0xEF();    break;  // ISC - Absolute
        case 0xF0:   opcode_0xF0();    break;  // BEQ - Branch on Zero Set
        case 0xF1:   opcode_0xF1();    break;  // SBC - Indirect Indexed  Y
        case 0xF2:   opcode_0xF2();    break;  // JAM
        case 0xF3:   opcode_0xF3();    break;  // ISC - Indirect Indexed  Y
        case 0xF4:   opcode_0xF4();    break;  // NOP - ZeroPage , X
        case 0xF5:   opcode_0xF5();    break;  // SBC - ZeroPage , X
        case 0xF6:   opcode_0xF6();    break;  // INC - ZeroPage , X
        case 0xF7:   opcode_0xF7();    break;  // ISC - ZeroPage , X
        case 0xF8:   opcode_0xF8();    break;  // SED
        case 0xF9:   opcode_0xF9();    break;  // SBC - Absolute , Y
        case 0xFA:   opcode_0xEA();    break;  // NOP
        case 0xFB:   opcode_0xFB();    break;  // ISC - Absolute , Y
        case 0xFC:   opcode_0xFC();    break;  // NOP - Absolute , X
        case 0xFD:   opcode_0xFD();    break;  // SBC - Absolute , X
        case 0xFE:   opcode_0xFE();    break;  // INC - Absolute , X
        case 0xFF:   opcode_0xFF();    break;  // ISC - Absolute , X
    }
}

#endif // OPCODE_DISPATCH_H