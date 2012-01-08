arch "../arch/table/snes-cpu.arch"

macro append name, value
  arch.append "{name} #*16; {value} =a"
  arch.append "{name} #*08; {value} =a"
endmacro

{append add, $18 $69}
{append sub, $38 $e9}

org $0000; fill $8000
org $0000
  dq 0x123456789abcdef0
  add #~0
  add #<~0
  sub #~0
  sub #<~0
  asl #4

