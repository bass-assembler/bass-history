//bass -Dvalue=0x56 -o test/test.bin test/test.asm

arch snes.cpu

org $0000; fill $8000
org $0000
  db $12,$34,{value}
