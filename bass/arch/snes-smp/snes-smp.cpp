bool BassSnesSmp::assembleBlock(const string &block) {
  if(Bass::assembleBlock(block) == true) return true;

  lstring part, byte;
  part.split(" ", block);
  string name = part[0];
  string args = part[1];
  signed relative;

  unsigned size = 0;
       if(name.endswith(".b")) { name.rtrim<1>(".b"); size = 1; }
  else if(name.endswith(".w")) { name.rtrim<1>(".w"); size = 2; }

  foreach(f, family) if(args.wildcard(f.pattern)) {
    foreach(o, f.opcode) if(name == o.name) {
      switch(o.mode) {
      case Mode::Implied:
        write(o.byte);
        return true;
      case Mode::Vector:
        write(o.byte | (eval(args) << 4));
        return true;
      case Mode::IndirectX:
        args.ltrim<1>("(");
        args.rtrim<1>("+x)");
      case Mode::Direct:
        if(!o.priority && size != 1 && !args.wildcard("$??")) break;
        write(o.byte);
        write(eval(args));
        return true;
      case Mode::DirectBit:
        byte.split(".", args);
        write(o.byte | (eval(byte[1]) << 5));
        write(eval(byte[0]));
        return true;
      case Mode::DirectBitRelative:
        part.split(",", args);
        byte.split(".", part[0]);
        relative = eval(part[1]) - (pc() + 3);
        if(relative < -128 || relative > +127) error("branch out of bounds");
        write(o.byte | (eval(byte[1]) << 5));
        write(eval(byte[0]));
        write(relative);
        return true;
      case Mode::Address:
        if(!o.priority && size != 2 && !args.wildcard("$????")) break;
        write(o.byte);
        write(eval(args), 2);
        return true;
      case Mode::DirectXRelative:
        part.split(",", args);
        part[0].rtrim<1>("+x");
        relative = eval(part[1]) - (pc() + 3);
        if(relative < -128 || relative > +127) error("branch out of bounds");
        write(o.byte);
        write(eval(part[0]));
        write(relative);
        return true;
      }
    }
  }

  return false;
}

BassSnesSmp::BassSnesSmp() {
  Opcode table[] = {
    { 0x00, "nop",  "",      1, Mode::Implied           },
    { 0x01, "jsrt", "*",     1, Mode::Vector            },
    { 0x02, "set1", "*.*",   1, Mode::DirectBit         },
    { 0x03, "bbs1", "*.?,*", 1, Mode::DirectBitRelative },
    { 0x04, "or",   "*",     0, Mode::Direct            },
    { 0x05, "or",   "*",     1, Mode::Address           },
    { 0x06, "or",   "(x)",   1, Mode::Implied           },
    { 0x07, "or",   "(*+x)", 1, Mode::IndirectX         },
    { 0xde, "cbne", "*+x,*", 1, Mode::DirectXRelative   },
  };

  lstring patterns;
  foreach(opcode, table) {
    opcode.pattern.replace("*", "?*");
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
