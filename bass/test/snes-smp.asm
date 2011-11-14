org $0000; fill $8000

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
  ori $55,$aa
  ori $15aa.2
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
  ori $55,#$aa
  ori (x),(y)
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
  ori !$15aa.2
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
  and $15aa.2
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
  and !$15aa.2
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

_8x:
  sec
  jst 8
  set $55.4
  bbs $55.4,_8x
  adc $55
  adc $55aa
  adc (x)
  adc ($55+x)
  adc #$55
  adc $55,$aa
  eor $15aa.2
  dec $55
  dec $55aa
  ldy #$55
  plp
  sti $55,#$aa

_9x:
  bcc _9x
  jst 9
  clr $55.4
  bbc $55.4,_9x
  adc $55+x
  adc $55aa+x
  adc $55aa+y
  adc ($55)+y
  adc $55,#$aa
  adc (x),(y)
  sbw $55
  dec $55+x
  dec
  tsx
  div
  xcn

_ax:
  sei
  jst 10
  set $55.5
  bbs $55.5,_ax
  sbc $55
  sbc $55aa
  sbc (x)
  sbc ($55+x)
  sbc #$55
  sbc $55,$aa
  ldi $15aa.2
  inc $55
  inc $55aa
  cpy #$55
  pla
  sta (x)+

_bx:
  bcs _bx
  jst 11
  clr $55.5
  bbc $55.5,_bx
  sbc $55+x
  sbc $55aa+x
  sbc $55aa+y
  sbc ($55)+y
  sbc $55,#$aa
  sbc (x),(y)
  ldw $55
  inc $55+x
  inc
  txs
  das
  lda (x)+

_cx:
  cli
  jst 12
  set $55.6
  bbs $55.6,_cx
  sta $55
  sta $55aa
  sta (x)
  sta ($55+x)
  cpx #$55
  stx $55aa
  sti $15aa.2
  sty $55
  sty $55aa
  ldx #$55
  plx
  mul

_dx:
  bne _dx
  jst 13
  clr $55.6
  bbc $55.6,_dx
  sta $55+x
  sta $55aa+x
  sta $55aa+y
  sta ($55)+y
  stx $55
  stx $55+y
  stw $55
  sty $55+x
  dey
  tya
  cbne $55+x,_dx
  daa

_ex:
  clv
  jst 14
  set $55.7
  bbs $55.7,_ex
  lda $55
  lda $55aa
  lda (x)
  lda ($55+x)
  lda #$55
  ldx $55aa
  not $15aa.2
  ldy $55
  ldy $55aa
  not
  ply
  wai

_fx:
  beq _fx
  jst 15
  clr $55.7
  bbc $55.7,_fx
  lda $55+x
  lda $55aa+x
  lda $55aa+y
  lda ($55)+y
  ldx $55
  ldx $55+y
  sti $55,$aa
  ldy $55+x
  iny
  tay
  dbn y,_fx
  stp

