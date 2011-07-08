org $0000; fill $10000

org $0000; base $8000

_0x:
  nop               //nop
  tcall 0           //jsrt n
  set1  $11.0       //set1 $dp.0
  bbs1  $11.0,_0x   //bbs1 $dp.0,$rl
  or    a,$11       //or   $dp
  or    a,$1122     //or   $addr
  or    a,(x)       //or   (x)
  or    a,($11+x)   //or   ($dp+x)
  or    a,#$11      //or   #$im
  or    $11,$22     //or   $dp,$sp
  or1   c,$1122.0   //or1  $addr.b
  asl   $11         //asl  $dp
  asl   $1122       //asl  $addr
  push  p           //php
  tset  $1122,a     //tsb  $1122
  brk               //brk

_1x:
  bpl   _1x         //bpl  $rl
  tcall 1           //jsrt n
  clr1  $11.0       //clr1 $dp.0
  bbc1  $11.0,_1x   //bbc1 $dp.0,$rl
  or    a,$11+x     //or   $dp+x
  or    a,$1122+x   //or   $addr+x
  or    a,$1122+y   //or   $addr+y
  or    a,($11)+y   //or   ($dp)+y
  or    $11,#$22    //or   $dp,#$im
  or    (x),(y)     //or   (x),(y)
  decw  $11         //decw $dp
  asl   $11+x       //asl  $dp+x
  asl   a           //asl
  dec   x           //dex
  cmp   x,$1122     //cpx  $addr
  jmp   ($1122+x)   //jmp  ($addr+x)

_2x:
  clrp              //clp
  tcall 2           //jsrt 2
  set1  $11.1       //set1 $dp.1
  bbs1  $11.1,_2x   //bbs1 $dp.1,$rl
  and   a,$11       //and  $dp
  and   a,$1122     //and  $addr
  and   a,(x)       //and  (x)
  and   a,($11+x)   //and  ($dp+x)
  and   a,#$11      //and  #$im
  and   $11,$22     //and  $dp,$sp
  or1   c,!$1122.0  //or1  !$addr.b
  rol   $11         //rol  $dp
  rol   $1122       //rol  $addr
  push  a           //pha
  cbne  $11,_2x     //cbne $dp,$rl
  bra   _2x         //bra  $rl

_3x:
  bmi   _3x         //bmi  $rl
  tcall 3           //jsrt 3
  clr1  $11.1       //clr1 $dp.1
  bbc1  $11.1,_3x   //bbc1 $dp.1,$rl
  and   a,$11+x     //and  $dp+x
  and   a,$1122+x   //and  $addr+x
  and   a,$1122+y   //and  $addr+y
  and   a,($11)+y   //and  ($dp)+y
  and   $11,#$22    //and  $dp,#$im
  and   (x),(y)     //and  (x),(y)
  incw  $11         //incw $dp
  rol   $11+x       //rol  $dp+x
  rol   a           //rol
  inc   x           //inx
  cmp   x,$11       //cpx  $dp
  call  $1122       //jsr  $addr

_4x:
  setp              //sep
  tcall 4           //jsrt 4
  set1  $11.2       //set1 $dp.2
  bbs1  $11.2,_4x   //bbs1 $dp.2,$rl
  eor   a,$11       //eor  $dp
  eor   a,$1122     //eor  $addr
  eor   a,(x)       //eor  (x)
  eor   a,($11+x)   //eor  ($dp+x)
  eor   a,#$11      //eor  #$im
  eor   $11,$22     //eor  $dp,$sp
  and1  c,$1122.0   //and1 $addr.b
  lsr   $11         //lsr  $dp
  lsr   $1122       //lsr  $addr
  push  x           //phx
  tclr  $1122,a     //trb  $addr
  pcall $11         //jsrp $nn

