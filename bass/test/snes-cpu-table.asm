arch snes.cpu

macro seek n
  org (({n} & 0x7f0000) >> 1) | ({n} & 0x7fff)
  base {n}
endmacro

{seek $808000}; fill $8000
{seek $808000}

bank0x:
  brk #$55
  ora ($55,x)
  cop #$55
  ora $55,s
  tsb $55
  ora $55
  asl $55
  ora [$55]
  php
  ora #$55
  asl
  phd
  tsb $55aa
  ora $55aa
  asl $55aa
  ora $55aaff

bank1x:
  bpl bank1x
  ora ($55),y
  ora ($55)
  ora ($55,s),y
  trb $55
  ora $55,x
  asl $55,x
  ora [$55],y
  clc
  ora $55aa,y
  inc
  tas
  trb $55aa
  ora $55aa,x
  asl $55aa,x
  ora $55aaff,x

bank2x:
  jsr $55aa
  and ($55,x)
  jsl $55aaff
  and $55,s
  bit $55
  and $55
  rol $55
  and [$55]
  plp
  and #$55
  rol
  pld
  bit $55aa
  and $55aa
  rol $55aa
  and $55aaff

bank3x:
  bmi bank3x
  and ($55),y
  and ($55)
  and ($55,s),y
  bit $55,x
  and $55,x
  rol $55,x
  and [$55],y
  sec
  and $55aa,y
  dec
  tsa
  bit $55aa,x
  and $55aa,x
  rol $55aa,x
  and $55aaff,x

bank4x:
  rti
  eor ($55,x)
  wdm #$55
  eor $55,s
  mvp $aa=$55
  eor $55
  lsr $55
  eor [$55]
  pha
  eor #$55
  lsr
  phk
  jmp $55aa
  eor $55aa
  lsr $55aa
  eor $55aaff

bank5x:
  bvc bank5x
  eor ($55),y
  eor ($55)
  eor ($55,s),y
  mvn $aa=$55
  eor $55,x
  lsr $55,x
  eor [$55],y
  cli
  eor $55aa,y
  phy
  tad
  jml $55aaff
  eor $55aa,x
  lsr $55aa,x
  eor $55aaff,x

bank6x:
  rts
  adc ($55,x)
  per $55aa
  adc $55,s
  stz $55
  adc $55
  ror $55
  adc [$55]
  pla
  adc #$55
  ror
  rtl
  jmp ($55aa)
  adc $55aa
  ror $55aa
  adc $55aaff

bank7x:
  bvs bank7x
  adc ($55),y
  adc ($55)
  adc ($55,s),y
  stz $55,x
  adc $55,x
  ror $55,x
  adc [$55],y
  sei
  adc $55aa,y
  ply
  tda
  jmp ($55aa,x)
  adc $55aa,x
  ror $55aa,x
  adc $55aaff,x

bank8x:
  bra bank8x
  sta ($55,x)
  brl bank8x
  sta $55,s
  sty $55
  sta $55
  stx $55
  sta [$55]
  dey
  bit #$55
  txa
  phb
  sty $55aa
  sta $55aa
  stx $55aa
  sta $55aaff

bank9x:
  bcc bank9x
  sta ($55),y
  sta ($55)
  sta ($55,s),y
  sty $55,x
  sta $55,x
  stx $55,y
  sta [$55],y
  tya
  sta $55aa,y
  txs
  txy
  stz $55aa
  sta $55aa,x
  stz $55aa,x
  sta $55aaff,x

bankax:
  ldy #$55
  lda ($55,x)
  ldx #$55
  lda $55,s
  ldy $55
  lda $55
  ldx $55
  lda [$55]
  tay
  lda #$11
  tax
  plb
  ldy $55aa
  lda $55aa
  ldx $55aa
  lda $55aaff

bankbx:
  bcs bankbx
  lda ($55),y
  lda ($55)
  lda ($55,s),y
  ldy $55,x
  lda $55,x
  ldx $55,y
  lda [$55],y
  clv
  lda $55aa,y
  tsx
  tyx
  ldy $55aa,x
  lda $55aa,x
  ldx $55aa,y
  lda $55aaff,x

bankcx:
  cpy #$55
  cmp ($55,x)
  rep #$55
  cmp $55,s
  cpy $55
  cmp $55
  dec $55
  cmp [$55]
  iny
  cmp #$55
  dex
  wai
  cpy $55aa
  cmp $55aa
  dec $55aa
  cmp $55aaff

bankdx:
  bne bankdx
  cmp ($55),y
  cmp ($55)
  cmp ($55,s),y
  pei ($55)
  cmp $55,x
  dec $55,x
  cmp [$55],y
  cld
  cmp $55aa,y
  phx
  stp
  jmp [$55aa]
  cmp $55aa,x
  dec $55aa,x
  cmp $55aaff,x

bankex:
  cpx #$55
  sbc ($55,x)
  sep #$55
  sbc $55,s
  cpx $55
  sbc $55
  inc $55
  sbc [$55]
  inx
  sbc #$55
  nop
  xba
  cpx $55aa
  sbc $55aa
  inc $55aa
  sbc $55aaff

bankfx:
  beq bankfx
  sbc ($55),y
  sbc ($55)
  sbc ($55,s),y
  pea $55aa
  sbc $55,x
  inc $55,x
  sbc [$55],y
  sed
  sbc $55aa,y
  plx
  xce
  jsr ($55aa,x)
  sbc $55aa,x
  inc $55aa,x
  sbc $55aaff,x
