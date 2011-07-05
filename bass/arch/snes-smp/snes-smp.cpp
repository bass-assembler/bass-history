bool BassSnesSmp::assembleBlock(const string &block) {
  if(Bass::assembleBlock(block) == true) return true;

  lstring part, list;
  part.split<1>(" ", block);
  string name = part[0];
  string args = part[1];

  unsigned size = 0;

  function<void ()> detectSize = [&size, &name, &args]() {
    if(size != 0) return;
    if(name.endswith(".b")) { size = 1; name.rtrim<1>(".b"); return; }
    if(name.endswith(".w")) { size = 2; name.rtrim<1>(".w"); return; }
    if(args.wildcard("$??"  )) { size = 1; return; }
    if(args.wildcard("$????")) { size = 2; return; }
  };

  //two-argument

  if(args == "a,(x)") {
    if(name == "or" ) { write(0x06); return true; }
    if(name == "and") { write(0x26); return true; }
    if(name == "eor") { write(0x46); return true; }
    if(name == "cmp") { write(0x66); return true; }
    if(name == "adc") { write(0x86); return true; }
    if(name == "sbc") { write(0xa6); return true; }
    return false;
  }

  if(args == "x,a") {
    if(name == "mov") { write(0x5d); return true; }
  }

  if(args == "a,x") {
    if(name == "mov") { write(0x7d); return true; }
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

  if(args.wildcard("(?*+x)")) {
    args.ltrim<1>("(");
    args.rtrim<1>("+x)");
    if(name == "jmp") { write(0x1f); write(eval(args), 2); return true; }
    return false;
  }

  if(args.wildcard("ya,?*")) {
    args.ltrim<1>("ya,");
    if(name == "cmpw") { write(0x5a); write(eval(args)); return true; }
    if(name == "addw") { write(0x7a); write(eval(args)); return true; }
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
    }
    if(size != 1) {
      if(name == "or" ) { write(0x15); write(eval(args), 2); return true; }
      if(name == "and") { write(0x35); write(eval(args), 2); return true; }
      if(name == "eor") { write(0x55); write(eval(args), 2); return true; }
      if(name == "cmp") { write(0x75); write(eval(args), 2); return true; }
      if(name == "adc") { write(0x95); write(eval(args), 2); return true; }
      if(name == "sbc") { write(0xb5); write(eval(args), 2); return true; }
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
    }
    if(size != 1) {
      if(name == "or" ) { write(0x05); write(eval(args), 2); return true; }
      if(name == "and") { write(0x25); write(eval(args), 2); return true; }
      if(name == "eor") { write(0x45); write(eval(args), 2); return true; }
      if(name == "cmp") { write(0x65); write(eval(args), 2); return true; }
      if(name == "adc") { write(0x85); write(eval(args), 2); return true; }
      if(name == "sbc") { write(0xa5); write(eval(args), 2); return true; }
    }
    return false;
  }

  if(args.wildcard("x,?*")) {
    args.ltrim<1>("x,");
    detectSize();
    if(size == 1) {
      if(name == "cmp") { write(0x3e); write(eval(args)); return true; }
    }
    if(size != 1) {
      if(name == "cmp") { write(0x1e); write(eval(args), 2); return true; }
    }
    return false;
  }

  if(args.wildcard("y,?*")) {
    args.ltrim<1>("y,");
    detectSize();
    if(size == 1) {
      if(name == "cmp") { write(0x7e); write(eval(args)); return true; }
    }
    if(size != 1) {
      if(name == "cmp") { write(0x5e); write(eval(args), 2); return true; }
    }
    return false;
  }

  if(args.wildcard("c,!?*.?")) {
    args.ltrim<1>("c,!");
    part.split<1>(".", args);
    unsigned data = (eval(part[1]) << 13) | (eval(part[0]) & 0x1fff);
    if(name == "or1" ) { write(0x2a); write(data, 2); return true; }
    if(name == "and1") { write(0x6a); write(data, 2); return true; }
    return false;
  }

  if(args.wildcard("c,?*.?")) {
    args.ltrim<1>("c,");
    part.split<1>(".", args);
    unsigned data = (eval(part[1]) << 13) | (eval(part[0]) & 0x1fff);
    if(name == "or1" ) { write(0x0a); write(data, 2); return true; }
    if(name == "and1") { write(0x4a); write(data, 2); return true; }
    return false;
  }

  if(args.wildcard("?*,a")) {
    args.rtrim<1>(",a");
    if(name == "tset") { write(0x0e); write(eval(args), 2); return true; }
    if(name == "tclr") { write(0x4e); write(eval(args), 2); return true; }
    return false;
  }

  if(args.wildcard("?*.?,?*")) {
    part.split<1>(",", args);
    list.split<1>(".", part[0]);
    signed relative = eval(part[1]) - (base + 3);
    if(relative < -128 || relative > 127) error("branch out of bounds");
    if(name == "bbs") { write(0x03 | (eval(list[1]) << 5)); write(eval(list[0])); write(relative); return true; }
    if(name == "bbc") { write(0x13 | (eval(list[1]) << 5)); write(eval(list[0])); write(relative); return true; }
    return false;
  }

  if(args.wildcard("?*,#?*")) {
    part.split<1>(",", args);
    part[1].ltrim<1>("#");
    if(name == "or" ) { write(0x18); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "and") { write(0x38); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "eor") { write(0x58); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "cmp") { write(0x78); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "adc") { write(0x98); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "sbc") { write(0xb8); write(eval(part[1])); write(eval(part[0])); return true; }
    return false;
  }

  if(args.wildcard("?*,?*")) {
    part.split<1>(",", args);
    if(name == "or" ) { write(0x09); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "and") { write(0x29); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "eor") { write(0x49); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "cmp") { write(0x69); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "adc") { write(0x89); write(eval(part[1])); write(eval(part[0])); return true; }
    if(name == "sbc") { write(0xa9); write(eval(part[1])); write(eval(part[0])); return true; }
    //
    signed relative = eval(part[1]) - (base + 2);
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
    return false;
  }

  if(args == "x") {
    if(name == "dec" ) { write(0x1d); return true; }
    if(name == "inc" ) { write(0x3d); return true; }
    if(name == "push") { write(0x4d); return true; }
    return false;
  }

  if(args == "y") {
    if(name == "push") { write(0x6d); return true; }
    return false;
  }

  if(args == "p") {
    if(name == "push") { write(0x0d); return true; }
    return false;
  }

  if(args.wildcard("?*.?")) {
    part.split<1>(".", args);
    if(name == "set1") { write(0x02 | (eval(part[1]) << 5)); write(eval(part[0])); return true; }
    if(name == "clr1") { write(0x12 | (eval(part[1]) << 5)); write(eval(part[0])); return true; }
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
    }
    if(size != 1) {
      if(name == "asl" ) { write(0x0c); write(eval(args), 2); return true; }
      if(name == "rol" ) { write(0x2c); write(eval(args), 2); return true; }
      if(name == "lsr" ) { write(0x4c); write(eval(args), 2); return true; }
      if(name == "ror" ) { write(0x6c); write(eval(args), 2); return true; }
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
      signed relative = eval(args) - (base + 2);
      if(relative < -128 || relative > 127) error("branch out of bounds");
      if(name == "bpl") { write(0x10); write(relative); return true; }
      if(name == "bra") { write(0x2f); write(relative); return true; }
      if(name == "bmi") { write(0x30); write(relative); return true; }
      if(name == "bvc") { write(0x50); write(relative); return true; }
      if(name == "bvs") { write(0x70); write(relative); return true; }
      if(name == "bcc") { write(0x90); write(relative); return true; }
    }
    return false;
  }

  //zero-argument

  if(args == "") {
    if(name == "nop" ) { write(0x00); return true; }
    if(name == "brk" ) { write(0x0f); return true; }
    if(name == "clrp") { write(0x20); return true; }
    if(name == "setp") { write(0x40); return true; }
    if(name == "clrc") { write(0x60); return true; }
    if(name == "ret" ) { write(0x6f); return true; }
    if(name == "reti") { write(0x7f); return true; }
    if(name == "setc") { write(0x80); return true; }
    return false;
  }

  return false;
}
