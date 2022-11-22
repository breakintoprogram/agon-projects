;
; Title:	Memory Dump - Main
; Author:	Dean Belfield
; Created:	15/11/2022
; Last Updated:	15/11/2022
;
; Modinfo:

			.ASSUME	ADL = 0				

			INCLUDE	"equs.inc"
			INCLUDE "mos_api.inc"	; In MOS/src

			SEGMENT CODE
			
			XDEF	MAIN

			XREF	ASC_TO_NUMBER
			
			XREF	Print_String
			XREF	Print_Hex24
			XREF	Print_Hex16
			XREF	Print_Hex8
			
; Error: Invalid parameter
;
ERR_INVALID_PARAM:	LD		HL, 19
			RET.L

; The main routine
; HLU: Address to parameters in string buffer (or 0 if no parameters)
; Returns:
;  HL: Error code
;
MAIN:			CALL		ASC_TO_NUMBER		; Fetch the first parameter
			JR		NC, ERR_INVALID_PARAM
			PUSH.LIL	DE
			CALL		ASC_TO_NUMBER		; Fetch the second parameter
			POP.LIL		HL
			JR		C, $F			
			LD.LIL		DE, 256			; Default value if not specified
$$:			CALL		Memory_Dump			
			LD		HL, 0			; Return with OK
			RET.L
			
; Memory Dump
; HLU: Start of memory to dump
; DE:  Number of bytes to dump out
;
Memory_Dump:		CALL		Print_Hex24
			LD		A, ':'
			RST		10h
			LD		A, ' '
			RST		10h
			LD		B, 16
			LD		IX, Buffer
			LD		(IX+0), ' '
;			
Memory_Dump_1:		LD.LIL		A, (HL)
			PUSH		AF
			CP		' '
			JR		NC, Memory_Dump_2
			LD		A, '.'
;			
Memory_Dump_2:		LD		(IX+1), A
			INC		IX
			POP		AF
			CALL		Print_Hex8
			INC.LIL		HL
			DEC		DE
			LD		A, D
			OR		E
			JR		Z, Memory_Dump_3
			DJNZ		Memory_Dump_1
			CALL		Memory_Dump_5			
			MOSCALL		mos_getkey		; Check for ESC
			CP		1Bh
			JR 		NZ, Memory_Dump
			RET
			
Memory_Dump_3:		LD		A, B
			OR		A
			JR		Z, Memory_Dump_5
			DEC		B
			JR		Z, Memory_Dump_5
			LD		A, ' '
;
Memory_Dump_4:		RST		10h
			RST 		10h
			DJNZ		Memory_Dump_4
;
Memory_Dump_5:		LD		(IX+1),0Dh
			LD		(IX+2),0Ah
			LD		(IX+3),00h
			PUSH.LIL	HL
			LD		HL, Buffer
			CALL		Print_String
			POP.LIL		HL
			RET
		
; RAM
; 
			DEFINE	LORAM, SPACE = ROM
			SEGMENT LORAM
			
Buffer:			DS	256
			