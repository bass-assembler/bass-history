arch snes.cpu

org $0000; fill $8000
org $0000

macro test level
  print "{level}"
  if {level} == 1
    print "Almost finished"
    eval step {level} - 1
    {test {eval step}}
  elseif {level} > 0
    eval step {level} - 1
    {test {eval step}}
  endif
endmacro

main:
   {test 10}
