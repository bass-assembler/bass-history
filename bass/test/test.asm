mapper lorom
org $8000; fill $8000

define base = $80
define add x = clc; adc.w #{base}+{x}
define sum x = {add {x}+0}; {add {x}+1}; {add {x}+2}; {add {x}+3}
define all x
  {sum {x}+0}; {sum {x}+4}
  {sum {x}+8}; {sum {x}+12}
enddef

define hvbjoy = $4212

define loop
  wait{#}:
    lda {hvbjoy}
    bmi wait{#}
enddef

define 'A' = 0x61
define 'B' = 0x62

org $8000
  {loop}
  {loop}

  lda.b #%11000011
  lda.b #5 % %11

  {all $20}; {all $30}
  db "ABCD"
  dw $8000 + $2000
  incbin "include.bin", 0x0000, 0x0004
  incsrc "include.asm"

