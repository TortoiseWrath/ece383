.include "p24Hxxxx.inc"
.global __reset

.bss
u16_x: .space 2 ; uint16 u16_x
u8_a:  .space 1 ; uint8  u8_a
u8_b:  .space 1 ; uint8  u8_b
u8_c:  .space 1 ; uint8  u8_c
u8_d:  .space 1 ; uint8  u8_d
u8_e:  .space 1 ; uint8  u8_e
u8_f:  .space 1 ; uint8  u8_f

.text
__reset:
	mov #__SP_init, W15		; initialize stack pointer
	mov #__SPLIM_init, W0
	mov W0,SPLIM			; initialize stack limit reg.
	

; u16_x = 0x0001
mov #0x0001, W0
mov W0, u16_x

; u8_a = 0xAF
mov.b #0xAF, W0
mov.b WREG, u8_a

; u8_b = 0x50
mov.b #0x50, W0
mov.b WREG, u8_b

; u8_c = u8_a & u8_b //bitwise AND
mov.b u8_a, WREG ; W0 = u8_a
and.b u8_b, WREG ; W0 = W0 & u8_b
mov.b WREG, u8_c ; u8_c = W0

; u8_d = u8_a | u8_b //bitwise IOR
mov.b u8_a, WREG ; W0 = u8_a
ior.b u8_b, WREG ; W0 = W0 | u8_b
mov.b WREG, u8_d ; u8_d = W0

; u8_e = u8_a ^ u8_b //bitwise XOR
mov.b u8_a, WREG ; W0 = u8_a
xor.b u8_b, WREG ; W0 = W0 ^ u8_b
mov.b WREG, u8_e ; u8_e = W0

; u8_f = ~u8_a //bitwise complement
com.b u8_a, WREG ; W0 = ~u8_a
mov.b WREG, u8_f ; u8_f = W0

; u16_x = ~u8_d | (u16_x & u8_c)
mov u16_x, W0 ; W0 = u16_x
and.b u8_c, WREG ; W0 = W0 & u8_c
mov W0, W1 ; W1 = W0
com.b u8_d, WREG ; lower W0 = ~u8_d
ior.b W1, W0, W0 ; lower W0 = lower W0 (~u8_d) | lower W1 (u16_x & u8_c)
; upper W0 is still upper W1
mov W0, u16_x ; u16_x = W0

done: goto done

.end