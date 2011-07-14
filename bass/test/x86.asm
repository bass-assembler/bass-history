org 0

main:
  bound eax,[eax]
  bound ax,[bx+si]
//arpl [eax],ax

  add eax,0x01234567
  add ax,0x0123
  add al,0x01
  add bl,0x01

  push< 0xff
  push> 0xffff
  push^ 0xffffffff

  add< [eax],0xff
  add> [eax],0xffff
  add^ [eax],0xffffffff

  add> [eax+<-1],0x0123

  add> [eax],<0xff
  add^ [eax],<0xff

  inc eax
  inc ax
  inc al
  inc ebx
  inc bx
  inc bl
  inc> [es:eax*4+ebx+0x01234567]

  add ebx,[es:eax*4+ebp+0x01234566+1]
  add ebx,[eax+0x01234567]
  add eax,eax
  add ax,ax
  add al,al

  jz< main
  jz> main
  jz^ main

