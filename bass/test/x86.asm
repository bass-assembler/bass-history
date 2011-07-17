org 0

main:
  //Implied
  pushad
  popad

  //Register
  push eax
  push ax

  //Effective
  inc <[eax]
  inc >[eax]
  inc ^[eax]
  inc  [eax]

  //Relative
  jmp <main
  jmp >main
  jmp ^main
  jmp  main

  //EffectiveRegister
  add [eax],al
  add al,al
  add [eax],eax
  add eax,eax
  add [es:eax*4+ebp+<0x20],ax
  arpl [eax],ax

  //RegisterEffective
  add al,[eax]
  add eax,[eax]
  add ax,[es:eax*4+ebp+<0x20]

  //AccumulatorImmediate
  add al,0x01
  add ax,0x0123
  add eax,0x01234567

  //Immediate
  push <0x01
  push >0x0123
  push ^0x01234567
  push  0x01234567

  //EffectiveImmediate
  add bl,<0x55
  add bx,>0x0123
  add ebx,^0x01234567
  add >[eax],<0x55

  //RegisterEffectiveImmediate
  imul eax,[es:ebx*8+ebp+0x01234567],0x89abcdef
  imul ax,[bx+si],>0x0123
  imul eax,[eax],<0x01

  mov [eax],al
  mov [eax],ax
  mov [eax],eax

  mov al,[eax]
  mov ax,[eax]
  mov eax,[eax]

  pop >[eax]
  pop ^[eax]
  pop  [eax]

