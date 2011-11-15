//note: this file uses a deprecated architecture
//it will not assemble, but is left to demonstrate functionality

mapper lorom
org $8000; fill $8000

org $8000; base $c000
  lda #3

  ~; beq {+}; lsr; dex; bra {-}; ~
  -; beq {+}; lsr; dex; bra {-}; +

  -; stz $55aa
  -; stz $55aa
  -; stz $55aa

  bra {-3}; bra {-2}; bra {-1}
  bra {+1}; bra {+2}; bra {+3}

  +; stz $55aa
  +; stz $55aa
  +; stz $55aa

  print "PC: {$} [{@}]"
  jmp {$}

