macro demo data
  ldx.b #{self::data}
endmacro

macro test data
  lda.b #{self::data}
  {demo {self::data}+1}
  eval self::counter {self::data}+1
  print "{hex self::counter}"

self::label:
  bra self::label
endmacro
