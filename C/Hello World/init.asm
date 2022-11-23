;
; Title:	Hello World - Initialisation Code
; Author:	Dean Belfield
; Created:	22/11/2022
; Last Updated:	22/11/2022
;
; Modinfo:

			SEGMENT CODE
			
			XREF	_main
			
			XDEF	_putch
			XDEF	_getch
			
			XDEF	__putch
			XDEF	__getch
		
			.ASSUME	ADL = 1							
;
; Start in ADL mode
;
			JP	_start		; Jump to start			
;
; The header stuff is from byte 64 onwards
;
			ALIGN	64
			
			DB	"MOS"		; Flag for MOS - to confirm this is a valid MOS command
			DB	00h		; MOS header version 0
			DB	01h		; Flag for run mode (0: Z80, 1: ADL)
;
; And the code follows on immediately after the header
;
_start:			PUSH	AF		; Preserve registers
			PUSH	BC
			PUSH	DE
			PUSH	IX
			PUSH	IY		; Preserve this
;
			PUSH	HL		; argv[0] = HL (pointer to the parameters)
			LD	HL, 1
			PUSH	HL		; argc
			CALL	_main		; int main(int argc, char *argv[])
			POP	DE		; Balance the stack
			POP	DE

			POP	IY		; Restore registers
			POP	IX
			POP	DE
			POP 	BC
			POP	AF
			RET
			
;
; Helper functions
;

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
	
; int getch(void)
;
__getch:
_getch:			LD	HL, 0
			RET

