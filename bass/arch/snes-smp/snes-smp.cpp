bool BassSnesSmp::assembleBlock(const string &block) {
  if(Bass::assembleBlock(block) == true) return true;

  lstring part, byte;
  part.split(" ", block);
  string name = part[0], args = part[1];
  signed relative, address, size = 0;
       if(name.endswith(".b")) { name.rtrim<1>(".b"); size = 1; }
  else if(name.endswith(".w")) { name.rtrim<1>(".w"); size = 2; }

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
        if(size == 2 || args.wildcard("$????")) break;
        if(!o.priority && size != 1 && !args.wildcard("$??")) break;
        write(o.prefix);
        write(eval(args));
        return true;
      case Mode::DirectX:
        if(size == 2 || args.wildcard("$????+x")) break;
        if(!o.priority && size != 1 && !args.wildcard("$??+x")) break;
        args.rtrim<1>("+x");
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
        if(relative < -128 || relative > +127) error("branch out of bounds");
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
        if(relative < -128 || relative > +127) error("branch out of bounds");
        write(o.prefix);
        write(eval(part[0]));
        write(relative);
        return true;
      case Mode::Absolute:
        if(size == 1 || args.wildcard("$??")) break;
        if(!o.priority && size != 2 && !args.wildcard("$????")) break;
        write(o.prefix);
        write(eval(args), 2);
        return true;
      case Mode::AbsoluteX:
        if(size == 1 || args.wildcard("$??+x")) break;
        if(!o.priority && size != 2 && !args.wildcard("$????+x")) break;
        args.rtrim<1>("+x");
        write(o.prefix);
        write(eval(args), 2);
        return true;
      case Mode::AbsoluteY:
        if(size == 1 || args.wildcard("$??+y")) break;
        if(!o.priority && size != 2 && !args.wildcard("$????+y")) break;
        args.rtrim<1>("+y");
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
        if(relative < -128 || relative > +127) error("branch out of bounds");
        write(o.prefix);
        write(relative);
        return true;
      }
    }
  }

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
    { 0x09, "ora *,*    ", 1, Mode::DirectDirect },
    { 0x0a, "ora *.?    ", 1, Mode::AbsoluteBit },
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
    { 0x18, "ora *,#*   ", 1, Mode::DirectImmediate },
    { 0x19, "ora (x),(y)", 1, Mode::Implied },
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
    { 0x2a, "ora !*.?   ", 1, Mode::AbsoluteBitNot },
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
  };

  lstring patterns;
  foreach(opcode, table) {
    while(opcode.mnemonic.position("  ")) opcode.mnemonic.replace("  ", " ");
    opcode.mnemonic.replace("*", "?*");
    opcode.mnemonic.rtrim(" ");
    lstring part;
    part.split(" ", opcode.mnemonic);
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
