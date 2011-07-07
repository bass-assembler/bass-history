org $0000; fill $10000

org $0000; base $8000

_0x:
  nop
  jst 0
  set $55.0
  bbs $55.0,_0x
  ora $55
  ora $55aa
  ora (x)
  ora ($55+x)
  ora #$55
  ora $55,$aa
  ora $55aa.0
  asl $55
  asl $55aa
  php
  tsb $55aa
  brk

_1x:
  bpl _1x
  jst 1
  clr $55.0
  bbc $55.0,_1x
  ora $55+x
  ora $55aa+x
  ora $55aa+y
  ora ($55)+y
  ora $55,#$aa
  ora (x),(y)
  dew $55
  asl $55+x
  asl
  dex
  cpx $55aa
  jmp ($55aa+x)

_2x:
  clp
  jst 2
  set $55.1
  bbs $55.1,_2x
  and $55
  and $55aa
  and (x)
  and ($55+x)
  and #$55
  and $55,$aa
  ora !$55aa.0
  rol $55
  rol $55aa
  pha
  cbn $55,_2x
  bra _2x

_3x:
  bmi _3x
  jst 3
  clr $55.1
  bbc $55.1,_3x
  and $55+x
  and $55aa+x
  and $55aa+y
  and ($55)+y
  and $55,#$aa
  and (x),(y)
  inw $55
  rol $55+x
  rol
  inx
  cpx $55
  jsr $55aa

_4x:
  sep
  jst 4
  set $55.2
  bbs $55.2,_4x
  eor $55
  eor $55aa
  eor (x)
  eor ($55+x)
  eor #$55
  eor $55,$aa
  and $55aa.0
  lsr $55
  lsr $55aa
  phx
  trb $55aa
  jsp $55

_5x:
  bvc _5x
  jst 5
  clr $55.2
  bbc $55.2,_5x
  eor $55+x
  eor $55aa+x
  eor $55aa+y
  eor ($55)+y
  eor $55,#$aa
  eor (x),(y)
  cpw $55
  lsr $55+x
  lsr
  tax
  cpy $55aa
  jmp $55aa

_6x:
  clc
  jst 6
  set $55.3
  bbs $55.3,_6x
  cmp $55
  cmp $55aa
  cmp (x)
  cmp ($55+x)
  cmp #$55
  cmp $55,$aa
  and !$55aa.0
  ror $55
  ror $55aa
  phy
  dbn $55,_6x
  rts

_7x:
  bvs _7x
  jst 7
  clr $55.3
  bbc $55.3,_7x
  cmp $55+x
  cmp $55aa+x
  cmp $55aa+y
  cmp ($55)+y
  cmp $55,#$aa
  cmp (x),(y)
  adw $55
  ror $55+x
  ror
  txa
  cpy $55
  rti

