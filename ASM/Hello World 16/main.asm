;
; Title:	Hello World - Main
; Author:	Dean Belfield
; Created:	06/11/2022
; Last Updated:	06/11/2022
;
; Modinfo:

			.ASSUME	ADL = 0				

			INCLUDE	"equs.inc"

			SEGMENT CODE
			
			XDEF	MAIN

; The main routine
;
MAIN:			LD	HL, HELLO_WORLD
			CALL	PRSTR
			LD	HL, 0
			RET.L
	
; Print a zero-terminated string
;
PRSTR:			LD	A,(HL)
			OR	A
			RET	Z
			RST	10h
			INC	HL
			JR	PRSTR

; Sample text
;
HELLO_WORLD:		DB 	"Hello World\n\r", 0
	
; RAM
; 
			DEFINE	LORAM, SPACE = ROM
			SEGMENT LORAM
			