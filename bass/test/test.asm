arch snes.cpu

org $0000; fill $8000
org $0000

macro usart_read serial
  lda {serial}
  -; beq {-}
endmacro

usart1_readb:
  {usart_read $16}
