bool BassSnesSmpCanonical::assembleBlock(const string &block) {
  if(Bass::assembleBlock(block) == true) return true;

  lstring part = block.split<1>(" "), list;
  string name = part[0], args = part(1, "");

  unsigned size = 0;

  function<void ()> detectSize = [&size, &name, &args]() {
    if(size != 0) return;
    if(name.endswith(".b")) { size = 1; name.rtrim<1>(".b"); return; }
    if(name.endswith(".w")) { size = 2; name.rtrim<1>(".w"); return; }
    if(args.wildcard("$??"  )) { size = 1; return; }
    if(args.wildcard("$????")) { size = 2; return; }
  };

  //two-argument

  if(block == "mov x,a"   ) { write(0x5d); return true; }
  if(block == "mov a,x"   ) { write(0x7d); return true; }
  if(block == "mov a,y"   ) { write(0xdd); return true; }
  if(block == "mov x,sp"  ) { write(0x9d); return true; }
  if(block == "mov y,a"   ) { write(0xfd); return true; }
  if(block == "mov sp,x"  ) { write(0xbd); return true; }
  if(block == "mov a,(x)+") { write(0xbf); return true; }
  if(block == "mov (x)+,a") { write(0xaf); return true; }
  if(block == "mov (x),a" ) { write(0xc6); return true; }
  if(block == "div ya,x"  ) { write(0x9e); return true; }

  if(args == "a,(x)") {
    if(name == "or" ) { write(0x06); return true; }
    if(name == "and") { write(0x26); return true; }
    if(name == "eor") { write(0x46); return true; }
    if(name == "cmp") { write(0x66); return true; }
    if(name == "adc") { write(0x86); return true; }
    if(name == "sbc") { write(0xa6); return true; }
    if(name == "mov") { write(0xe6); return true; }
    return false;
  }

  if(args == "(x),(y)") {
    if(name == "or" ) { write(0x19); return true; }
    if(name == "and") { write(0x39); return true; }
    if(name == "eor") { write(0x59); return true; }
    if(name == "cmp") { write(0x79); return true; }
    if(name == "adc") { write(0x99); return true; }
    if(name == "sbc") { write(0xb9); return true; }
    return false;
  }

  if(args.wildcard("ya,?*")) {
    args.ltrim<1>("ya,");
    if(name == "cmpw") { write(0x5a); write(eval(args)); return true; }
    if(name == "addw") { write(0x7a); write(eval(args)); return true; }
    if(name == "subw") { write(0x9a); write(eval(args)); return true; }
    if(name == "movw") { write(0xba); write(eval(args)); return true; }
    return false;
  }

  if(args.wildcard("?*,ya")) {
    args.rtrim<1>(",ya");
    if(name == "movw") { write(0xda); write(eval(args)); return true; }
    return false;
  }

  if(args.wildcard("a,#?*")) {
    args.ltrim<1>("a,#");
    if(name == "or" ) { write(0x08); write(eval(args)); return true; }
    if(name == "and") { write(0x28); write(eval(args)); return true; }
    if(name == "eor") { write(0x48); write(eval(args)); return true; }
    if(name == "cmp") { write(0x68); write(eval(args)); return true; }
    if(name == "adc") { write(0x88); write(eval(args)); return true; }
    if(name == "sbc") { write(0xa8); write(eval(args)); return true; }
    if(name == "mov") { write(0xe8); write(eval(args)); return true; }
    return false;
  }

  if(args.wildcard("x,#?*")) {
    args.ltrim<1>("x,#");
    if(name == "cmp") { write(0xc8); write(eval(args)); return true; }
    if(name == "mov") { write(0xcd); write(eval(args)); return true; }
    return false;
  }

  if(args.wildcard("a,(?*+x)")) {
    args.ltrim<1>("a,(");
    args.rtrim<1>("+x)");
    if(name == "or" ) { write(0x07); write(eval(args)); return true; }
    if(name == "and") { write(0x27); write(eval(args)); return true; }
    if(name == "eor") { write(0x47); write(eval(args)); return true; }
    if(name == "cmp") { write(0x67); write(eval(args)); return true; }
    if(name == "adc") { write(0x87); write(eval(args)); return true; }
    if(name == "sbc") { write(0xa7); write(eval(args)); return true; }
    if(name == "mov") { write(0xe7); write(eval(args)); return true; }
    return false;
  }

  if(args.wildcard("a,(?*)+y")) {
    args.ltrim<1>("a,(");
    args.rtrim<1>(")+y");
    if(name == "or" ) { write(0x17); write(eval(args)); return true; }
    if(name == "and") { write(0x37); write(eval(args)); return true; }
    if(name == "eor") { write(0x57); write(eval(args)); return true; }
    if(name == "cmp") { write(0x77); write(eval(args)); return true; }
    if(name == "adc") { write(0x97); write(eval(args)); return true; }
    if(name == "sbc") { write(0xb7); write(eval(args)); return true; }
    if(name == "mov") { write(0xf7); write(eval(args)); return true; }
    return false;
  }

  if(args.wildcard("(?*)+y,a")) {
    args.ltrim<1>("(");
    args.rtrim<1>(")+y,a");
    if(name == "mov") { write(0xd7); write(eval(args)); return true; }
    return false;
  }

  if(args.wildcard("a,?*+x")) {
    args.ltrim<1>("a,");
    args.rtrim<1>("+x");
    detectSize();
    if(size == 1) {
      if(name == "or" ) { write(0x14); write(eval(args)); return true; }
      if(name == "and") { write(0x34); write(eval(args)); return true; }
      if(name == "eor") { write(0x54); write(eval(args)); return true; }
      if(name == "cmp") { write(0x74); write(eval(args)); return true; }
      if(name == "adc") { write(0x94); write(eval(args)); return true; }
      if(name == "sbc") { write(0xb4); write(eval(args)); return true; }
      if(name == "mov") { write(0xf4); write(eval(args)); return true; }
    }
    if(size != 1) {
      if(name == "or" ) { write(0x15); write(eval(args), 2); return true; }
      if(name == "and") { write(0x35); write(eval(args), 2); return true; }
      if(name == "eor") { write(0x55); write(eval(args), 2); return true; }
      if(name == "cmp") { write(0x75); write(eval(args), 2); return true; }
      if(name == "adc") { write(0x95); write(eval(args), 2); return true; }
      if(name == "sbc") { write(0xb5); write(eval(args), 2); return true; }
      if(name == "mov") { write(0xf5); write(eval(args), 2); return true; }
    }
    return false;
  }

  if(args.wildcard("a,?*+y")) {
    args.ltrim<1>("a,");
    args.rtrim<1>("+y");
    detectSize();
    if(size != 1) {
      if(name == "or" ) { write(0x16); write(eval(args), 2); return true; }
      if(name == "and") { write(0x36); write(eval(args), 2); return true; }
      if(name == "eor") { write(0x56); write(eval(args), 2); return true; }
      if(name == "cmp") { write(0x76); write(eval(args), 2); return true; }
      if(name == "adc") { write(0x96); write(eval(args), 2); return true; }
      if(name == "sbc") { write(0xb6); write(eval(args), 2); return true; }
      if(name == "mov") { write(0xf6); write(eval(args), 2); return true; }
    }
    return false;
  }

  if(args.wildcard("a,?*")) {
    args.ltrim<1>("a,");
    detectSize();
    if(size == 1) {
      if(name == "or" ) { write(0x04); write(eval(args)); return true; }
      if(name == "and") { write(0x24); write(eval(args)); return true; }
      if(name == "eor") { write(0x44); write(eval(args)); return true; }
      if(name == "cmp") { write(0x64); write(eval(args)); return true; }
      if(name == "adc") { write(0x84); write(eval(args)); return true; }
      if(name == "sbc") { write(0xa4); write(eval(args)); return true; }
      if(name == "mov") { write(0xe4); write(eval(args)); return true; }
    }
    if(size != 1) {
      if(name == "or" ) { write(0x05); write(eval(args), 2); return true; }
      if(name == "and") { write(0x25); write(eval(args), 2); return true; }
      if(name == "eor") { write(0x45); write(eval(args), 2); return true; }
      if(name == "cmp") { write(0x65); write(eval(args), 2); return true; }
      if(name == "adc") { write(0x85); write(eval(args), 2); return true; }
      if(name == "sbc") { write(0xa5); write(eval(args), 2); return true; }
      if(name == "mov") { write(0xe5); write(eval(args), 2); return true; }
    }
    return false;
  }

  if(args.wildcard("x,?*+y")) {
    args.ltrim<1>("x,");
    args.rtrim<1>("+y");
    if(name == "mov") { write(0xf9); write(eval(args)); return true; }
    return false;
  }

  if(args.wildcard("x,?*")) {
    args.ltrim<1>("x,");
    detectSize();
    if(size == 1) {
      if(name == "cmp") { write(0x3e); write(eval(args), 1); return true; }
      if(name == "mov") { write(0xf8); write(eval(args), 1); return true; }
    }
    if(size != 1) {
      if(name == "cmp") { write(0x1e); write(eval(args), 2); return true; }
      if(name == "mov") { write(0xe9); write(eval(args), 2); return true; }
    }
    return false;
  }

  if(args.wildcard("y,#?*")) {
    args.ltrim<1>("y,#");
    if(name == "mov") { write(0x8d); write(eval(args)); return true; }
    if(name == "cmp") { write(0xad); write(eval(args)); return true; }
    return false;
  }

  if(args.wildcard("y,?*+x")) {
    args.ltrim<1>("y,");
    args.rtrim<1>("+x");
    if(name == "mov") { write(0xfb); write(eval(args)); return true; }
    return false;
  }

  if(args.wildcard("y,?*")) {
    args.ltrim<1>("y,");
    detectSize();
    if(size == 1) {
      if(name == "cmp") { write(0x7e); write(eval(args), 1); return true; }
      if(name == "mov") { write(0xeb); write(eval(args), 1); return true; }
    }
    if(size != 1) {
      if(name == "cmp") { write(0x5e); write(eval(args), 2); return true; }
      if(name == "mov") { write(0xec); write(eval(args), 2); return true; }
    }
    //
    signed relative = eval(args) - (pc() + 2);
    if(relative < -128 || relative > 127) error("branch out of bounds");
    if(name == "dbnz") { write(0xfe); write(relative); return true; }
    return false;
  }

  if(args.wildcard("c,!?*.?")) {
    args.ltrim<1>("c,!");
    part = args.split<1>(".");
    unsigned data = (eval(part[1]) << 13) | (eval(part[0]) & 0x1fff);
    if(name == "or1" ) { write(0x2a); write(data, 2); return true; }
    if(name == "and1") { write(0x6a); write(data, 2); return true; }
    return false;
  }

  if(args.wildcard("c,?*.?")) {
    args.ltrim<1>("c,");
    part = args.split<1>(".");
    unsigned data = (eval(part[1]) << 13) | (eval(part[0]) & 0x1fff);
    if(name == "or1" ) { write(0x0a); write(data, 2); return true; }
    if(name == "and1") { write(0x4a); write(data, 2); return true; }
    if(name == "eor1") { write(0x8a); write(data, 2); return true; }
    if(name == "mov1") { write(0xaa); write(data, 2); return true; }
    if(name == "not1") { write(0xea); write(data, 2); return true; }
    return false;
  }

  if(args.wildcard("?*.?,c")) {
    args.rtrim<1>(",c");
    part = args.split<1>(".");
    unsigned data = (eval(part[1]) << 13) | (eval(part[0]) & 0x1fff);
    if(name == "mov1") { write(0xca); write(data, 2); return true; }
    return false;
  }

  if(args.wildcard("(?*+x),a")) {
    args.ltrim<1>("(");
    args.rtrim<1>("+x),a");
    if(name == "mov") { write(0xc7); write(eval(args)); return true; }
    return false;
  }

  if(args.wildcard("?*+x,a")) {
    args.rtrim<1>("+x,a");
    detectSize();
    if(size == 1) {
      if(name == "mov") { write(0xd4); write(eval(args), 1); return true; }
    }
    if(size != 1) {
      if(name == "mov") { write(0xd5); write(eval(args), 2); return true; }
    }
    return false;
  }

  if(args.wildcard("?*+y,a")) {
    args.rtrim<1>("+y,a");
    if(name == "mov") { write(0xd6); write(eval(args), 2); return true; }
    return false;
  }

  if(args.wildcard("?*,a")) {
    args.rtrim<1>(",a");
    if(name == "tset") { write(0x0e); write(eval(args), 2); return true; }
    if(name == "tclr") { write(0x4e); write(eval(args), 2); return true; }
    //
    detectSize();
    if(size == 1) {
      if(name == "mov") { write(0xc4); write(eval(args), 1); return true; }
    }
    if(size != 1) {
      if(name == "mov") { write(0xc5); write(eval(args), 2); return true; }
    }
    return false;
  }

  if(args.wildcard("?*+x,y")) {
    args.rtrim<1>("+x,y");
    if(name == "mov") { write(0xdb); write(eval(args)); return true; }
    return false;
  }

  if(args.wildcard("?*+y,x")) {
    args.rtrim<1>("+y,x");
    if(name == "mov") { write(0xd9); write(eval(args)); return true; }
    return false;
  }

  if(args.wildcard("?*,x")) {
    args.rtrim<1>(",x");
    detectSize();
    if(size == 1) {
      if(name == "mov") { write(0xd8); write(eval(args), 1); return true; }
    }
    if(size != 1) {
      if(name == "mov") { write(0xc9); write(eval(args), 2); return true; }
    }
    return false;
  }

  if(args.wildcard("?*,y")) {
    args.rtrim<1>(",y");
    detectSize();
    if(size == 1) {
      if(name == "mov") { write(0xcb); write(eval(args), 1); return true; }
    }
    if(size != 1) {
      if(name == "mov") { write(0xcc); write(eval(args), 2); return true; }
    }
    return false;
  }

  if(args.wildcard("?*.?,?*")) {
    part = args.split<1>(",");
    list = part[0].split<1>(".");
    signed relative = eval(part[1]) - (pc() + 3);
    if(relative < -128 || relative > 127) error("branch out of bounds");
    if(name == "bbs1") { write(0x03 | (eval(list[1]) << 5)); write(eval(list[0])); write(relative); return true; }
    if(name == "bbc1") { write(0x13 | (eval(list[1]) << 5)); write(eval(list[0])); write(relative); return true; }
    return false;
  }

  if(args.wildcard("?*,#?*")) {
    part = args.split<1>(",");
    part[1].ltrim<1>("#");
    if(name == "or" ) { write(0x18); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "and") { write(0x38); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "eor") { write(0x58); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "cmp") { write(0x78); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "adc") { write(0x98); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "sbc") { write(0xb8); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "mov") { write(0x8f); write(eval(part[1])); write(eval(part[0])); return true; }
    return false;
  }

  if(args.wildcard("?*+x,?*")) {
    part = args.split<1>(",");
    part[0].rtrim<1>("+x");
    signed relative = eval(part[1]) - (pc() + 3);
    if(relative < -128 || relative > 127) error("branch out of bounds");
    if(name == "cbne") { write(0xde); write(eval(part[0])); write(relative); return true; }
    return false;
  }

  if(args.wildcard("?*,?*")) {
    part = args.split<1>(",");
    if(name == "or" ) { write(0x09); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "and") { write(0x29); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "eor") { write(0x49); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "cmp") { write(0x69); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "adc") { write(0x89); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "sbc") { write(0xa9); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "mov") { write(0xfa); write(eval(part[1])); write(eval(part[0])); return true; }
    //
    signed relative = eval(part[1]) - (pc() + 3);
    if(relative < -128 || relative > 127) error("branch out of bounds");
    if(name == "cbne") { write(0x2e); write(eval(part[0])); write(relative); return true; }
    if(name == "dbnz") { write(0x6e); write(eval(part[0])); write(relative); return true; }
    return false;
  }

  //one-argument

  if(args == "a") {
    if(name == "asl" ) { write(0x1c); return true; }
    if(name == "push") { write(0x2d); return true; }
    if(name == "rol" ) { write(0x3c); return true; }
    if(name == "lsr" ) { write(0x5c); return true; }
    if(name == "ror" ) { write(0x7c); return true; }
    if(name == "dec" ) { write(0x9c); return true; }
    if(name == "xcn" ) { write(0x9f); return true; }
    if(name == "pop" ) { write(0xae); return true; }
    if(name == "inc" ) { write(0xbc); return true; }
    if(name == "das" ) { write(0xbe); return true; }
    if(name == "daa" ) { write(0xdf); return true; }
    return false;
  }

  if(args == "x") {
    if(name == "dec" ) { write(0x1d); return true; }
    if(name == "inc" ) { write(0x3d); return true; }
    if(name == "push") { write(0x4d); return true; }
    if(name == "pop" ) { write(0xce); return true; }
    return false;
  }

  if(args == "y") {
    if(name == "push") { write(0x6d); return true; }
    if(name == "dec" ) { write(0xdc); return true; }
    if(name == "pop" ) { write(0xee); return true; }
    if(name == "inc" ) { write(0xfc); return true; }
    return false;
  }

  if(args == "ya") {
    if(name == "mul") { write(0xcf); return true; }
    return false;
  }

  if(args == "p") {
    if(name == "push") { write(0x0d); return true; }
    if(name == "pop" ) { write(0x8e); return true; }
    return false;
  }

  if(args.wildcard("?*.?")) {
    part = args.split<1>(".");
    if(name == "set1") { write(0x02 | (eval(part[1]) << 5)); write(eval(part[0])); return true; }
    if(name == "clr1") { write(0x12 | (eval(part[1]) << 5)); write(eval(part[0])); return true; }
    return false;
  }

  if(args.wildcard("(?*+x)")) {
    args.ltrim<1>("(");
    args.rtrim<1>("+x)");
    if(name == "jmp") { write(0x1f); write(eval(args), 2); return true; }
    return false;
  }

  if(args.wildcard("?*+x")) {
    args.rtrim<1>("+x");
    detectSize();
    if(size != 2) {
      if(name == "asl") { write(0x1b); write(eval(args)); return true; }
      if(name == "rol") { write(0x3b); write(eval(args)); return true; }
      if(name == "lsr") { write(0x5b); write(eval(args)); return true; }
      if(name == "ror") { write(0x7b); write(eval(args)); return true; }
      if(name == "dec") { write(0x9b); write(eval(args)); return true; }
      if(name == "inc") { write(0xbb); write(eval(args)); return true; }
    }
    return false;
  }

  if(args.wildcard("?*")) {
    detectSize();
    if(size == 1) {
      if(name == "asl") { write(0x0b); write(eval(args)); return true; }
      if(name == "rol") { write(0x2b); write(eval(args)); return true; }
      if(name == "lsr") { write(0x4b); write(eval(args)); return true; }
      if(name == "ror") { write(0x6b); write(eval(args)); return true; }
      if(name == "dec") { write(0x8b); write(eval(args)); return true; }
      if(name == "inc") { write(0xab); write(eval(args)); return true; }
    }
    if(size != 1) {
      if(name == "asl" ) { write(0x0c); write(eval(args), 2); return true; }
      if(name == "rol" ) { write(0x2c); write(eval(args), 2); return true; }
      if(name == "lsr" ) { write(0x4c); write(eval(args), 2); return true; }
      if(name == "ror" ) { write(0x6c); write(eval(args), 2); return true; }
      if(name == "dec" ) { write(0x8c); write(eval(args), 2); return true; }
      if(name == "inc" ) { write(0xac); write(eval(args), 2); return true; }
      if(name == "call") { write(0x3f); write(eval(args), 2); return true; }
      if(name == "jmp" ) { write(0x5f); write(eval(args), 2); return true; }
      //
      if(name == "tcall") { write(0x01 | (eval(args) << 4)); return true; }
    }
    if(size != 2) {
      if(name == "decw" ) { write(0x1a); write(eval(args)); return true; }
      if(name == "incw" ) { write(0x3a); write(eval(args)); return true; }
      if(name == "pcall") { write(0x4f); write(eval(args)); return true; }
      //
      signed relative = eval(args) - (pc() + 2);
      if(relative < -128 || relative > 127) error("branch out of bounds");
      if(name == "bpl") { write(0x10); write(relative); return true; }
      if(name == "bra") { write(0x2f); write(relative); return true; }
      if(name == "bmi") { write(0x30); write(relative); return true; }
      if(name == "bvc") { write(0x50); write(relative); return true; }
      if(name == "bvs") { write(0x70); write(relative); return true; }
      if(name == "bcc") { write(0x90); write(relative); return true; }
      if(name == "bcs") { write(0xb0); write(relative); return true; }
      if(name == "bne") { write(0xd0); write(relative); return true; }
      if(name == "beq") { write(0xf0); write(relative); return true; }
    }
    return false;
  }

  //zero-argument

  if(args == "") {
    if(name == "nop"  ) { write(0x00); return true; }
    if(name == "brk"  ) { write(0x0f); return true; }
    if(name == "clrp" ) { write(0x20); return true; }
    if(name == "setp" ) { write(0x40); return true; }
    if(name == "clrc" ) { write(0x60); return true; }
    if(name == "ret"  ) { write(0x6f); return true; }
    if(name == "reti" ) { write(0x7f); return true; }
    if(name == "setc" ) { write(0x80); return true; }
    if(name == "ei"   ) { write(0xa0); return true; }
    if(name == "di"   ) { write(0xc0); return true; }
    if(name == "clrv" ) { write(0xe0); return true; }
    if(name == "notc" ) { write(0xed); return true; }
    if(name == "sleep") { write(0xef); return true; }
    if(name == "stop" ) { write(0xff); return true; }
    return false;
  }

  return false;
}
