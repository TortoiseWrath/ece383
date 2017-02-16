.include "p24Hxxxx.inc"
.global __reset

.bss
xx:		.space 2	; uint16 xx
yy: 	.space 2	; uint16 yy
i:   	.space 1	; uint8 i
j:  	.space 1	; uint8 j
k:  	.space 1	; uint8 k
l:		.space 1	; uint8 l
m:		.space 1	; uint8 m

.text
__reset:
	mov #__SP_init, W15		; initialize stack pointer
	mov #__SPLIM_init, W0
	mov W0,SPLIM			; initialize stack limit reg.
	

; xx = 0xDEAD
mov #0xDEAD, W0
mov W0, xx

; yy = 0xBEEF
mov #0xBEEF, W0
mov W0, yy

; CWID = 11367275
; i = 75, j = 72, k = 36
mov.b #75, W0
mov.b WREG, i
mov.b #72, W0
mov.b WREG, j
mov.b #36, W0
mov.b WREG, k

; l = i + k
mov #0, W0 ; zero WREG
mov.b i, WREG
mov W0, W1 ; W1 = 0x00ii
mov.b k, WREG
mov W0, W2 ; W2 = 0x00kk
add.b W1, W2, W0 ; W0 = i + k
mov.b WREG, l

; m = j - l
mov #0, W0 ; zero WREG
mov.b j, WREG
mov W0, W1 ; W1 = 0x00jj
mov.b l, WREG
mov W0, W2 ; W2 = 0x00ll
sub W1, W2, W0 ; W0 = j - l
mov.b WREG, m

; xx = xx - yy - m
mov #0, W0 ; zero WREG
mov.b m, WREG ; move m into lower bits of W0
mov W0, W1 ; copy W0 to W1 (W1 = 0x00mm)
mov xx, W0
mov yy, W2
sub W0, W2, W0 ; W0 = xx - yy
sub W0, W1, W0 ; W0 = W0 - m
mov W0, xx

done: goto done

.end