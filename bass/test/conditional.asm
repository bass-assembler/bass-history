mapper lorom
org $8000; fill $8000
org $8000

define size = -16

db $01,$02
seek -2
db $03,$04

if 0
  if 0
    lda #$01
  elseif 0
    lda #$02
  else
    lda #$03
  endif
elseif 1
  if 0
    lda #$04
  elseif 1
    lda #$05
  else
    lda #$06
  endif
else
  if 0
    lda #$07
  elseif 0
    lda #$08
  else
    lda #$09
  endif
endif
  lda #$0a

if {size} < 0
  lda #$10
elseif {size} > 0
  lda #$20
else
  lda #$30
endif

