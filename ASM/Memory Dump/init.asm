;
; Title:	Memory Dump - Initialisation Code
; Author:	Dean Belfield
; Created:	15/11/2022
; Last Updated:	15/11/2022
;
; Modinfo:

			SEGMENT __VECTORS
		
			XREF	INIT_UART
			XREF	INIT_IM2
			XREF	GPIOB_SETMODE
			XREF	MAIN
		
			.ASSUME	ADL = 0
				
			INCLUDE	"equs.inc"
			
;
; Start in mixed mode. Assumes MBASE is set to correct segment
;
			JP		_START		; Jump to start
			DS		5

RST_08:			RST.LIS		08h		; API call
			RET
			DS 		5
			
RST_10:			RST.LIS 	10h		; Output
			RET
			DS		5
			
RST_18:			DS		8
RST_20:			DS		8
RST_28:			DS		8
RST_30:			DS		8	
;	
; The NMI interrupt vector (not currently used by AGON)
;
RST_38:			EI
			RETI
;
; The header stuff is from byte 64 onwards
;
			ALIGN		64
			
			DB		"MOS"		; Flag for MOS - to confirm this is a valid MOS command
			DB		00h		; MOS header version 0
			DB		00h		; Flag for run mode (0: Z80, 1: ADL)
;
; And the code follows on immediately after the header
;
_START:			EI				; Enable the MOS interrupts
			JP	MAIN			; Start user code
			