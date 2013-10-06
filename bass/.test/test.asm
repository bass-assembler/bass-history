arch snes.cpu
output "test.bin", create
include "source.asm"

macro text(value) {
  print {value}
}

macro hex(value) {
  if {value} == 0 {
    define result(0)
  } else {
    define result()
    while {value} {
      evaluate nibble({value} & 15)
      evaluate value({value} >> 4)
      if {nibble} <=  9 {; define result({nibble}{result}); }
      if {nibble} == 10 {; define result(a{result}); }
      if {nibble} == 11 {; define result(b{result}); }
      if {nibble} == 12 {; define result(c{result}); }
      if {nibble} == 13 {; define result(d{result}); }
      if {nibble} == 14 {; define result(e{result}); }
      if {nibble} == 15 {; define result(f{result}); }
    }
  }
  print "0x{result}"
}

text("Program counter is at: "); hex(pc()); text("\n")
