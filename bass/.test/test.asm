arch snes.cpu

origin $0000; fill $10000
origin $0000

macro echo counter
self::main:
  lda #<{self::counter}
  print "{hex self::main}: {hex {self::counter}}; {self::counter}"
  if {self::counter} > 0
  //{echo {self::counter}-1}
    {echo {eval {self::counter}-1}}
  else
    print "Finished"
  endif
endmacro

macro foo value
  namespace self
    print "{value}"
  endnamespace
endmacro

macro bar value
  namespace self
    {global::foo {value}}
  endnamespace
endmacro

main:
  {echo 10}
  {bar hello}
  {foo goodbye}

  echo "{$}\n"
  incbinas binary, "include.bin"
  print "{hex binary}: {eval binary.size}; {hex binary+binary.size}"
  print "{$}"
