bool BassSnesSmp::assembleBlock(const string &block_) {
  string block = block_;
  if(Bass::assembleBlock(block) == true) return true;

  signed relative, address, size = 0;
  if(block[3] == '<') { block[3] = ' '; size = 1; }
  if(block[3] == '>') { block[3] = ' '; size = 2; }

  lstring part, byte;
  part.split<1>(" ", block);
  string name = part[0], args = part[1];

  #define isbyte() !( \
    (size == 2 || args.wildcard("$????")) || \
    (!o.priority && size != 1 && !args.wildcard("$??")) \
  )

  #define isword() !( \
    (size == 1 || args.wildcard("$??")) || \
    (!o.priority && size != 2 && !args.wildcard("$????")) \
  )

  foreach(f, family) if(args.wildcard(f.pattern)) {
    foreach(o, f.opcode) if(name == o.name) {
      switch(o.mode) {
      case Mode::Implied:
        write(o.prefix);
        return true;
      case Mode::Vector:
        write(o.prefix | (eval(args) << 4));
        return true;
      case Mode::Immediate:
        args.ltrim<1>("#");
        write(o.prefix);
        write(eval(args));
        return true;
      case Mode::Direct:
        if(isbyte() == false) break;
        write(o.prefix);
        write(eval(args));
        return true;
      case Mode::DirectX:
        args.rtrim<1>("+x");
        if(isbyte() == false) break;
        write(o.prefix);
        write(eval(args));
        return true;
      case Mode::DirectY:
        args.rtrim<1>("+y");
        if(isbyte() == false) break;
        write(o.prefix);
        write(eval(args));
        return true;
      case Mode::DirectBit:
        byte.split(".", args);
        write(o.prefix | (eval(byte[1]) << 5));
        write(eval(byte[0]));
        return true;
      case Mode::DirectBitRelative:
        part.split(",", args);
        byte.split(".", part[0]);
        relative = eval(part[1]) - (pc() + 3);
        if(pass == 2 && (relative < -128 || relative > +127)) error("branch out of bounds");
        write(o.prefix | (eval(byte[1]) << 5));
        write(eval(byte[0]));
        write(relative);
        return true;
      case Mode::IndirectX:
        args.ltrim<1>("(");
        args.rtrim<1>("+x)");
        write(o.prefix);
        write(eval(args));
        return true;
      case Mode::IndirectY:
        args.ltrim<1>("(");
        args.rtrim<1>(")+y");
        write(o.prefix);
        write(eval(args));
        return true;
      case Mode::DirectImmediate:
        part.split(",", args);
        part[1].ltrim<1>("#");
        write(o.prefix);
        write(eval(part[1]));
        write(eval(part[0]));
        return true;
      case Mode::DirectDirect:
        part.split(",", args);
        write(o.prefix);
        write(eval(part[1]));
        write(eval(part[0]));
        return true;
      case Mode::DirectRelative:
        part.split(",", args);
        relative = eval(part[1]) - (pc() + 3);
        if(pass == 2 && (relative < -128 || relative > +127)) error("branch out of bounds");
        write(o.prefix);
        write(eval(part[0]));
        write(relative);
        return true;
      case Mode::DirectXRelative:
        part.split(",", args);
        part[0].rtrim<1>("+x");
        relative = eval(part[1]) - (pc() + 3);
        if(pass == 2 && (relative < -128 || relative > +127)) error("branch out of bounds");
        write(o.prefix);
        write(eval(part[0]));
        write(relative);
        return true;
      case Mode::Absolute:
        if(isword() == false) break;
        write(o.prefix);
        write(eval(args), 2);
        return true;
      case Mode::AbsoluteX:
        args.rtrim<1>("+x");
        if(isword() == false) break;
        write(o.prefix);
        write(eval(args), 2);
        return true;
      case Mode::AbsoluteY:
        args.rtrim<1>("+y");
        if(isword() == false) break;
        write(o.prefix);
        write(eval(args), 2);
        return true;
      case Mode::AbsoluteBit:
        byte.split(".", args);
        address = (eval(byte[1]) << 13) | (eval(byte[0]) & 0x1fff);
        write(o.prefix);
        write(address, 2);
        return true;
      case Mode::AbsoluteBitNot:
        args.ltrim<1>("!");
        byte.split(".", args);
        address = (eval(byte[1]) << 13) | (eval(byte[0]) & 0x1fff);
        write(o.prefix);
        write(address, 2);
        return true;
      case Mode::IndirectAbsoluteX:
        args.ltrim<1>("(");
        args.rtrim<1>("+x)");
        write(o.prefix);
        write(eval(args), 2);
        return true;
      case Mode::Relative:
        relative = eval(args) - (pc() + 2);
        if(pass == 2 && (relative < -128 || relative > +127)) error("branch out of bounds");
        write(o.prefix);
        write(relative);
        return true;
      case Mode::YRelative:
        args.ltrim<1>("y,");
        relative = eval(args) - (pc() + 2);
        if(pass == 2 && (relative < -128 || relative > +127)) error("branch out of bounds");
        write(o.prefix);
        write(relative);
        return true;
      }
    }
  }

  #undef isbyte
  #undef isword

  return false;
}

