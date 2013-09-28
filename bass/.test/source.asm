arch snes.cpu

origin 0x0000
base 0x8000

db "hello", 0  //string

function main {
  lda #0x80
  jsr main
  nop #2
}
