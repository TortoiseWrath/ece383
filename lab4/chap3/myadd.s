.include "p24Hxxxx.inc"
.global __reset

.bss
aa:		.space 1	; uint8 aa
bb: 	.space 1	; uint8 bb
lsp:	.space 2	; uint16 lsp
msp:	.space 2	; uint16 msp
sum:	.space 2	; uint16 sum

.text
__reset:
	mov #__SP_init, W15		; initialize stack pointer
	mov #__SPLIM_init, W0
	mov W0,SPLIM			; initialize stack limit reg.
	
; aa = 100
mov.b #100, W0
mov.b WREG, aa		
; bb = 22
mov.b #22, W0
mov.b WREG, bb	

; msp = 0x8859
mov #0x8859, W0
mov W0, lsp
; lsp = 0x1171
mov #0x1171, W0
mov W0, msp

; sum = lsp + msp
mov lsp, WREG
add msp, WREG
mov WREG, sum

; sum = wreg + aa + bb
mov #0, W0  		; clear WREG so upper byte = 0
mov.b aa, WREG		; move aa to wreg 
mov W0, W2  		; save this value of WREG (aa as word)
mov.b bb, WREG		; bb as word in WREG
add W0, W2, W0		; add W2 (aa as word) to w0 (bb as word)
add sum, WREG		; add previous sum (lsp + msp) to WREG
mov WREG, sum		; move to sum

done: goto done

.end