BassSnesSmp::BassSnesSmp() {
  Opcode table[] = {
    { 0x00, "nop        ", 1, Mode::Implied },
    { 0x01, "jst *      ", 1, Mode::Vector },
    { 0x02, "set *.?    ", 1, Mode::DirectBit },
    { 0x03, "bbs *.?,*  ", 1, Mode::DirectBitRelative },
    { 0x04, "ora *      ", 0, Mode::Direct },
    { 0x05, "ora *      ", 1, Mode::Absolute },
    { 0x06, "ora (x)    ", 1, Mode::Implied },
    { 0x07, "ora (*+x)  ", 1, Mode::IndirectX },
    { 0x08, "ora #*     ", 1, Mode::Immediate },
    { 0x09, "ori *,*    ", 1, Mode::DirectDirect },
    { 0x0a, "ori *.?    ", 1, Mode::AbsoluteBit },
    { 0x0b, "asl *      ", 0, Mode::Direct },
    { 0x0c, "asl *      ", 1, Mode::Absolute },
    { 0x0d, "php        ", 1, Mode::Implied },
    { 0x0e, "tsb *      ", 1, Mode::Absolute },
    { 0x0f, "brk        ", 1, Mode::Implied },

    { 0x10, "bpl *      ", 1, Mode::Relative },
    { 0x12, "clr *.?    ", 1, Mode::DirectBit },
    { 0x13, "bbc *.?,*  ", 1, Mode::DirectBitRelative },
    { 0x14, "ora *+x    ", 0, Mode::DirectX },
    { 0x15, "ora *+x    ", 1, Mode::AbsoluteX },
    { 0x16, "ora *+y    ", 1, Mode::AbsoluteY },
    { 0x17, "ora (*)+y  ", 1, Mode::IndirectY },
    { 0x18, "ori *,#*   ", 1, Mode::DirectImmediate },
    { 0x19, "ori (x),(y)", 1, Mode::Implied },
    { 0x1a, "dew *      ", 1, Mode::Direct },
    { 0x1b, "asl *+x    ", 1, Mode::DirectX },
    { 0x1c, "asl        ", 1, Mode::Implied },
    { 0x1d, "dex        ", 1, Mode::Implied },
    { 0x1e, "cpx *      ", 1, Mode::Absolute },
    { 0x1f, "jmp (*+x)  ", 1, Mode::IndirectAbsoluteX },

    { 0x20, "clp        ", 1, Mode::Implied },
    { 0x24, "and *      ", 0, Mode::Direct },
    { 0x25, "and *      ", 1, Mode::Absolute },
    { 0x26, "and (x)    ", 1, Mode::Implied },
    { 0x27, "and (*+x)  ", 1, Mode::IndirectX },
    { 0x28, "and #*     ", 1, Mode::Immediate },
    { 0x29, "and *,*    ", 1, Mode::DirectDirect },
    { 0x2a, "ori !*.?   ", 1, Mode::AbsoluteBitNot },
    { 0x2b, "rol *      ", 0, Mode::Direct },
    { 0x2c, "rol *      ", 1, Mode::Absolute },
    { 0x2d, "pha        ", 1, Mode::Implied },
    { 0x2e, "cbn *,*    ", 1, Mode::DirectRelative },
    { 0x2f, "bra *      ", 1, Mode::Relative },

    { 0x30, "bmi *      ", 1, Mode::Relative },
    { 0x34, "and *+x    ", 0, Mode::DirectX },
    { 0x35, "and *+x    ", 1, Mode::AbsoluteX },
    { 0x36, "and *+y    ", 1, Mode::AbsoluteY },
    { 0x37, "and (*)+y  ", 1, Mode::IndirectY },
    { 0x38, "and *,#*   ", 1, Mode::DirectImmediate },
    { 0x39, "and (x),(y)", 1, Mode::Implied },
    { 0x3a, "inw *      ", 1, Mode::Direct },
    { 0x3b, "rol *+x    ", 1, Mode::DirectX },
    { 0x3c, "rol        ", 1, Mode::Implied },
    { 0x3d, "inx        ", 1, Mode::Implied },
    { 0x3e, "cpx *      ", 0, Mode::Direct },
    { 0x3f, "jsr *      ", 1, Mode::Absolute },

    { 0x40, "sep        ", 1, Mode::Implied },
    { 0x44, "eor *      ", 0, Mode::Direct },
    { 0x45, "eor *      ", 1, Mode::Absolute },
    { 0x46, "eor (x)    ", 1, Mode::Implied },
    { 0x47, "eor (*+x)  ", 1, Mode::IndirectX },
    { 0x48, "eor #*     ", 1, Mode::Immediate },
    { 0x49, "eor *,*    ", 1, Mode::DirectDirect },
    { 0x4a, "and *.?    ", 1, Mode::AbsoluteBit },
    { 0x4b, "lsr *      ", 0, Mode::Direct },
    { 0x4c, "lsr *      ", 1, Mode::Absolute },
    { 0x4d, "phx        ", 1, Mode::Implied },
    { 0x4e, "trb *      ", 1, Mode::Absolute },
    { 0x4f, "jsp *      ", 1, Mode::Direct },

    { 0x50, "bvc *      ", 1, Mode::Relative },
    { 0x54, "eor *+x    ", 0, Mode::DirectX },
    { 0x55, "eor *+x    ", 1, Mode::AbsoluteX },
    { 0x56, "eor *+y    ", 1, Mode::AbsoluteY },
    { 0x57, "eor (*)+y  ", 1, Mode::IndirectY },
    { 0x58, "eor *,#*   ", 1, Mode::DirectImmediate },
    { 0x59, "eor (x),(y)", 1, Mode::Implied },
    { 0x5a, "cpw *      ", 1, Mode::Direct },
    { 0x5b, "lsr *+x    ", 1, Mode::DirectX },
    { 0x5c, "lsr        ", 1, Mode::Implied },
    { 0x5d, "tax        ", 1, Mode::Implied },
    { 0x5e, "cpy *      ", 1, Mode::Absolute },
    { 0x5f, "jmp *      ", 1, Mode::Absolute },

    { 0x60, "clc        ", 1, Mode::Implied },
    { 0x64, "cmp *      ", 0, Mode::Direct },
    { 0x65, "cmp *      ", 1, Mode::Absolute },
    { 0x66, "cmp (x)    ", 1, Mode::Implied },
    { 0x67, "cmp (*+x)  ", 1, Mode::IndirectX },
    { 0x68, "cmp #*     ", 1, Mode::Immediate },
    { 0x69, "cmp *,*    ", 1, Mode::DirectDirect },
    { 0x6a, "and !*.?   ", 1, Mode::AbsoluteBitNot },
    { 0x6b, "ror *      ", 0, Mode::Direct },
    { 0x6c, "ror *      ", 1, Mode::Absolute },
    { 0x6d, "phy        ", 1, Mode::Implied },
    { 0x6e, "dbn *,*    ", 1, Mode::DirectRelative },
    { 0x6f, "rts        ", 1, Mode::Implied },

    { 0x70, "bvs *      ", 1, Mode::Relative },
    { 0x74, "cmp *+x    ", 0, Mode::DirectX },
    { 0x75, "cmp *+x    ", 1, Mode::AbsoluteX },
    { 0x76, "cmp *+y    ", 1, Mode::AbsoluteY },
    { 0x77, "cmp (*)+y  ", 1, Mode::IndirectY },
    { 0x78, "cmp *,#*   ", 1, Mode::DirectImmediate },
    { 0x79, "cmp (x),(y)", 1, Mode::Implied },
    { 0x7a, "adw *      ", 1, Mode::Direct },
    { 0x7b, "ror *+x    ", 1, Mode::DirectX },
    { 0x7c, "ror        ", 1, Mode::Implied },
    { 0x7d, "txa        ", 1, Mode::Implied },
    { 0x7e, "cpy *      ", 1, Mode::Direct },
    { 0x7f, "rti        ", 1, Mode::Implied },

    { 0x80, "sec        ", 1, Mode::Implied },
    { 0x84, "adc *      ", 0, Mode::Direct },
    { 0x85, "adc *      ", 1, Mode::Absolute },
    { 0x86, "adc (x)    ", 1, Mode::Implied },
    { 0x87, "adc (*+x)  ", 1, Mode::IndirectX },
    { 0x88, "adc #*     ", 1, Mode::Immediate },
    { 0x89, "adc *,*    ", 1, Mode::DirectDirect },
    { 0x8a, "eor *.?    ", 1, Mode::AbsoluteBit },
    { 0x8b, "dec *      ", 0, Mode::Direct },
    { 0x8c, "dec *      ", 1, Mode::Absolute },
    { 0x8d, "ldy #*     ", 1, Mode::Immediate },
    { 0x8e, "plp        ", 1, Mode::Implied },
    { 0x8f, "sti *,#*   ", 1, Mode::DirectImmediate },

    { 0x90, "bcc *      ", 1, Mode::Relative },
    { 0x94, "adc *+x    ", 0, Mode::DirectX },
    { 0x95, "adc *+x    ", 1, Mode::AbsoluteX },
    { 0x96, "adc *+y    ", 1, Mode::AbsoluteY },
    { 0x97, "adc (*)+y  ", 1, Mode::IndirectY },
    { 0x98, "adc *,#*   ", 1, Mode::DirectImmediate },
    { 0x99, "adc (x),(y)", 1, Mode::Implied },
    { 0x9a, "sbw *      ", 1, Mode::Direct },
    { 0x9b, "dec *+x    ", 1, Mode::DirectX },
    { 0x9c, "dec        ", 1, Mode::Implied },
    { 0x9d, "tsx        ", 1, Mode::Implied },
    { 0x9e, "div        ", 1, Mode::Implied },
    { 0x9f, "xcn        ", 1, Mode::Implied },

    { 0xa0, "sei        ", 1, Mode::Implied },
    { 0xa4, "sbc *      ", 0, Mode::Direct },
    { 0xa5, "sbc *      ", 1, Mode::Absolute },
    { 0xa6, "sbc (x)    ", 1, Mode::Implied },
    { 0xa7, "sbc (*+x)  ", 1, Mode::IndirectX },
    { 0xa8, "sbc #*     ", 1, Mode::Immediate },
    { 0xa9, "sbc *,*    ", 1, Mode::DirectDirect },
    { 0xaa, "ldi *.?    ", 1, Mode::AbsoluteBit },
    { 0xab, "inc *      ", 0, Mode::Direct },
    { 0xac, "inc *      ", 1, Mode::Absolute },
    { 0xad, "cpy #*     ", 1, Mode::Immediate },
    { 0xae, "pla        ", 1, Mode::Implied },
    { 0xaf, "sta (x)+   ", 1, Mode::Implied },

    { 0xb0, "bcs *      ", 1, Mode::Relative },
    { 0xb4, "sbc *+x    ", 0, Mode::DirectX },
    { 0xb5, "sbc *+x    ", 1, Mode::AbsoluteX },
    { 0xb6, "sbc *+y    ", 1, Mode::AbsoluteY },
    { 0xb7, "sbc (*)+y  ", 1, Mode::IndirectY },
    { 0xb8, "sbc *,#*   ", 1, Mode::DirectImmediate },
    { 0xb9, "sbc (x),(y)", 1, Mode::Implied },
    { 0xba, "ldw *      ", 1, Mode::Direct },
    { 0xbb, "inc *+x    ", 1, Mode::DirectX },
    { 0xbc, "inc        ", 1, Mode::Implied },
    { 0xbd, "txs        ", 1, Mode::Implied },
    { 0xbe, "das        ", 1, Mode::Implied },
    { 0xbf, "lda (x)+   ", 1, Mode::Implied },

    { 0xc0, "cli        ", 1, Mode::Implied },
    { 0xc4, "sta *      ", 0, Mode::Direct },
    { 0xc5, "sta *      ", 1, Mode::Absolute },
    { 0xc6, "sta (x)    ", 1, Mode::Implied },
    { 0xc7, "sta (*+x)  ", 1, Mode::IndirectX },
    { 0xc8, "cpx #*     ", 1, Mode::Immediate },
    { 0xc9, "stx *      ", 1, Mode::Absolute },
    { 0xca, "sti *.?    ", 1, Mode::AbsoluteBit },
    { 0xcb, "sty *      ", 0, Mode::Direct },
    { 0xcc, "sty *      ", 1, Mode::Absolute },
    { 0xcd, "ldx #*     ", 1, Mode::Immediate },
    { 0xce, "plx        ", 1, Mode::Implied },
    { 0xcf, "mul        ", 1, Mode::Implied },

    { 0xd0, "bne *      ", 1, Mode::Relative },
    { 0xd4, "sta *+x    ", 0, Mode::DirectX },
    { 0xd5, "sta *+x    ", 1, Mode::AbsoluteX },
    { 0xd6, "sta *+y    ", 1, Mode::AbsoluteY },
    { 0xd7, "sta (*)+y  ", 1, Mode::IndirectY },
    { 0xd8, "stx *      ", 1, Mode::Direct },
    { 0xd9, "stx *+y    ", 1, Mode::DirectY },
    { 0xda, "stw *      ", 1, Mode::Direct },
    { 0xdb, "sty *+x    ", 1, Mode::DirectX },
    { 0xdc, "dey        ", 1, Mode::Implied },
    { 0xdd, "tya        ", 1, Mode::Implied },
    { 0xde, "cbne *+x,* ", 1, Mode::DirectXRelative },
    { 0xdf, "daa        ", 1, Mode::Implied },

    { 0xe0, "clv        ", 1, Mode::Implied },
    { 0xe4, "lda *      ", 0, Mode::Direct },
    { 0xe5, "lda *      ", 1, Mode::Absolute },
    { 0xe6, "lda (x)    ", 1, Mode::Implied },
    { 0xe7, "lda (*+x)  ", 1, Mode::IndirectX },
    { 0xe8, "lda #*     ", 1, Mode::Immediate },
    { 0xe9, "ldx *      ", 1, Mode::Absolute },
    { 0xea, "not *.?    ", 1, Mode::AbsoluteBit },
    { 0xeb, "ldy *      ", 0, Mode::Direct },
    { 0xec, "ldy *      ", 1, Mode::Absolute },
    { 0xed, "not        ", 1, Mode::Implied },
    { 0xee, "ply        ", 1, Mode::Implied },
    { 0xef, "wai        ", 1, Mode::Implied },

    { 0xf0, "beq *      ", 1, Mode::Relative },
    { 0xf4, "lda *+x    ", 0, Mode::DirectX },
    { 0xf5, "lda *+x    ", 1, Mode::AbsoluteX },
    { 0xf6, "lda *+y    ", 1, Mode::AbsoluteY },
    { 0xf7, "lda (*)+y  ", 1, Mode::IndirectY },
    { 0xf8, "ldx *      ", 0, Mode::Direct },
    { 0xf9, "ldx *+y    ", 1, Mode::DirectY },
    { 0xfa, "sti *,*    ", 1, Mode::DirectDirect },
    { 0xfb, "ldy *+x    ", 1, Mode::DirectX },
    { 0xfc, "iny        ", 1, Mode::Implied },
    { 0xfd, "tay        ", 1, Mode::Implied },
    { 0xfe, "dbn y,*    ", 1, Mode::YRelative },
    { 0xff, "stp        ", 1, Mode::Implied },
  };

  lstring patterns;
  foreach(opcode, table) {
    while(opcode.mnemonic.position("  ")) opcode.mnemonic.replace("  ", " ");
    opcode.mnemonic.replace("*", "?*");
    opcode.mnemonic.rtrim(" ");
    lstring part;
    part.split<1>(" ", opcode.mnemonic);
    opcode.name = part[0];
    opcode.pattern = part[1];
    if(!patterns.find(opcode.pattern)) patterns.append(opcode.pattern);
  }

  foreach(x, patterns) {
    foreach(y, patterns) {
      if(&x != &y && x.wildcard(y)) swap(x, y);
    }
  }

  foreach(opcode, table) {
    if(auto position = patterns.find(opcode.pattern)) {
      family[position()].pattern = opcode.pattern;
      family[position()].opcode.append(opcode);
    }
  }
}
