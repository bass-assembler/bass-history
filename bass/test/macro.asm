mapper lorom
org $8000; fill $8000; org $8000

macro add n; clc; adc {n}; endmacro
macro sub n; sec; sbc {n}; endmacro
macro addsub x,y
  {add {x}}
  {sub {y}}
endmacro

define offset $7efff8 + 8
define length 32
define 'A' 0x61
define 'B' 0x62

main:
  {addsub #$20,#$40}
  lda^ offset
  {add #length}
  db "ABCD"

if {pc} >= 0x8040
  error "Out of space! PC = {pc}"
elseif {pc} >= 0x8020
  warning "Almost out of space! PC = {pc}"
endif

macro m1 x
  macro m2 y
    {x} {y}
  endmacro
endmacro

{m1 lda}; {m2 #$ff}

macro test; nop #2; endmacro; {test}

