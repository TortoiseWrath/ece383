.include "p24Hxxxx.inc"

.global __reset

.bss
check_val: .space 2 	; uint16 check_val
ones_count: .space 1	; uint8 ones_count
first_one: .space 1		; uint8 first_one

.text
__reset:
	mov #__SP_init, W15	; initialize stack pointer
	mov #__SPLIM_init, W0
	mov W0, SPLIM		; initialize stack limit reg. 
	
; check_val = 0xF508;
mov #0xF508, W0
mov W0, check_val

; ones_count = 0;
clr.b ones_count

; first_one = 16;
mov #16, W0
mov.b WREG, first_one

; W1 = check_val
mov check_val, W1

; W2 = 0x0001 (mask)
mov #0x0001, W2

clr W0					; W0 (pos) = 0

top_for: 
	cp W0, #16			; test: W0 (pos) < 16
	bra nn, done	 	; if pos - 16 >= 0 (pos >= 16), finished
	
	and W1, W2, W3		; W3 = W1 (check_val) & W2 (mask)
	cp0 W3				; test: W3 - 0 (W3 > 0 means lsb of W1 is 1)
	bra z, end_if		; if W3 == 0, go to end_if
	; if (check_val & 1) {
		cp0.b ones_count	; test: ones_count - 0
		bra nz, incr	; if ones_count != 0, go to incr
		; if (!ones_count) {
			mov.b WREG, first_one ; first_one = WREG (pos)
		; }
		incr:
			inc ones_count ; ones_count++
	; }
	end_if:
	lsr W1, W1			; check_val (W1) >>= 1
	inc W0, W0			; W0 (pos) ++
	
	bra top_for			; loop
	
done: goto done