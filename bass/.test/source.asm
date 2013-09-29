arch snes.cpu

macro seek(offset) {
  origin (({offset} & 0x7f0000) >> 1) | ({offset} & 0x7fff)
  base {offset}
}

seek(0x8000)

macro factorial(number, result) {
  if {number} <= 1 {
    print "{result}\n"
  } else {
    evaluate newResult({number} * {result})
    evaluate newNumber({number} - 1)
    factorial({newNumber}, {newResult})
  }
}

macro factorial(number) {
  factorial({number}, 1)
}

factorial(10)

define n(0)
while {n} < 100000 {
  evaluate n({n} + 1)
}

insert "insert.bin"

main: {
  ldx #$0008; ldy #<8
  loop:; dex; bne loop
  -; beq +; lsr; dex; bne -; +
  rts
}

jmp main.loop

table.assign 'A', 0x01, 26
table.assign 'a', 0x21, 26
table.assign '0', 0x41, 10

db "ABCabc012"
