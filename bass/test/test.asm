arch snes.cpu

org $0000; fill $8000
org $0000

main:
   lda #$aa
   lda #$55aa

   ora.b #$aa
   ora.w #$55aa

