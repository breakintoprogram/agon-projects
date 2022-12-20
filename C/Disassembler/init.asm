;
; Title:	Disassembler - Initialisation Code
; Author:	Dean Belfield
; Created:	18/12/2022
; Last Updated: 18/12/2022
;
; Modinfo:

			SEGMENT CODE
			
			XREF	__low_bss
			XREF	__len_bss
			
			XREF	_main
			
			XDEF	_putch
			XDEF	_getch
			
			XDEF	__putch
			XDEF	__getch
			
			XDEF	_errno
		
			.ASSUME	ADL = 1	

argv_ptrs_max:		EQU	16			; Maximum number of arguments allowed in argv

;
; Start in ADL mode
;

			JP	_start			; Jump to start

;
; The header stuff
;
_exec_name:		DB	"DISASSEMBLE.BIN", 0	; The executable name, only used in argv

			ALIGN	64			; The executable header is from byte 64 onwards
			
			DB	"MOS"			; Flag for MOS - to confirm this is a valid MOS command
			DB	00h			; MOS header version 0
			DB	01h			; Flag for run mode (0: Z80, 1: ADL)
			
; And the code follows on immediately after the header
;
_start:			PUSH	AF			; Preserve registers
			PUSH	BC
			PUSH	DE
			PUSH	IX
			PUSH	IY			; Need to preserve IY for MOS			
;			
			PUSH	HL			; Clear the RAM
			CALL	_clear_bss
			POP	HL
;
			LD	IX, argv_ptrs		; The argv array pointer address
			PUSH	IX			; Parameter 2: argv[0] = IX
			CALL	_parse_params		; Parse the parameters
			LD	B, 0			; Clear B from BCU as we just want ARGC
			PUSH	BC			; Parameter 1: argc
			CALL	_main			; int main(int argc, char *argv[])
			POP	DE			; Balance the stack
			POP	DE
;
			POP	IY			; Restore registers
			POP	IX
			POP	DE
			POP 	BC
			POP	AF
			RET
			
; Clear the memory
;
_clear_bss:		LD	BC, __len_bss		; Check for non-zero length
			LD	a, __len_bss >> 16
			OR	A, C
			OR	A, B
			RET	Z			; BSS is zero-length ...
			XOR	A, A
			LD 	(__low_bss), A
			SBC	HL, HL			; HL = 0
			DEC	BC			; 1st byte's taken care of
			SBC	HL, BC
			RET	Z		  	; Just 1 byte ...
			LD	HL, __low_bss		; Reset HL
			LD	DE, __low_bss + 1	; [DE] = bss + 1
			LDIR				; Clear this section
			RET
			
; Parse the parameter string into a C array
; Parameters
; - HL: Address of parameter string
; - IX: Address for array pointer storage
; Returns:
; -  C: Number of parameters parsed
;
_parse_params:		LD	BC, _exec_name
			LD	(IX+0), BC		; ARGV[0] = the executable name
			INC	IX
			INC	IX
			INC	IX
			CALL	_skip_spaces		; Skip HL past any leading spaces
;
			LD	BC, 1			; C: ARGC = 1 - also clears out top 16 bits of BCU
			LD	B, argv_ptrs_max - 1	; B: Maximum number of argv_ptrs
;
_parse_params_1:	
			PUSH	BC			; Stack ARGC	
			PUSH	HL			; Stack start address of token
			CALL	_get_token		; Get the next token
			LD	A, C			; A: Length of the token in characters
			POP	DE			; Start address of token (was in HL)
			POP	BC			; ARGC
			OR	A			; Check for A=0 (no token found) OR at end of string
			RET	Z
;
			LD	(IX+0), DE		; Store the pointer to the token
			PUSH	HL			; DE=HL
			POP	DE
			CALL	_skip_spaces		; And skip HL past any spaces onto the next character
			XOR	A
			LD	(DE), A			; Zero-terminate the token
			INC	IX
			INC	IX
			INC	IX			; Advance to next pointer position
			INC	C			; Increment ARGC
			LD	A, C			; Check for C >= A
			CP	B
			JR	C, _parse_params_1	; And loop
			RET

; Get the next token
; Parameters:
; - HL: Address of parameter string
; Returns:
; - HL: Address of first character after token
; -  C: Length of token (in characters)
;
_get_token:		LD	C, 0			; Initialise length
$$:			LD	A, (HL)			; Get the character from the parameter string
			OR	A			; Exit if 0 (end of parameter string in MOS)
			RET 	Z
			CP	13			; Exit if CR (end of parameter string in BBC BASIC)
			RET	Z
			CP	' '			; Exit if space (end of token)
			RET	Z
			INC	HL			; Advance to next character
			INC 	C			; Increment length
			JR	$B
	
; Skip spaces in the parameter string
; Parameters:
; - HL: Address of parameter string
; Returns:
; - HL: Address of next none-space character
;    F: Z if at end of string, otherwise NZ if there are more tokens to be parsed
;
_skip_spaces:		LD	A, (HL)			; Get the character from the parameter string	
			CP	' '			; Exit if not space
			RET	NZ
			INC	HL			; Advance to next character
			JR	_skip_spaces		; Increment length

; Write a character out to the ESP32
; int putch(int ch)
;
__putch:
_putch:			PUSH	IY
			LD	IY, 0
			ADD	IY, SP
			LD	A, (IY+6)
			RST.LIL	10h	
			LD	HL, 0
			LD	L, A
			LD	SP, IY
			POP	IY				
			RET

; Read a character in from the ESP32
; int getch(void)
;
__getch:
_getch:			LD	HL, 0
			RET

			SEGMENT DATA


; Storage for the argv array pointers
;
argv_ptrs:		BLKP	argv_ptrs_max, 0
	
			SEGMENT BSS			; This section is reset to 0
			
_errno:			DS 	3			; extern int _errno
			
			END
