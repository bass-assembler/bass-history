mapper lorom
org $008000; fill $8000

define pushall php; rep #$30; pha; phb; phd; phx; phy
define pullall rep #$30; ply; plx; pld; plb; pla; plp

macro square(n)
  {n}*{n}
endmacro

macro fill(addr, length, byte)
  lda.b #{byte}
  ldx.w {addr}
  -; sta $0000,x; dey; bne -
  rts
endmacro

org $008000
  dw {square(8)}
  {fill($4000, $2000, $ff)}
  incsrc "test/include.asm"
  incbin "test/include.bin"
  {pushall}
  {pullall}

