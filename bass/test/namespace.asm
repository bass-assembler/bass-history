mapper lorom
org $8000; fill $8000

org $8000

namespace render
define offset = $000000
define offset = $7fff00
define length
  $000000
enddef
define length
  $7fff02
enddef

namespace global
define offset = $7fff04
define length
  $7fff06
enddef

namespace global

lda {render::offset}
lda {render::length}
lda {offset}
lda {length}

