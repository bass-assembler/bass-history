macro seek(offset) {
  origin (({offset} & 0x7f0000) >> 1) | ({offset} & 0x7fff)
  base {offset}
}

seek(0x8000)

//functional recursion

macro factorial(number, result) {
  if {number} <= 1 {
    variable result({result})
  } else {
    evaluate result({number} * {result})
    evaluate number({number} - 1)
    factorial({number}, {result})
  }
}

macro factorial(number): {
  factorial({number}, 1)
}

factorial(10)
print factorial.result, "\n"

//iterative repetition

macro factorial(number): {
  variable result(1)
  define :result(1)
  while {number} > 1 {
    variable result(result * {number})
    evaluate :result({:result} * {number})
    evaluate number({number} - 1)
  }
}

factorial(10)
print "{factorial.result}, ", factorial.result, "\n"

//benchmarking (processing speed)

variable n(0)
while n < 100000 {
  variable n(n + 1)
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

map 'A', 0x01, 26
map 'a', 0x21, 26
map '0', 0x41, 10

db "ABCabc012", +$10, -$10

lda #$55
lda #%01010101

if pc() >= 0x9000 {
  error "output exceeds 4KB"
} else if pc() >= 0x8020 {
  warning "output exceeds 32B"
}

//assemble with -d message=... to print a message
if defined(message) {
  print "{message}\n"
}
