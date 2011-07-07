org $0000; fill $10000

org $0000; base $8000

_0x:
  nop
  jsrt 0
  set1 $55.0
  bbs1 $55.0,_0x
  or   $55
  or   $55aa
  or   (x)
  or   ($55+x)

  //0xde
  cbne $55+x,_0x