_5x:
  bvc   _5x         //bvc  $rl
  tcall 5           //jsrt 5
  clr1  $11.2       //clr1 $dp.2
  bbc1  $11.2,_5x   //bbc1 $dp.2,$rl
  eor   a,$11+x     //eor  $dp+x
  eor   a,$1122+x   //eor  $addr+x
  eor   a,$1122+y   //eor  $addr+y
  eor   a,($11)+y   //eor  ($dp)+y
  eor   $11,#$22    //eor  $dp,#$im
  eor   (x),(y)     //eor  (x),(y)
  cmpw  ya,$11      //cmpw $dp
  lsr   $11+x       //lsr  $dp+x
  lsr   a           //lsr
  mov   x,a         //tax
  cmp   y,$1122     //cpy  $addr
  jmp   $1122       //jmp  $addr

_6x:
  clrc              //clc
  tcall 6           //jsrt 6
  set1  $11.3       //set1 $dp.3
  bbs1  $11.3,_6x   //bbs1 $dp.3,$rl
  cmp   a,$11       //cmp  $dp
  cmp   a,$1122     //cmp  $addr
  cmp   a,(x)       //cmp  (x)
  cmp   a,($11+x)   //cmp  ($dp+x)
  cmp   a,#$11      //cmp  #$im
  cmp   $11,$22     //cmp  $dp,$sp
  and1  c,!$1122.0  //and1 !$addr.b
  ror   $11         //ror  $dp
  ror   $1122       //ror  $addr
  push  y           //phy
  dbnz  $11,_6x     //dbnz $dp,$rl
  ret               //rts

_7x:
  bvs   _7x         //bvs  $rl
  tcall 7           //jsrt 7
  clr1  $11.3       //clr1 $dp.3
  bbc1  $11.3,_7x   //bbc1 $dp.3,$rl
  cmp   a,$11+x     //cmp  $dp+x
  cmp   a,$1122+x   //cmp  $addr+x
  cmp   a,$1122+y   //cmp  $addr+y
  cmp   a,($11)+y   //cmp  ($dp)+y
  cmp   $11,#$22    //cmp  $dp,#$im
  cmp   (x),(y)     //cmp  (x),(y)
  addw  ya,$11      //addw $dp
  ror   $11+x       //ror  $dp+x
  ror   a           //ror
  mov   a,x         //txa
  cmp   y,$11       //cpy  $dp
  reti              //rti

_8x:
  setc              //sec
  tcall 8           //jsrt 8
  set1  $11.4       //set1 $dp.4
  bbs1  $11.4,_8x   //bbs1 $dp.4,$rl
  adc   a,$11       //adc  $dp
  adc   a,$1122     //adc  $addr
  adc   a,(x)       //adc  (x)
  adc   a,($11+x)   //adc  ($dp+x)
  adc   a,#$11      //adc  #$im
  adc   $11,$22     //adc  $dp,$sp
  eor1  c,$1122.0   //eor1 $addr.b
  dec   $11         //dec  $dp
  dec   $1122       //dec  $addr
  mov   y,#$11      //ldy  #$11
  pop   p           //plp
  mov   $11,#$22    //st   $dp,#$im

_9x:
  bcc   _9x         //bcc  $rl
  tcall 9           //jsrt 9
  clr1  $11.4       //clr1 $dp.4
  bbc1  $11.4,_8x   //bbc1 $dp.4,$rl
  adc   a,$11+x     //adc  $dp+x
  adc   a,$1122+x   //adc  $addr+x
  adc   a,$1122+y   //adc  $addr+y
  adc   a,($11)+y   //adc  ($dp)+y
  adc   $11,#$22    //adc  $dp,#$im
  adc   (x),(y)     //adc  (x),(y)
  subw  $11         //subw $dp
  dec   $11+x       //dec  $dp+x
  dec   a           //dec
  mov   x,sp        //tsx
  div   ya,x        //div
  xcn   a           //xcn

_ax:
  ei                //sei
  tcall 10          //jsrt 10
  set1  $11.5       //set1 $dp.5
  bbs1  $11.5,_ax   //bbs1 $dp.5,$rl
  sbc   a,$11       //sbc  $dp
  sbc   a,$1122     //sbc  $addr
  sbc   a,(x)       //sbc  (x)
  sbc   a,($11+x)   //sbc  ($dp+x)
  sbc   a,#$11      //sbc  #$im
  sbc   $11,$22     //sbc  $dp,$sp
  mov1  c,$1122.0   //mov1 $addr.b
  inc   $11         //inc  $dp
  inc   $1122       //inc  $addr
  cmp   y,#$11      //cpy  #$im
  pop   a           //pla
  mov   (x)+,a      //sta  (x)+

