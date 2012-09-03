arch snes.cpu

org $0000; fill $8000
org $0000

define x

if {defined y}
   lda #$aa
else
  lda #${x}55aa
endif
