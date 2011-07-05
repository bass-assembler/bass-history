org $0000; fill $10000

org $0000; base $8000

_0x:
  nop
  tcall 0
  set1  $11.0
  bbs   $11.0,_0x
  or    a,$11
  or    a,$1122
  or    a,(x)
  or    a,($11+x)
  or    a,#$11
  or    $11,$22
  or1   c,$1122.0
  asl   $11
  asl   $1122
  push  p
  tset  $1122,a
  brk

_1x:
  bpl   _1x
  tcall 1
  clr1  $11.0
  bbc   $11.0,_1x
  or    a,$11+x
  or    a,$1122+x
  or    a,$1122+y
  or    a,($11)+y
  or    $11,#$22
  or    (x),(y)
  decw  $11
  asl   $11+x
  asl   a
  dec   x
  cmp   x,$1122
  jmp   ($1122+x)

_2x:
  clrp
  tcall 2
  set1  $11.1
  bbs   $11.1,_2x
  and   a,$11
  and   a,$1122
  and   a,(x)
  and   a,($11+x)
  and   a,#$11
  and   $11,$22
  or1   c,!$1122.0
  rol   $11
  rol   $1122
  push  a
  cbne  $11,_2x
  bra   _2x

_3x:
  bmi   _3x
  tcall 3
  clr1  $11.1
  bbc   $11.1,_3x
  and   a,$11+x
  and   a,$1122+x
  and   a,$1122+y
  and   a,($11)+y
  and   $11,#$22
  and   (x),(y)
  incw  $11
  rol   $11+x
  rol   a
  inc   x
  cmp   x,$11
  call  $1122

_4x:
  setp
  tcall 4
  set1  $11.2
  bbs   $11.2,_4x
  eor   a,$11
  eor   a,$1122
  eor   a,(x)
  eor   a,($11+x)
  eor   a,#$11
  eor   $11,$22
  and1  c,$1122.0
  lsr   $11
  lsr   $1122
  push  x
  tclr  $1122,a
  pcall $11

_5x:
  bvc   _5x
  tcall 5
  clr1  $11.2
  bbc   $11.2,_5x
  eor   a,$11+x
  eor   a,$1122+x
  eor   a,$1122+y
  eor   a,($11)+y
  eor   $11,#$22
  eor   (x),(y)
  cmpw  ya,$11
  lsr   $11+x
  lsr   a
  mov   x,a
  cmp   y,$1122
  jmp   $1122

_6x:
  clrc
  tcall 6
  set1  $11.3
  bbs   $11.3,_6x
  cmp   a,$11
  cmp   a,$1122
  cmp   a,(x)
  cmp   a,($11+x)
  cmp   a,#$11
  cmp   $11,$22
  and1  c,!$1122.0
  ror   $11
  ror   $1122
  push  y
  dbnz  $11,_6x
  ret

_7x:
  bvs   _7x
  tcall 7
  clr1  $11.3
  bbc   $11.3,_7x
  cmp   a,$11+x
  cmp   a,$1122+x
  cmp   a,$1122+y
  cmp   a,($11)+y
  cmp   $11,#$22
  cmp   (x),(y)
  addw  ya,$11
  ror   $11+x
  ror   a
  mov   a,x
  cmp   y,$11
  reti

_8x:
  setc
  tcall 8
  set1  $11.4
  bbs   $11.4,_8x

_9x:
  bcc   _9x
  tcall 9
  clr1  $11.4
  bbc   $11.4,_8x

