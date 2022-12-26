;
; Title:	Memory Dump - Main
; Author:	Dean Belfield
; Created:	15/11/2022
; Last Updated:	23/12/2022
;
; Modinfo:
; 23/12/2022:	Added parameter parsing code, help text

			.ASSUME	ADL = 0				

			INCLUDE	"equs.inc"
			INCLUDE "mos_api.inc"	; In MOS/src

			SEGMENT CODE
			
			XDEF	_main

			XREF	ASC_TO_NUMBER
			
			XREF	Print_String
			XREF	Print_Hex24
			XREF	Print_Hex16
			XREF	Print_Hex8
			
; Error: Invalid parameter
;
_err_invalid_param:	LD		HL, 19			; The return code: Invalid parameters
			RET

; Help text
;
_help:			LD		HL, _help_text
			CALL		Print_String
			LD		HL, 0			; The return code: OK
			RET
			
_help_text:		DB 		"AGON Memory Dump by Dean Belfield\n\r"
			DB 		"Usage:\n\r";
			DB 		"memdump address <length>\n\r", 0;

; The main routine
; IXU: argv - pointer to array of parameters
;   C: argc - number of parameters
; Returns:
;  HL: Error code, or 0 if OK
;
_main:			LD		DE, 100h		; Default number of bytes to fetch
			LD		A, C			; Fetch number of parameters
			CP		2			; Is it less than 2?
			JR		C, _help		; Then goto help
			JR		Z, $F			; If it is equal to 2, then proceed with default number of bytes to fetch
			CP		4			; Is it greater than or equal to 4?
			JR		NC, _help		; Yes, so goto help
;
			LD.LIL		HL,(IX+6)		; HLU: Pointer to the length parameter string
			CALL		ASC_TO_NUMBER		; DEU: length
;			
$$:			PUSH.LIL	DE			; Stack the length
			LD.LIL		HL,(IX+3)		; HLU: Pointer to the start address parameter string
			CALL		ASC_TO_NUMBER		; DEU: Start address
			DB		5Bh			; Prefix for EX.L DE, HL (bodge- cannot get Zilog tools to compile this!)
			EX		DE, HL
;			
			POP.LIL		DE			; Restore the length
			CALL		Memory_Dump			
;
			LD		HL, 0			; Return with OK
			RET
			
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
			CP      	07Fh                	; DEL is non-printable
			JR      	Z, Memory_Dump_1a
			CP		' '                 	; As are all control chars.
			JR		NC, Memory_Dump_2
Memory_Dump_1a:  	LD		A, '.'			; replace nonprintable chars with dot.			
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
			