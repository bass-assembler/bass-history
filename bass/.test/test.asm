arch snes.cpu
output "test.bin", create

origin $0000; fill $10000
origin $0000

macro add x,y
  return {eval {self::x} + {self::y}}
endmacro

macro factorial n
  if {self::n} == 1
    return 1
  else
    return {eval {self::n} * {factorial {eval {self::n} - 1}}}
  endif
endmacro

macro echo text
  print {self::text}
endmacro

main:
  print "{add {add 1,2}, {add 3,4}}! = {factorial 10}"
  {echo "hello, world!"}
