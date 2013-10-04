macro seek(offset) {
  origin (({offset} & 0x7f0000) >> 1) | ({offset} & 0x7fff)
  base {offset}
}

seek(0x8000)

//functional recursion

macro factorial(number, result) {
  if {number} <= 1 {
    print "{result}\n"
  } else {
    evaluate result({number} * {result})
    evaluate number({number} - 1)
    factorial({number}, {result})
  }
}

macro factorial(number) {
  factorial({number}, 1)
}

factorial(10)

//iterative repetition

macro factorial(number) {
  evaluate result(1)
  while {number} > 1 {
    evaluate result({result} * {number})
    evaluate number({number} - 1)
  }
  print "{result}\n"
}

factorial(10)

//benchmarking (processing speed)

define n(0)
while {n} < 100000 {
  evaluate n({n} + 1)
}

insert "insert.bin"

scope main: {
  ldx #$0008; ldy #<8
  loop:; dex; bne loop
  -; beq +; lsr; dex; bne -; +
  rts
}

jmp main
jmp main.loop

table.assign 'A', 0x01, 26
table.assign 'a', 0x21, 26
table.assign '0', 0x41, 10

db "ABCabc012", +$10, -$10

lda #$55
lda #%01010101
