arch snes.cpu

org $0000; fill $8000
define word >

macro seek addr
   origin (({addr} & 0x7f0000) >> 1) | ({addr} & 0x7fff)
   base {addr}
endmacro

{seek $808000}

hook:

push origin, base

{seek $879fe4}
   jsl hook
   bra {+}
   nop
+

pull base, origin

test:
   lda #{word}test
   nop

