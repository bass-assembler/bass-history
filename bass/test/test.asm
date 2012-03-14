arch snes.cpu

define flag 1

org $0000; fill $8000
org $0000

macro test
  if {flag}
    db $12,$34
  else
    db $56,$78
  endif
endmacro

{test}
