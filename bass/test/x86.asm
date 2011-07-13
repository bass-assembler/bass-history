org 0

main:
  add [es:ebx*8+ebp+<0x01234567],<0x89abcdef

  inc eax
  inc ax
  inc al
  inc ebx
  inc bx
  inc bl
  inc [es:eax*4+ebx+0x01234567]

  add ebx,[es:eax*4+ebp+0x01234566+1]
  add ebx,[eax+0x01234567]
  add eax,eax
  add ax,ax
  add al,al
  add eax,0x01234567
//add ax,0x0123
  add al,0x01
  add bl,0x01

