mapper lorom
org $8000; fill $8000

define base $80
macro add x; clc; adc> #{base}+{x}; endmacro
macro sum x; {add {x}+0}; {add {x}+1}; {add {x}+2}; {add {x}+3}; endmacro
macro all x,multiplier
  {sum {x}+0*{multiplier}}; {sum {x}+1*{multiplier}}
  {sum {x}+2*{multiplier}}; {sum {x}+3*{multiplier}}
endmacro

define hvbjoy $4212

macro loop
  wait{#}:
    lda {hvbjoy}
    bmi wait{#}
endmacro

define 'A' 0x61
define 'B' 0x62

org $8000; base $7ec000
  {all $20, 4}; {all $30, 4}
  {add $20}

  nop #4
main:

print "{pc}"
pushpc
org $8100
  db $01,$02,$40,$80
pullpc

  jml main

  {loop}
  {loop}

  lda< #%11000011
  lda< #5 % %11

  db "ABCD"
  dw $8000 + $2000
  incbin "include.bin", 0x0000, 0x0004
  incsrc "include.asm"
