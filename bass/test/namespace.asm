mapper lorom
org $8000; fill $8000

org $8000

namespace render
define offset $000000
define offset $7fff00
macro length
  $000000
endmacro
macro length
  $7fff02
endmacro

namespace global
define offset $7fff04
macro length
  $7fff06
endmacro

namespace global

lda {render::offset}
lda {render::length}
lda {offset}
lda {length}

