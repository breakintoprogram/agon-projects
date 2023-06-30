/*
 * Title:			Disassembler - Main
 * Author:			Dean Belfield
 * Created:			18/12/2022
 * Last Updated:		30/03/2023
 *
 * Based upon information in http://www.z80.info/decoding.htm 
 *
 * Modinfo:
 * 04/01/2023:		Optimisations
 * 16/01/2023:		Additional eZ80 instructions; SLP, RSMIX, IN0, OUT0 and the extra block instructions
 * 18/01/2023:		Additional eZ80 instructions: LD A,MB/LD MB,A, TSTIO, LEA, PEA
 * 21/01/2023:		Added eZ80 addressing modes, fixed LD, ADD, INC, DEC for IX and IY; fixed column widths, t_alu format
 * 27/01/2023:		Fixed default ADL mode, LD SP, EX (SP) and JP (rr) for IX and IY
 * 30/03/2023:		Fixed decode bug in LD [rp],(Mmn)
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Storage for the opcode decoder
//
struct s_opcode {
	long address;				// Start address of the opcode
	long count;					// Size of the opcode in bytes
	unsigned char addressMode;	// Addressing mode (0-5)
	unsigned char shift;		// Shift byte (0X00, 0xCB, 0xDD, 0xED, 0xFD)
	unsigned char byteData[8];	// The byte data
	char text[32];				// Storage for the opcode text	
};

void 			help(void);
void 			pad(int count, char c);
int				parseNumber(char * ptr, long * value);
unsigned char	decodeByte(long * address, struct s_opcode * opcode);
long			decodeWord(long * address, struct s_opcode * opcode);
long			decodeJR(long * address, struct s_opcode * opcode);
void 			decodeOperand(long * address, struct s_opcode * opcode);
void 			decodeOperandCB(long * address, struct s_opcode * opcode);
void 			decodeOperandED(long * address, struct s_opcode * opcode);

extern int errno;				// errno - used by stdlib
extern int putch(int ch);		// In init.asm
extern int getch(void);

long	adl;					// ADL mode

// Lookup tables
//
const char * t_r[3][8] = {
	{ "B", "C", "D", "E",   "H",   "L", "(HL)", "A" },
	{ "B", "C", "D", "E", "IXH", "IXL", "(IX)", "A" },
	{ "B", "C", "D", "E", "IYH", "IYL", "(IY)", "A" }
};
const char * t_rp[8][4] = {
	{ "BC", "DE", "HL", "SP" },
	{ "BC", "DE", "IX", "SP" },
	{ "BC", "DE", "IY", "SP" },
	{ "BC", "DE", "HL", "AF" },
	{ "BC", "DE", "IX", "AF" },
	{ "BC", "DE", "IY", "AF" },
	{ "BC", "DE", "HL", "IX" },
	{ "BC", "DE", "HL", "IY" }
};
const char * t_shift[] = { "HL", "IX", "IY" };
const char * t_cc[] = { "NZ", "Z", "NC", "C", "PO", "PE", "P", "M" };
const char * t_alu[] = { "ADD", "ADC", "SUB", "SBC", "AND", "XOR", "OR", "CP" };
const char * t_rot[] = { "RLC", "RRC", "RL", "RR", "SLA", "SRA", "SLL", "SRL"};
const char * t_bit[] = { "BIT", "RES", "SET" };
const char * t_im[] = { "0", "0/1", "1", "2" };
const char * t_io[] = { "IN", "OUT" };
const char * t_inc[] = { "INC", "DEC" };
const char * t_bl1[4][5] = {
	{ "LDI",  "CPI",  "INI",  "OUTI", "OUTI2" },
	{ "LDD",  "CPD",  "IND",  "OUTD", "OUTD2" },
	{ "LDIR", "CPIR", "INIR", "OTIR", "OTI2R" },
	{ "LDDR", "CPDR", "INDR", "OTDR", "OTD2R" }
};
const char * t_bl2[4][3] = {
	{ "INIM",  "OTIM",  "INI2"  },
	{ "INDM",  "OTDM",  "IND2"  },
	{ "INIMR", "OTIMR", "INI2R" },
	{ "INDMR", "OTDMR", "IND2R" }
};
const char * t_o1[] = { "RLCA", "RRCA", "RLA", "RRA", "DAA", "CPL", "SCF", "CCF" };
const char * t_o2[] = { "LD I,A", "LD R,A", "LD A,I", "LD A,R", "RRD", "RLD", "NOP", "NOP" };
const char * t_am[] = { "", ".SIS", ".LIS", ".SIL", ".LIL" };

// Parameters:
// - argc: Argument count
// - argv: Pointer to the argument string - zero terminated, parameters separated by spaces
//
int main(int argc, char * argv[]) {
	struct 	s_opcode opcode;
	long	address;
	long	count;
	int		i;
	char	c;
	
	adl	= 1;	// Default ADL mode
	
	if(argc < 3 || argc > 4) {
		help();
		return 0;
	}
	
	if(	!parseNumber(argv[1], &address) ||
		!parseNumber(argv[2], &count)
	) { 			
		return 19;
	}
	
	if(argc == 4) {
		if(!parseNumber(argv[3], &adl)) return 19;
	}

	while(count > 0) {
		opcode.shift = 0x00;
		opcode.addressMode = 0x00;
		opcode.text[0] = '\0';
		opcode.address = address;
		opcode.count = 0;
		decodeOperand(&address, &opcode);
		if(opcode.addressMode > 0) {
			decodeOperand(&address, &opcode);
		}
		if(opcode.shift > 0) {
			decodeOperand(&address, &opcode);
		}	
		printf("%06X ", opcode.address);
		for(i=0; i<opcode.count; i++) {
			printf("%02X ",opcode.byteData[i]);
		}
		pad((6 - opcode.count) * 3, ' ');
		for(i=0; i<opcode.count; i++) {
			c = opcode.byteData[i];
			putch((c > 31 && c < 127) ? c : '.');
		}
		pad((6 - opcode.count) , ' ');
		printf(" %s\n\r", opcode.text);
		count -= opcode.count;
	}
	return 0;
}

void pad(int count, char c) {
	int	i;

	for(i=0; i<count; i++) putch(c);
}

// Help text
//
void help() {
	printf("AGON eZ80 Disassembler by Dean Belfield\n\r");
	printf("Usage:\n\r");
	printf("disassemble address length [adl]\n\r");
}

// Parse a number
// Parameters:
// - ptr: Pointer to the number to parse
// - value: Pointer to the storage for the return value
// Returns:
// - 1: Parsed OK
// - 0: Error parsing the number
//
int parseNumber(char * ptr, long * value) {
	char *  endptr;
	int 	base = 10;
	
	if(*ptr == '&') {
		base = 16;
		ptr++;
	}	
	*value = strtol(ptr, &endptr, base);
	if(ptr == endptr || *endptr != '\0' || errno != 0) {
		return 0;
	}	
	return 1;
}

// Decode a byte
// Parameters:
// - address: Pointer to the address counter
// - opcode: Pointer to the opcode structure
// Returns:
// - unsigned char: Word
//
unsigned char decodeByte(long * address, struct s_opcode * opcode) {
	unsigned char b;
	
	b = *(unsigned char *)((*address)++);
	opcode->byteData[opcode->count++] = b;
	return b;
}

// Decode a relative jump
// Parameters:
// - address: Pointer to the address counter
// - opcode: Pointer to the opcode structure
// Returns:
// - long: Word
//
long decodeJR(long * address, struct s_opcode * opcode) {
	char b;
	
	b = *(char *)((*address)++);
	opcode->byteData[opcode->count++] = b;
	return (*address) + b;
}

// Decode a word (2 or 3 bytes, depending upon ADL mode
// Parameters:
// - address: Pointer to the address counter
// - opcode: Pointer to the opcode structure
// Returns:
// - long: Word
//
long decodeWord(long * address, struct s_opcode * opcode) {
	unsigned char	l, h, u, am;
	
	am = opcode->addressMode;

	l = *(long *)((*address)++);
	h = *(long *)((*address)++);

	opcode->byteData[opcode->count++] = l;
	opcode->byteData[opcode->count++] = h;

	// 2 or 3 byte fetches are determined by ADL mode AND opcode->addressMode
	//
	if(adl == 1 || am >= 3) {
		//
		// Word size = 3; fetch a 24-bit word from the code
		//
		u = *(long *)((*address)++);
		opcode->byteData[opcode->count++] = u;
	}
	else {
		//
		// Word size = 2; fetch a 16-bit word, and set bits 16-23 to the segment address
		//
		u = (*address & 0xFF0000) >> 16;
	}	
	return l | (h << 8) | (u << 16);
}

// Decode an opcode
// Parameters:
// - address: Pointer to the address counter
// - opcode: Pointer to the opcode structure
//
void decodeOperand(long * address, struct s_opcode * opcode) {
	unsigned char	b;
	unsigned char	x, y, z, p, q;
	char * 			t;
	int				shift = 0;

	unsigned char 	am = opcode->addressMode;
	
	switch(opcode->shift) {
		//
		// Handle CB prefixed instructions (rolls, shifts, and bit operations)
		//		
		case 0xCB: {
			return decodeOperandCB(address, opcode);
		} break;
		//
		// Handle ED prefixed instructions (miscellanous operations)
		//
		case 0xED: {
			return decodeOperandED(address, opcode);
		} break;
		//
		// IX
		//
		case 0xDD: {
			shift = 1;
		} break;
		//
		// IY
		//
		case 0xFD: {
			shift = 2;
		} break;
	}

	b = *(long *)((*address)++);		// Fetch the byte and increment the pointer
	
	x = (b & 0xC0) >> 6;				// 0b11000000
	y = (b & 0x38) >> 3;				// 0b00111000
	z = (b & 0X07);						// 0b00000111
	p = y >> 1;							// 0b00110000 01 110 111 x=1 y=6 z=7
	q = y & 1; 							// 0b00001000
	
	// Extra eZ80 instructions added:
	//
	// LD (IX/Y+n),BC	0x0F = 0b00001111: x=0,y=1,z=7,p=0,q=1
	// LD (IX/Y+n),DE	0x1F = 0b00011111: x=0,y=3,z=7,p=1,q=1
	// LD (IX/Y+n),HL	0x2F = 0b00101111: x=0,y=5,z=7,p=2,q=1
	// LD BC,(IX/Y+n)	0x07 = 0b00000111: x=0,y=0,z=7,p=0,q=0
	// LD DE,(IX/Y+n)	0x17 = 0b00010111: x=0,y=2,z=7,p=1,q=0
	// LD HL,(IX/Y+n)	0x27 = 0b00100111: x=0,y=4,z=7,p=2,q=0	
	
	// Extra eZ80 suffixes for addressing modes added:
	//
	// .SIS	(LD B,B)	0x40 = 0b01000000: x=1,y=0,z=0
	// .LIS	(LD C,C)	0x49 = 0b01001001: x=1,y=1,z=1
	// .SIL	(LD D,D)	0x52 = 0b01010010: x=1,y=2,z=2
	// .LIL	(LD E,E)	0x5B = 0b01011011: x=1,y=3,z=3
	
	t = opcode->text;
	
	opcode->byteData[opcode->count++] = b;

	switch(x) {
		//
		// X=0
		//
		case 0: {						
			switch(z) {
				//
				// Z=0: Relative jumps and assorted ops
				//
				case 0:	{	
					switch(y) {
						case 0:	{
							strcpy(t, "NOP");
						} break;
						case 1:	{
							strcpy(t, "EX AF,AF'");
						} break;
						case 2:	{
							sprintf(t, "DJNZ &%06X", decodeJR(address, opcode));
						} break;
						case 3:	{
							sprintf(t, "JR &%06X", decodeJR(address, opcode));
						} break;
						default: {
							sprintf(t, "JR %s,&%06X", t_cc[y-4], decodeJR(address, opcode));
						} break;
					}
				} break;
				//
				// Z=1: 16-bit load immediate/add
				//
				case 1: {				
					if(q == 0) {
						sprintf(t, "LD%s %s,&%06X", t_am[am], t_rp[shift][p], decodeWord(address, opcode));
					}
					else {
						sprintf(t, "ADD%s %s,%s", t_am[am], t_rp[shift][2], t_rp[0][p]);
					}
				} break;
				//
				// Z=2: Indirect load
				//
				case 2: { 		
					if(q == 0) {
						switch(p) {
							case 0: {
								strcpy(t, "LD (BC),A");
							} break;
							case 1: {
								strcpy(t, "LD (DE),A");
							} break;
							case 2: {
								sprintf(t, "LD%s (&%06X),HL", t_am[am], decodeWord(address, opcode));
							} break;
							case 3: {
								sprintf(t, "LD%s (&%06X),A", t_am[am], decodeWord(address, opcode));
							} break;
						}
					}
					else {
						switch(p) {
							case 0: {
								strcpy(t, "LD A,(BC)");
							} break;
							case 1: {
								strcpy(t, "LD A,(DE)");
							} break;
							case 2: {
								sprintf(t, "LD%s HL,(&%06X)", t_am[am], decodeWord(address, opcode));
							} break;
							case 3: {
								sprintf(t, "LD%s A,(&%06X)", t_am[am], decodeWord(address, opcode));
							} break;
						}
						
					}
				} break; 
				//
				// Z=3: 16-bit increment/decrement
				//
				case 3: {		
					sprintf(t, "%s%s %s", t_inc[q], t_am[am], t_rp[3+shift][p]);
				} break;
				//
				// Z=4: 8-bit increment
				// Z=5: 8-bit decrement
				//
				case 4:
				case 5: {				
					sprintf(t, "%s %s", t_inc[z-4], t_r[shift][y]);
				} break;
				//
				// Z=6: 8-bit load immediate
				//
				case 6: { 				
					sprintf(t, "LD %s,&%02X", t_r[shift][y], decodeByte(address, opcode));
				} break;
				//
				// Z=7: Assorted operations on accumulator flags / LD (IX/Y+n),rr / LD rr, (IX/Y+n)
				//
				case 7: {			
					if(shift == 0) {
						strcpy(t, t_o1[y]);
					}
					else {
						if(q == 0) {
							sprintf(t, "LD %s,(%s%+d)", t_rp[shift][p], t_shift[shift], (char)decodeByte(address, opcode));
						}
						else {
							sprintf(t, "LD (%s%+d),%s", t_shift[shift], (char)decodeByte(address, opcode), t_rp[shift][p]);
						}
					}
				} break;
			}
		} break;
		//
		// X = 1
		//
		case 1: {
			//
			// Select an addressing mode
			//
			if(y == z && y < 4) {
				opcode->addressMode = y + 1;
			}
			else if(y == 6 && z == 6) {
				strcpy(t, "HALT");
			}
			else {
				//
				// LD (IX/Y+d),r
				// LD r,(IX/Y+d)
				//
				if(shift > 0 && (y == 6 || z == 6)) {
					if(y == 6) {
						sprintf(t, "LD%s (%s%+d),%s", t_am[am], t_shift[shift], (char)decodeByte(address, opcode), t_r[0][z]);
					}
					else if(z == 6) {
						sprintf(t, "LD%s %s,(%s%+d)", t_am[am], t_r[0][y], t_shift[shift], (char)decodeByte(address, opcode));
					}
				}
				//
				// LD r,r'
				//
				else {
					sprintf(t, "LD%s %s,%s", t_am[am], t_r[shift][y], t_r[shift][z]);
				}
			}
		} break;
		//
		// X = 2: ALU operations
		//
		case 2: {						
			sprintf(t, "%s A,%s", t_alu[y], t_r[shift][z]);
		} break;	
		//
		// X = 3
		//
		case 3: {						
			switch(z) {
				//
				// Z=0: Conditional return
				//
				case 0: {				
					sprintf(t, "RET%s %s", t_am[am], t_cc[y]);
				} break;
				//
				// Z=1: POP and various operations
				//
				case 1: {				
					if(q == 0) {
						sprintf(t, "POP%s %s", t_am[am], t_rp[3+shift][p]);
					}
					else {
						switch(p) {
							case 0: {
								sprintf(t, "RET%s", t_am[am]);
							} break;
							case 1: {
								strcpy(t, "EXX");
							} break;
							case 2: {
								sprintf(t, "JP%s (%s)", t_am[am], t_rp[shift][2]);
							} break;
							case 3: {
								sprintf(t, "LD%s SP,%s", t_am[am], t_rp[shift][2]);
							} break;
						}
					}
					
				} break;
				//
				// Z=2: Conditional jump
				//
				case 2: {				
					sprintf(t, "JP%s %s,&%06X", t_am[am], t_cc[y], decodeWord(address, opcode));
				} break;
				//
				// Z=3: Assorted operations
				//
				case 3: {				
					switch(y) {
						case 0: {
							sprintf(t, "JP%s &%06X", t_am[am], decodeWord(address, opcode));
						} break;
						case 1: {
							opcode->shift = 0xCB;
						} break;
						case 2: {
							sprintf(t, "OUT (&%02X),A", decodeByte(address, opcode));
						} break;
						case 3: {
							sprintf(t, "IN (&%02X),A", decodeByte(address, opcode));
						} break;
						case 4: {
							sprintf(t, "EX (SP),%s", t_rp[shift][2]);
						} break;
						case 5: {
							strcpy(t, "EX DE,HL");
						} break;
						case 6: {
							strcpy(t, "DI");
						} break;
						case 7: {
							strcpy(t, "EI");
						} break;
					}
				} break;
				//
				// Z=4: Conditional call
				//
				case 4: {				
					sprintf(t, "CALL%s %s,&%06X", t_am[am], t_cc[y], decodeWord(address, opcode));
				} break;
				//
				// Z=5: PUSH and various operations
				//
				case 5: {				
					if(q == 0) {
						sprintf(t, "PUSH%s %s", t_am[am], t_rp[3+shift][p]);
					}
					else {
						switch(p) {
							case 0: {
								sprintf(t, "CALL%s &%06X", t_am[am], decodeWord(address, opcode));
							} break;
							case 1: {
								opcode->shift = 0xDD;
							} break;
							case 2: {
								opcode->shift = 0xED;
							} break;
							case 3: {
								opcode->shift = 0xFD;
							} break;
						}
					}
				} break;
				//
				// Z=6: Operate on accumulator and immediate operand
				//
				case 6: {				
					sprintf(t, "%s A,&%02X", t_alu[y], decodeByte(address, opcode));
				} break;
				//
				// Z=7: Restart instructions
				//
				case 7: {				
					sprintf(t, "RST%s &%02X", t_am[am], y<<3);
				} break;
			} break;
		} break;	
	}
}

// Decode an opcode (CB-prefixed operands)
// Parameters:
// - address: Pointer to the address counter
// - opcode: Pointer to the opcode structure
//
void decodeOperandCB(long * address, struct s_opcode * opcode) {
	unsigned char	b;
	unsigned char	x, y, z, p, q;
	char * 			t;

	b = *(long *)((*address)++);		// Fetch the byte and increment the pointer
	
	x = (b & 0xC0) >> 6;				// 0b11000000
	y = (b & 0x38) >> 3;				// 0b00111000
	z = (b & 0X07);						// 0b00000111
	p = y >> 1;							// 0b00110000
	q = y & 1; 							// 0b00001000
	
	t = opcode->text;
	
	opcode->byteData[opcode->count++] = b;
	
	switch(x) {
		//
		// X=0: Rotate and Shift Operands
		//
		case 0: {
			sprintf(t, "%s %s", t_rot[y], t_r[0][z]);		
		} break;
		//
		// X=1: BIT
		// X=2: RES
		// X=3: SET
		//
	    case 1:
		case 2:
		case 3: {
			sprintf(t, "%s %d,%s", t_bit[x-1], y, t_r[0][z]);
		} break;
	}
}

// Decode an opcode (ED-prefixed operands)
// Parameters:
// - address: Pointer to the address counter
// - opcode: Pointer to the opcode structure
//
void decodeOperandED(long * address, struct s_opcode * opcode) {
	unsigned char	b;
	unsigned char	x, y, z, p, q;
	char * 			t;

	unsigned char 	am = opcode->addressMode;

	b = *(long *)((*address)++);		// Fetch the byte and increment the pointer
	
	x = (b & 0xC0) >> 6;				// 0b11000000
	y = (b & 0x38) >> 3;				// 0b00111000
	z = (b & 0X07);						// 0b00000111
	p = y >> 1;							// 0b00110000
	q = y & 1; 							// 0b00001000
	
	// Extra eZ80 instructions added:
	//
	// IN0 B,(N)	0x00 = 0b00000000: x=0,z=0,y=0
	// IN0 C,(N)	0x08 = 0b00001000: x=0,z=0,y=1
	// IN0 D,(N)	0x10 = 0b00010000: x=0,z=0,y=2
	// IN0 E,(N)	0x18 = 0b00011000: x=0,z=0,y=3
	// IN0 H,(N)	0x20 = 0b00100000: x=0,z=0,y=4
	// IN0 L,(N)	0x28 = 0b00101000: x=0,z=0,y=5
	// IN0 A,(N)	0x38 = 0b00111000: x=0,z=0,y=7
	//
	// OUT0 (N),B	0x01 = 0b00000001: x=0,z=1,y=0
	// OUT0 (N),C	0x09 = 0b00001001: x=0,z=1,y=1
	// OUT0 (N),D	0x11 = 0b00010001: x=0,z=1,y=2
	// OUT0 (N),E	0x19 = 0b00011001: x=0,z=1,y=3
	// OUT0 (N),H	0x21 = 0b00100001: x=0,z=1,y=4
	// OUT0 (N),L	0x29 = 0b00101001: x=0,z=1,y=5
	// OUT0 (N),A	0x39 = 0b00111001: x=0,z=1,y=7
	//
	// LEA BC, IX+d	0x02 = 0b00000010: x=0,z=2,y=0
	// LEA DE, IX+d	0x12 = 0b00010010: x=0,z=2,y=2
	// LEA HL, IX+d	0x22 = 0b00100010: x=0,z=2,y=4
	// LEA IX, IX+d	0X32 = 0b00110010: x=0,z=2,y=6
		
	// LEA BC, IY+d	0x03 = 0b00000011: x=0,z=3,y=0
	// LEA DE, IY+d	0x13 = 0b00010011: x=0,z=3,y=2
	// LEA HL, IY+d	0x23 = 0b00100011: x=0,z=3,y=4
	// LEA IY, IY+d 0X33 = 0b00110011: x=0,z=3,y=6
	//
	// TST A,B		0x04 = 0b00000100: x=0,z=4,y=0
	// TST A,C		0X0C = 0b00001100: x=0,z=4,y=1
	// TST A,D		0x14 = 0b00010100: x=0,z=4,y=2
	// TST A,E		0x1C = 0b00011100: x=0,z=4,y=3
	// TST A,H		0x24 = 0b00100100: x=0,z=4,y=4
	// TST A,L		0x2C = 0b00101100: x=0,z=4,y=5
	// TST A,(HL)	0x34 = 0b00110100: x=0,z=4,y=6
	// TST A,A		0x3C = 0b00111100: x=0,z=4,y=7
	//
	// LD BC,(HL)	0x07 = 0b00000111: x=0,z=7,y=0
	// LD (HL),BC	0x0F = 0b00001111: x=0,z=7,y=1
	// LD DE,(HL)	0x17 = 0b00010111: x=0,z=7,y=2
	// LD (HL),DE	0x1F = 0b00011111: x=0,z=7,y=3
	// LD HL,(HL)	0x27 = 0b00100111: x=0,z=7,y=4
	// LD (HL),HL	0x2F = 0b00101111: x=0,z=7,y=5
	//
	// MLT BC 		0x4C = 0b01001100: x=1,z=4,y=1
	// LEA IX, IY+d	0X54 = 0b01010100: x=1,z=4,y=2
	// MLT DE		0x5C = 0b01011100: x=1,z=4,y=3
	// TST A,n		0x64 = 0b01100100: x=1,z=4,y=4
	// MTL HL		0x6C = 0b01101100: x=1,z=4,y=5
	// TSTIO n		0x74 = 0b01110100: x=1,z=4,y=6

	// LEA IY, IX+d 0X55 = 0b01010101: x=1,z=5,y=2
	// PEA IX+d		0x65 = 0b01100101: x=1,z=5,y=4
	// LD MB, A		0x6D = 0b01101101: x=1,z=5,y=5
	// STMIX		0x7D = 0b01111101: x=1,z=5,y=7

	// PEA IY-d		0x66 = 0b01100110: x=y,z=6,y=4
	// LD A, MB		0x6E = 0b01101110: x=1,z=6,y=5
	// SLP			0x76 = 0b01110110: x=1,z=6,y=6
	// RSMIX		0x7E = 0b01111110: x=1,z=6,y=7

	// Plus the block instructions
	
	t = opcode->text;
	
	opcode->byteData[opcode->count++] = b;

	switch(x) {
		//
		// X = 0
		//
		case 0: {
			switch(z) {
				case 0: {
					sprintf(t, "IN0 %s,(&%02X)", t_r[0][y], decodeByte(address, opcode));
				} break;
				case 1: {
					sprintf(t, "OUT0 (&%02X),%s", decodeByte(address, opcode), t_r[0][y]);
				} break;
				case 2: {
					sprintf(t, "LEA %s,IX%+d",  t_rp[6][y>>1], (char)decodeByte(address, opcode));
				} break;
				case 3: {
					sprintf(t, "LEA %s,IX%+d",  t_rp[7][y>>1], (char)decodeByte(address, opcode));
				} break;
				case 4: {
					sprintf(t, "TST A,%s", t_r[0][y]);
				} break;
				case 7: {
					if(q == 0) {
						sprintf(t, "LD %s,(HL)", t_rp[0][p]);
					}
					else {
						sprintf(t, "LD (HL),%s", t_rp[0][p]);
					}
				} break;
 			}
		} break;
		//
		// X = 1
		//
		case 1: {
			switch(z) {
				case 0:
				case 1: {
					if(y != 6) {
						sprintf(t, "%s %s,(C)", t_io[z], t_r[0][y]);
					}
					else {
						sprintf(t, "%s (C)", t_io[z]);
					}
				} break;
				case 2: {
					if(q == 0) {
						sprintf(t, "SBC HL,%s", t_rp[0][p]);
					}
					else {
						sprintf(t, "ADC HL,%s", t_rp[0][p]);
					}
				} break; 
				case 3: {
					if(q == 0) {
						sprintf(t, "LD (&%06X),%s", decodeWord(address, opcode), t_rp[0][p]);
					}
					else {
						sprintf(t, "LD %s,(&%06X)", t_rp[0][p], decodeWord(address, opcode));
					}
				} break;
				case 4: {
					switch(y) {
						case 0: {
							strcpy(t, "NEG");
						} break;
						case 1: {
							strcpy(t, "MLT BC");
						} break;
						case 2: {
							sprintf(t, "LEA IX,IY%+d", (char)decodeByte(address, opcode));
						} break;
						case 3: {
							strcpy(t, "MLT DE");
						} break;
						case 4: {
							sprintf(t, "TST A,&%02X", decodeByte(address, opcode));
						} break;
						case 5: {
							strcpy(t, "MLT HL");
						} break;
						case 6: {
							sprintf(t, "TSTIO &%02X", decodeByte(address, opcode));
						} break;
					}
				} break;
				case 5: {
					switch(y) {
						case 1: {
							sprintf(t, "RETI%s", t_am[am]);
						} break;
						case 2: {
							sprintf(t, "LEA IY,IX%+d", (char)decodeByte(address, opcode));
						} break;
						case 4: {
							sprintf(t, "PEA IX%+d", (char)decodeByte(address, opcode));
						} break;
						case 5: {
							strcpy(t, "LD MB, A");
						} break;
						case 7: {
							strcpy(t, "STMIX");
						} break;
						default: {
							sprintf(t, "RETN%s", t_am[am]);
						} break;
					}
				} break;
				case 6: {
					switch(y) {
						case 4: {
							sprintf(t, "PEA IY%+d", (char)decodeByte(address, opcode));
						} break;						
						case 5: {
							strcpy(t, "LD A, MB");
						} break;
						case 6: {
							strcpy(t, "SLP");
						} break;
						case 7: {
							strcpy(t, "RSMIX");
						} break;
						default: {
							sprintf(t, "IM %s", t_im[y]);
						} break;
					}
				} break;
				case 7: {
					strcpy(t, t_o2[y]);
				} break;
			}
		} break;
		//
		// X = 2: Block operations
		//
		case 2: {
			if(y < 4) {
				if(z >= 2 && z <= 4) {
					sprintf(t, "%s%s", t_am[am], t_bl2[y][z-2]);
				}
			}
			else {
				if(z <= 4) {
					sprintf(t, "%s%s", t_am[am], t_bl1[y-4][z]);
				}
			}
			
		} break;
	}
}