_bx:
  bcs   _bx         //bcs  $rl
  tcall 11          //jsrt 11
  clr1  $11.5       //clr1 $dp.5
  bbc1  $11.5,_bx   //bbc1 $dp.5,$rl
  sbc   a,$11+x     //sbc  $dp+x
  sbc   a,$1122+x   //sbc  $addr+x
  sbc   a,$1122+y   //sbc  $addr+y
  sbc   a,($11)+y   //sbc  ($dp)+y
  sbc   $11,#$22    //sbc  $dp,#$im
  sbc   (x),(y)     //sbc  (x),(y)
  movw  $11         //movw $dp
  inc   $11+x       //inc  $dp+x
  inc   a           //inc
  mov   sp,x        //txs
  das   a           //das
  mov   a,(x)+      //lda  (x)+

_cx:
  di                //cli
  tcall 12          //jsrt 12
  set1  $11.6       //set1 $dp.6
  bbs1  $11.6,_cx   //bbs1 $dp.6,$rl
  mov   $11,a       //sta  $dp
  mov   $1122,a     //sta  $addr
  mov   (x),a       //sta  (x)
  mov   ($11+x),a   //sta  ($dp+x)
  cmp   x,#$11      //cpx  #$im
  mov   $1122,x     //stx  $addr
  mov1  $1122.0,c   //st1  $addr.b
  mov   $11,y       //sty  $dp
  mov   $1122,y     //sty  $addr
  mov   x,#$11      //ldx  #$im
  pop   x           //plx
  mul   ya          //mul

_dx:
  bne   _dx         //bne  $rl
  tcall 13          //jsrt 13
  clr1  $11.6       //clr1 $dp.6
  bbc1  $11.6,_dx   //bbc1 $dp.6,$rl
  mov   $11+x,a     //sta  $dp+x
  mov   $1122+x,a   //sta  $addr+x
  mov   $1122+y,a   //sta  $addr+y
  mov   ($11)+y,a   //sta  ($dp)+y
  mov   $11,x       //stx  $dp
  mov   $11+y,x     //stx  $dp+y
  movw  $11,ya      //stw  $dp
  mov   $11+x,y     //sty  $dp+x
  dec   y           //dey
  mov   a,y         //tya
  cbne  $11+x,_dx   //cbne $dp+x,$rl
  daa   a           //daa

_ex:
  clrv              //clv
  tcall 14          //jsrt 14
  set1  $11.7       //set1 $dp.7
  bbs1  $11.7,_ex   //bbs1 $dp.7,$rl
  mov   a,$11       //lda  $dp
  mov   a,$1122     //lda  $addr
  mov   a,(x)       //lda  (x)
  mov   a,($11+x)   //lda  ($dp+x)
  mov   a,#$11      //lda  #$11
  mov   x,$1122     //ldx  $addr
  not1  c,$1122.0   //not1 $addr.b
  mov   y,$11       //ldy  $dp
  mov   y,$1122     //ldy  $addr
  notc              //notc
  pop   y           //ply
  sleep             //wai

_fx:
  beq  _fx          //beq  $rl
  tcall 15          //jsrt 15
  clr1  $11.7       //clr1 $dp.7
  bbc1  $11.7,_fx   //bbc1 $dp.7,$rl
  mov   a,$11+x     //lda  $dp+x
  mov   a,$1122+x   //lda  $addr+x
  mov   a,$1122+y   //lda  $addr+y
  mov   a,($11)+y   //lda  ($dp)+y
  mov   x,$11       //ldx  $dp
  mov   x,$11+y     //ldx  $dp+y
  mov   $11,$22     //st   $dp,$sp
  mov   y,$11+x     //ldy  $dp+x
  inc   y           //iny
  mov   y,a         //tay
  dbnz  y,_fx       //dbnz y,$rl
  stop              //stp

