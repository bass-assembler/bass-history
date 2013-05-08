arch snes.cpu
output "test.bin", create

origin $0000; fill $10000
origin $0000; base $8000

macro add x, y
  return {eval {#x} + {#y}}
endmacro

macro factorial n
  if {#n} == 1
    return 1
  else
    return {eval {#n} * {factorial {eval {#n} - 1}}}
  endif
endmacro

macro vsync
  #loop:
    beq #.end
    bra #loop
  #.end:
endmacro

main:
  print "{add {add 1, 2}, {add 3, 4}}! = {factorial 10}\n"
  {vsync}; {vsync}
