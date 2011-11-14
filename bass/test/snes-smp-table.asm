arch snes.smp
org $0000; fill $8000
org $0000

bank0x:
  nop
  jst 0
  set $55:0
  bbs $55:0=bank0x
  ora $55
  ora $55aa
  ora (x)
  ora ($55,x)
  ora #$55
  orr $55=$aa
  orc $15aa:2
  asl $55
  asl $55aa
  php
  tsb $55aa
  brk

bank1x:
  bpl bank1x
  jst 1
  clr $55:0
  bbc $55:0=bank1x
  ora $55,x
  ora $55aa,x
  ora $55aa,y
  ora ($55),y
  orr $55=#$aa
  orr (x)=(y)
  dew $55
  asl $55,x
  asl
  dex
  cpx $55aa
  jmp ($55aa,x)

bank2x:
  clp
  jst 2
  set $55:1
  bbs $55:1=bank2x
  and $55
  and $55aa
  and (x)
  and ($55,x)
  and #$55
  and $55=$aa
  orc !$15aa:2
  rol $55
  rol $55aa
  pha
  bne $55=bank2x
  bra bank2x

bank3x:
  bmi bank3x
  jst 3
  clr $55:1
  bbc $55:1=bank3x
  and $55,x
  and $55aa,x
  and $55aa,y
  and ($55),y
  and $55=#$aa
  and (x)=(y)
  inw $55
  rol $55,x
  rol
  inx
  cpx $55
  jsr $55aa

bank4x:
  sep
  jst 4
  set $55:2
  bbs $55:2=bank4x
  eor $55
  eor $55aa
  eor (x)
  eor ($55,x)
  eor #$55
  eor $55=$aa
  and $15aa:2
  lsr $55
  lsr $55aa
  phx
  trb $55aa
  jsp $55

bank5x:
  bvc bank5x
  jst 5
  clr $55:2
  bbc $55:2=bank5x
  eor $55,x
  eor $55aa,x
  eor $55aa,y
  eor ($55),y
  eor $55=#$aa
  eor (x)=(y)
  cpw $55
  lsr $55,x
  lsr
  tax
  cpy $55aa
  jmp $55aa

bank6x:
  clc
  jst 6
  set $55:3
  bbs $55:3=bank6x
  cmp $55
  cmp $55aa
  cmp (x)
  cmp ($55,x)
  cmp #$55
  cmp $55=$aa
  and !$15aa:2
  ror $55
  ror $55aa
  phy
  bnz $55=bank6x
  rts

bank7x:
  bvs bank7x
  jst 7
  clr $55:3
  bbc $55:3=bank7x
  cmp $55,x
  cmp $55aa,x
  cmp $55aa,y
  cmp ($55),y
  cmp $55=#$aa
  cmp (x)=(y)
  adw $55
  ror $55,x
  ror
  txa
  cpy $55
  rti

bank8x:
  sec
  jst 8
  set $55:4
  bbs $55:4=bank8x
  adc $55
  adc $55aa
  adc (x)
  adc ($55,x)
  adc #$55
  adc $55=$aa
  eor $15aa:2
  dec $55
  dec $55aa
  ldy #$55
  plp
  str $55=#$aa

bank9x:
  bcc bank9x
  jst 9
  clr $55:4
  bbc $55:4=bank9x
  adc $55,x
  adc $55aa,x
  adc $55aa,y
  adc ($55),y
  adc $55=#$aa
  adc (x)=(y)
  sbw $55
  dec $55,x
  dec
  tsx
  div
  xcn

bankax:
  sei
  jst 10
  set $55:5
  bbs $55:5=bankax
  sbc $55
  sbc $55aa
  sbc (x)
  sbc ($55,x)
  sbc #$55
  sbc $55=$aa
  ldc $15aa:2
  inc $55
  inc $55aa
  cpy #$55
  pla
  sta (x)+

bankbx:
  bcs bankbx
  jst 11
  clr $55:5
  bbc $55:5=bankbx
  sbc $55,x
  sbc $55aa,x
  sbc $55aa,y
  sbc ($55),y
  sbc $55=#$aa
  sbc (x)=(y)
  ldw $55
  inc $55,x
  inc
  txs
  das
  lda (x)+

bankcx:
  cli
  jst 12
  set $55:6
  bbs $55:6=bankcx
  sta $55
  sta $55aa
  sta (x)
  sta ($55,x)
  cpx #$55
  stx $55aa
  stc $15aa:2
  sty $55
  sty $55aa
  ldx #$55
  plx
  mul

bankdx:
  bne bankdx
  jst 13
  clr $55:6
  bbc $55:6=bankdx
  sta $55,x
  sta $55aa,x
  sta $55aa,y
  sta ($55),y
  stx $55
  stx $55,y
  stw $55
  sty $55,x
  dey
  tya
  bne $55,x=bankdx
  daa

bankex:
  clv
  jst 14
  set $55:7
  bbs $55:7=bankex
  lda $55
  lda $55aa
  lda (x)
  lda ($55,x)
  lda #$55
  ldx $55aa
  not $15aa:2
  ldy $55
  ldy $55aa
  cmc
  ply
  wai

bankfx:
  beq bankfx
  jst 15
  clr $55:7
  bbc $55:7=bankfx
  lda $55,x
  lda $55aa,x
  lda $55aa,y
  lda ($55),y
  ldx $55
  ldx $55,y
  str $55=$aa
  ldy $55,x
  iny
  tay
  bnz y=bankfx
  stp

