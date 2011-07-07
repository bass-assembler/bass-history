mapper lorom
org $8000; fill $8000

define base = $80
define add x = clc; adc.w #{base}+{x}
define sum x = {add {x}+0}; {add {x}+1}; {add {x}+2}; {add {x}+3}
define all x,multiplier
  {sum {x}+0*{multiplier}}; {sum {x}+1*{multiplier}}
  {sum {x}+2*{multiplier}}; {sum {x}+3*{multiplier}}
enddef

define hvbjoy = $4212

define loop
  wait{#}:
    lda {hvbjoy}
    bmi wait{#}
enddef

define 'A' = 0x61
define 'B' = 0x62

org $8000; base $7ec000
  {all $20, 4}; {all $30, 4}

  nop #4
main:

print origin, ",", pc
enqueue pc
org $8100
  db $01,$02,$40,$80
dequeue pc

  jml main

  {loop}
  {loop}

  lda.b #%11000011
  lda.b #5 % %11

  db "ABCD"
  dw $8000 + $2000
  incbin "include.bin", 0x0000, 0x0004
  incsrc "include.asm"

