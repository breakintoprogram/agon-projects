;
; Title:	Hello World - Main
; Author:	Dean Belfield
; Created:	06/11/2022
; Last Updated:	17/12/2022
;
; Modinfo:
; 17/12/2022:	Added parameter processing

			.ASSUME	ADL = 1				

			INCLUDE	"equs.inc"

			SEGMENT CODE
			
			XDEF	_main

; The main routine
; IX: argv - pointer to array of parameters
;  C: argc - number of parameters
; Returns:
; HL: Error code, or 0 if OK
;
_main:			LD	HL, s_HELLO_WORLD	; Print "Hello World"
			CALL	PRSTR
			LD	HL, s_ARGUMENTS		; Print "Arguments:"
			CALL	PRSTR
			
			LD	B, C			; B: # of arguments
			
; Loop round printing the arguments
;
$$:			LD	HL, s_ARGV		; Print " - argv: "
			CALL	PRSTR
			LD	HL, (IX+0)		; Get the argument string address from the array
			CALL	PRSTR			; And print it
			INC	IX			; Increment to the next index in the array
			INC	IX
			INC	IX
			LD	HL, s_CRLF		; Print a carriage return
			CALL	PRSTR			
			DJNZ	$B			; And loop
			
; Return with error code 0
;
			LD	HL, 0
			RET
	
; Print a zero-terminated string
;
PRSTR:			LD	A,(HL)
			OR	A
			RET	Z
			RST.LIL	10h
			INC	HL
			JR	PRSTR

; Sample text
;
s_HELLO_WORLD:		DB 	"Hello World\n\r", 0
s_ARGUMENTS:		DB	"Arguments:\n\r", 0
s_ARGV:			DB	" - argv: ", 0
s_CRLF:			DB	"\n\r", 0
	
; RAM
; 
			DEFINE	LORAM, SPACE = ROM
			SEGMENT LORAM
			