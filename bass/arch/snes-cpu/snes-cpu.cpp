void BassSnesCpu::seek(unsigned offset) {
  switch(mapper) {
  case Mapper::LoROM: offset = ((offset & 0x7f0000) >> 1) | (offset & 0x7fff); break;
  case Mapper::HiROM: offset = ((offset & 0x3fffff)); break;
  }
  Bass::seek(offset);
}

bool BassSnesCpu::assembleBlock(const string &block) {
  if(Bass::assembleBlock(block) == true) return true;

  lstring part;
  part.split<1>(" ", block);
  string name   = part[0];
  string arg    = part[1];
  unsigned size = 0;

  if(name == "mapper") {
    if(arg == "none" ) { mapper = Mapper::None;  return true; }
    if(arg == "lorom") { mapper = Mapper::LoROM; return true; }
    if(arg == "hirom") { mapper = Mapper::HiROM; return true; }
    error("invalid mapper ID");
  }

  function<void ()> detectSize = [&name, &arg, &size]() {
         if(arg.wildcard("$??"    )) size = 1;
    else if(arg.wildcard("$????"  )) size = 2;
    else if(arg.wildcard("$??????")) size = 3;
         if(name.endswith(".b")) { size = 1; name.rtrim<1>(".b"); }
    else if(name.endswith(".w")) { size = 2; name.rtrim<1>(".w"); }
    else if(name.endswith(".l")) { size = 3; name.rtrim<1>(".l"); }
  };

  if(arg == "" || arg.wildcard("#?*")) {
    arg.ltrim<1>("#");
    unsigned n = arg == "" ? 1 : eval(arg);
    if(name == "asl") { for(unsigned r = 0; r < n; r++) write(0x0a); return true; }
    if(name == "clc") { for(unsigned r = 0; r < n; r++) write(0x18); return true; }
    if(name == "cld") { for(unsigned r = 0; r < n; r++) write(0xd8); return true; }
    if(name == "cli") { for(unsigned r = 0; r < n; r++) write(0x58); return true; }
    if(name == "clv") { for(unsigned r = 0; r < n; r++) write(0xb8); return true; }
    if(name == "dec") { for(unsigned r = 0; r < n; r++) write(0x3a); return true; }
    if(name == "dex") { for(unsigned r = 0; r < n; r++) write(0xca); return true; }
    if(name == "dey") { for(unsigned r = 0; r < n; r++) write(0x88); return true; }
    if(name == "inc") { for(unsigned r = 0; r < n; r++) write(0x1a); return true; }
    if(name == "inx") { for(unsigned r = 0; r < n; r++) write(0xe8); return true; }
    if(name == "iny") { for(unsigned r = 0; r < n; r++) write(0xc8); return true; }
    if(name == "lsr") { for(unsigned r = 0; r < n; r++) write(0x4a); return true; }
    if(name == "nop") { for(unsigned r = 0; r < n; r++) write(0xea); return true; }
    if(name == "pha") { for(unsigned r = 0; r < n; r++) write(0x48); return true; }
    if(name == "phb") { for(unsigned r = 0; r < n; r++) write(0x8b); return true; }
    if(name == "phd") { for(unsigned r = 0; r < n; r++) write(0x0b); return true; }
    if(name == "phk") { for(unsigned r = 0; r < n; r++) write(0x4b); return true; }
    if(name == "php") { for(unsigned r = 0; r < n; r++) write(0x08); return true; }
    if(name == "phx") { for(unsigned r = 0; r < n; r++) write(0xda); return true; }
    if(name == "phy") { for(unsigned r = 0; r < n; r++) write(0x5a); return true; }
    if(name == "pla") { for(unsigned r = 0; r < n; r++) write(0x68); return true; }
    if(name == "plb") { for(unsigned r = 0; r < n; r++) write(0xab); return true; }
    if(name == "pld") { for(unsigned r = 0; r < n; r++) write(0x2b); return true; }
    if(name == "plp") { for(unsigned r = 0; r < n; r++) write(0x28); return true; }
    if(name == "plx") { for(unsigned r = 0; r < n; r++) write(0xfa); return true; }
    if(name == "ply") { for(unsigned r = 0; r < n; r++) write(0x7a); return true; }
    if(name == "rol") { for(unsigned r = 0; r < n; r++) write(0x2a); return true; }
    if(name == "ror") { for(unsigned r = 0; r < n; r++) write(0x6a); return true; }
    if(name == "rti") { for(unsigned r = 0; r < n; r++) write(0x40); return true; }
    if(name == "rtl") { for(unsigned r = 0; r < n; r++) write(0x6b); return true; }
    if(name == "rts") { for(unsigned r = 0; r < n; r++) write(0x60); return true; }
    if(name == "sec") { for(unsigned r = 0; r < n; r++) write(0x38); return true; }
    if(name == "sed") { for(unsigned r = 0; r < n; r++) write(0xf8); return true; }
    if(name == "sei") { for(unsigned r = 0; r < n; r++) write(0x78); return true; }
    if(name == "stp") { for(unsigned r = 0; r < n; r++) write(0xdb); return true; }
    if(name == "tax") { for(unsigned r = 0; r < n; r++) write(0xaa); return true; }
    if(name == "tay") { for(unsigned r = 0; r < n; r++) write(0xa8); return true; }
    if(name == "tcd") { for(unsigned r = 0; r < n; r++) write(0x5b); return true; }
    if(name == "tcs") { for(unsigned r = 0; r < n; r++) write(0x1b); return true; }
    if(name == "tdc") { for(unsigned r = 0; r < n; r++) write(0x7b); return true; }
    if(name == "tsc") { for(unsigned r = 0; r < n; r++) write(0x3b); return true; }
    if(name == "tsx") { for(unsigned r = 0; r < n; r++) write(0xba); return true; }
    if(name == "txa") { for(unsigned r = 0; r < n; r++) write(0x8a); return true; }
    if(name == "txs") { for(unsigned r = 0; r < n; r++) write(0x9a); return true; }
    if(name == "txy") { for(unsigned r = 0; r < n; r++) write(0x9b); return true; }
    if(name == "tya") { for(unsigned r = 0; r < n; r++) write(0x98); return true; }
    if(name == "tyx") { for(unsigned r = 0; r < n; r++) write(0xbb); return true; }
    if(name == "wai") { for(unsigned r = 0; r < n; r++) write(0xcb); return true; }
    if(name == "xba") { for(unsigned r = 0; r < n; r++) write(0xeb); return true; }
    if(name == "xce") { for(unsigned r = 0; r < n; r++) write(0xfb); return true; }
    if(arg == "") return false;
    detectSize();
    if(size == 0) size = 1;
    if(name == "adc") { write(0x69); write(n, size); return true; }
    if(name == "and") { write(0x29); write(n, size); return true; }
    if(name == "bit") { write(0x89); write(n, size); return true; }
    if(name == "brk") { write(0x00); write(n,    1); return true; }
    if(name == "cmp") { write(0xc9); write(n, size); return true; }
    if(name == "cop") { write(0x02); write(n,    1); return true; }
    if(name == "cpx") { write(0xe0); write(n, size); return true; }
    if(name == "cpy") { write(0xc0); write(n, size); return true; }
    if(name == "eor") { write(0x49); write(n, size); return true; }
    if(name == "lda") { write(0xa9); write(n, size); return true; }
    if(name == "ldx") { write(0xa2); write(n, size); return true; }
    if(name == "ldy") { write(0xa0); write(n, size); return true; }
    if(name == "ora") { write(0x09); write(n, size); return true; }
    if(name == "rep") { write(0xc2); write(n,    1); return true; }
    if(name == "sbc") { write(0xe9); write(n, size); return true; }
    if(name == "sep") { write(0xe2); write(n,    1); return true; }
    if(name == "wdm") { write(0x42); write(n,    1); return true; }
    return false;
  }

  if(arg.wildcard("?*,s")) {
    arg.rtrim<1>(",s");
    unsigned n = eval(arg);
    if(name == "adc") { write(0x63); write(n); return true; }
    if(name == "and") { write(0x23); write(n); return true; }
    if(name == "cmp") { write(0xc3); write(n); return true; }
    if(name == "eor") { write(0x43); write(n); return true; }
    if(name == "lda") { write(0xa3); write(n); return true; }
    if(name == "ora") { write(0x03); write(n); return true; }
    if(name == "sbc") { write(0xe3); write(n); return true; }
    if(name == "sta") { write(0x83); write(n); return true; }
    return false;
  }

  if(arg.wildcard("(?*,s),y")) {
    arg.ltrim<1>("(");
    arg.rtrim<1>(",s),y");
    unsigned n = eval(arg);
    if(name == "adc") { write(0x73); write(n); return true; }
    if(name == "and") { write(0x33); write(n); return true; }
    if(name == "cmp") { write(0xd3); write(n); return true; }
    if(name == "eor") { write(0x53); write(n); return true; }
    if(name == "lda") { write(0xb3); write(n); return true; }
    if(name == "ora") { write(0x13); write(n); return true; }
    if(name == "sbc") { write(0xf3); write(n); return true; }
    if(name == "sta") { write(0x93); write(n); return true; }
    return false;
  }

  if(arg.wildcard("(?*,x)")) {
    arg.ltrim<1>("(");
    arg.rtrim<2>(",x)");
    unsigned n = eval(arg);
    if(name == "adc") { write(0x61); write(n, 1); return true; }
    if(name == "and") { write(0x21); write(n, 1); return true; }
    if(name == "cmp") { write(0xc1); write(n, 1); return true; }
    if(name == "eor") { write(0x41); write(n, 1); return true; }
    if(name == "jmp") { write(0x7c); write(n, 2); return true; }
    if(name == "jsr") { write(0xfc); write(n, 2); return true; }
    if(name == "lda") { write(0xa1); write(n, 1); return true; }
    if(name == "ora") { write(0x01); write(n, 1); return true; }
    if(name == "sbc") { write(0xe1); write(n, 1); return true; }
    if(name == "sta") { write(0x81); write(n, 1); return true; }
    return false;
  }

  if(arg.wildcard("(?*),y")) {
    arg.ltrim<1>("(");
    arg.rtrim<1>("),y");
    unsigned n = eval(arg);
    if(name == "adc") { write(0x71); write(n); return true; }
    if(name == "and") { write(0x31); write(n); return true; }
    if(name == "cmp") { write(0xd1); write(n); return true; }
    if(name == "eor") { write(0x51); write(n); return true; }
    if(name == "lda") { write(0xb1); write(n); return true; }
    if(name == "ora") { write(0x11); write(n); return true; }
    if(name == "sbc") { write(0xf1); write(n); return true; }
    if(name == "sta") { write(0x91); write(n); return true; }
    return false;
  }

  if(arg.wildcard("(?*)")) {
    arg.ltrim<1>("(");
    arg.rtrim<1>(")");
    unsigned n = eval(arg);
    if(name == "adc") { write(0x72); write(n, 1); return true; }
    if(name == "and") { write(0x32); write(n, 1); return true; }
    if(name == "cmp") { write(0xd2); write(n, 1); return true; }
    if(name == "eor") { write(0x52); write(n, 1); return true; }
    if(name == "jmp") { write(0x6c); write(n, 2); return true; }
    if(name == "lda") { write(0xb2); write(n, 1); return true; }
    if(name == "ora") { write(0x12); write(n, 1); return true; }
    if(name == "pei") { write(0xd4); write(n, 1); return true; }
    if(name == "sbc") { write(0xf2); write(n, 1); return true; }
    if(name == "sta") { write(0x92); write(n, 1); return true; }
    return false;
  }

  if(arg.wildcard("[?*],y")) {
    arg.ltrim<1>("[");
    arg.rtrim<1>("],y");
    unsigned n = eval(arg);
    if(name == "adc") { write(0x77); write(n); return true; }
    if(name == "and") { write(0x37); write(n); return true; }
    if(name == "cmp") { write(0xd7); write(n); return true; }
    if(name == "eor") { write(0x57); write(n); return true; }
    if(name == "lda") { write(0xb7); write(n); return true; }
    if(name == "ora") { write(0x17); write(n); return true; }
    if(name == "sbc") { write(0xf7); write(n); return true; }
    if(name == "sta") { write(0x97); write(n); return true; }
    return false;
  }

  if(arg.wildcard("[?*]")) {
    arg.ltrim<1>("[");
    arg.rtrim<1>("]");
    unsigned n = eval(arg);
    if(name == "adc") { write(0x67); write(n, 1); return true; }
    if(name == "and") { write(0x27); write(n, 1); return true; }
    if(name == "cmp") { write(0xc7); write(n, 1); return true; }
    if(name == "eor") { write(0x47); write(n, 1); return true; }
    if(name == "jmp") { write(0xdc); write(n, 2); return true; }
    if(name == "lda") { write(0xa7); write(n, 1); return true; }
    if(name == "ora") { write(0x07); write(n, 1); return true; }
    if(name == "sbc") { write(0xe7); write(n, 1); return true; }
    if(name == "sta") { write(0x87); write(n, 1); return true; }
    return false;
  }

  if(arg.wildcard("?*,x")) {
    arg.rtrim<1>(",x");
    unsigned n = eval(arg);
    detectSize();
    if(size == 3 || size == 0) {
      if(name == "adc") { write(0x7f); write(n, 3); return true; }
      if(name == "and") { write(0x3f); write(n, 3); return true; }
      if(name == "cmp") { write(0xdf); write(n, 3); return true; }
      if(name == "eor") { write(0x5f); write(n, 3); return true; }
      if(name == "lda") { write(0xbf); write(n, 3); return true; }
      if(name == "ora") { write(0x1f); write(n, 3); return true; }
      if(name == "sbc") { write(0xff); write(n, 3); return true; }
      if(name == "sta") { write(0x9f); write(n, 3); return true; }
    }
    if(size == 2 || size == 0) {
      if(name == "adc") { write(0x7d); write(n, 2); return true; }
      if(name == "and") { write(0x3d); write(n, 2); return true; }
      if(name == "asl") { write(0x1e); write(n, 2); return true; }
      if(name == "bit") { write(0x3c); write(n, 2); return true; }
      if(name == "cmp") { write(0xdd); write(n, 2); return true; }
      if(name == "dec") { write(0xde); write(n, 2); return true; }
      if(name == "eor") { write(0x5d); write(n, 2); return true; }
      if(name == "inc") { write(0xfe); write(n, 2); return true; }
      if(name == "lda") { write(0xbd); write(n, 2); return true; }
      if(name == "ldy") { write(0xbc); write(n, 2); return true; }
      if(name == "lsr") { write(0x5e); write(n, 2); return true; }
      if(name == "ora") { write(0x1d); write(n, 2); return true; }
      if(name == "rol") { write(0x3e); write(n, 2); return true; }
      if(name == "ror") { write(0x7e); write(n, 2); return true; }
      if(name == "sbc") { write(0xfd); write(n, 2); return true; }
      if(name == "sta") { write(0x9d); write(n, 2); return true; }
      if(name == "stz") { write(0x9e); write(n, 2); return true; }
    }
    if(size == 1 || size == 0) {
      if(name == "adc") { write(0x75); write(n, 1); return true; }
      if(name == "and") { write(0x35); write(n, 1); return true; }
      if(name == "asl") { write(0x16); write(n, 1); return true; }
      if(name == "bit") { write(0x34); write(n, 1); return true; }
      if(name == "cmp") { write(0xd5); write(n, 1); return true; }
      if(name == "dec") { write(0xd6); write(n, 1); return true; }
      if(name == "eor") { write(0x55); write(n, 1); return true; }
      if(name == "inc") { write(0xf6); write(n, 1); return true; }
      if(name == "lda") { write(0xb5); write(n, 1); return true; }
      if(name == "ldy") { write(0xb4); write(n, 1); return true; }
      if(name == "lsr") { write(0x56); write(n, 1); return true; }
      if(name == "ora") { write(0x15); write(n, 1); return true; }
      if(name == "rol") { write(0x36); write(n, 1); return true; }
      if(name == "ror") { write(0x76); write(n, 1); return true; }
      if(name == "sbc") { write(0xf5); write(n, 1); return true; }
      if(name == "sta") { write(0x95); write(n, 1); return true; }
      if(name == "sty") { write(0x94); write(n, 1); return true; }
      if(name == "stz") { write(0x74); write(n, 1); return true; }
    }
    return false;
  }

  if(arg.wildcard("?*,y")) {
    arg.rtrim<1>(",y");
    unsigned n = eval(arg);
    detectSize();
    if(size == 2 || size == 0) {
      if(name == "adc") { write(0x79); write(n, 2); return true; }
      if(name == "and") { write(0x39); write(n, 2); return true; }
      if(name == "cmp") { write(0xd9); write(n, 2); return true; }
      if(name == "eor") { write(0x59); write(n, 2); return true; }
      if(name == "lda") { write(0xb9); write(n, 2); return true; }
      if(name == "ldx") { write(0xbe); write(n, 2); return true; }
      if(name == "ora") { write(0x19); write(n, 2); return true; }
      if(name == "sbc") { write(0xf9); write(n, 2); return true; }
      if(name == "sta") { write(0x99); write(n, 2); return true; }
    }
    if(size == 1 || size == 0) {
      if(name == "ldx") { write(0xb6); write(n, 1); return true; }
      if(name == "stx") { write(0x96); write(n, 1); return true; }
    }
    return false;
  }

  if(arg.wildcard("?*,?*")) {
    lstring bank;
    bank.split(",", arg);
    if(name == "mvn") { write(0x54); write(eval(bank[1])); write(eval(bank[0])); return true; }
    if(name == "mvp") { write(0x44); write(eval(bank[1])); write(eval(bank[0])); return true; }
    return false;
  }

  signed n = eval(arg);

  if(name == "brl") {
    if(!arg.wildcard("$????")) n = n - (pc() + 3);
    if(pass == 2 && (n < -32768 || n > 32767)) warning("branch out of bounds");
    write(0x82);
    write(n, 2);
    return true;
  }

  function<bool (const string&, uint8_t)> relative = [this, &name, &arg, &n](const string &test, uint8_t opcode) {
    if(name != test) return false;
    if(!arg.wildcard("$??")) n = n - (this->pc() + 2);
    if(pass == 2 && (n < -128 || n > 127)) this->warning("branch out of bounds");
    this->write(opcode);
    this->write(n);
    return true;
  };
  if(relative("bcc", 0x90)) return true;
  if(relative("bcs", 0xb0)) return true;
  if(relative("beq", 0xf0)) return true;
  if(relative("bne", 0xd0)) return true;
  if(relative("bmi", 0x30)) return true;
  if(relative("bpl", 0x10)) return true;
  if(relative("bvc", 0x50)) return true;
  if(relative("bvs", 0x70)) return true;
  if(relative("bra", 0x80)) return true;

  detectSize();
  if(size == 3 || size == 0) {
    if(name == "adc") { write(0x6f); write(n, 3); return true; }
    if(name == "and") { write(0x2f); write(n, 3); return true; }
    if(name == "cmp") { write(0xcf); write(n, 3); return true; }
    if(name == "eor") { write(0x4f); write(n, 3); return true; }
    if(name == "jml") { write(0x5c); write(n, 3); return true; }
    if(name == "jsl") { write(0x22); write(n, 3); return true; }
    if(name == "lda") { write(0xaf); write(n, 3); return true; }
    if(name == "ora") { write(0x0f); write(n, 3); return true; }
    if(name == "sbc") { write(0xef); write(n, 3); return true; }
    if(name == "sta") { write(0x8f); write(n, 3); return true; }
  }
  if(size == 2 || size == 0) {
    if(name == "adc") { write(0x6d); write(n, 2); return true; }
    if(name == "and") { write(0x2d); write(n, 2); return true; }
    if(name == "asl") { write(0x0e); write(n, 2); return true; }
    if(name == "bit") { write(0x2c); write(n, 2); return true; }
    if(name == "cmp") { write(0xcd); write(n, 2); return true; }
    if(name == "cpx") { write(0xec); write(n, 2); return true; }
    if(name == "cpy") { write(0xcc); write(n, 2); return true; }
    if(name == "dec") { write(0xce); write(n, 2); return true; }
    if(name == "eor") { write(0x4d); write(n, 2); return true; }
    if(name == "inc") { write(0xee); write(n, 2); return true; }
    if(name == "jmp") { write(0x4c); write(n, 2); return true; }
    if(name == "jsr") { write(0x20); write(n, 2); return true; }
    if(name == "lda") { write(0xad); write(n, 2); return true; }
    if(name == "ldx") { write(0xae); write(n, 2); return true; }
    if(name == "ldy") { write(0xac); write(n, 2); return true; }
    if(name == "lsr") { write(0x4e); write(n, 2); return true; }
    if(name == "ora") { write(0x0d); write(n, 2); return true; }
    if(name == "pea") { write(0xf4); write(n, 2); return true; }
    if(name == "per") { write(0x62); write(n, 2); return true; }
    if(name == "rol") { write(0x2e); write(n, 2); return true; }
    if(name == "ror") { write(0x6e); write(n, 2); return true; }
    if(name == "sbc") { write(0xed); write(n, 2); return true; }
    if(name == "sta") { write(0x8d); write(n, 2); return true; }
    if(name == "stx") { write(0x8e); write(n, 2); return true; }
    if(name == "sty") { write(0x8c); write(n, 2); return true; }
    if(name == "stz") { write(0x9c); write(n, 2); return true; }
    if(name == "trb") { write(0x1c); write(n, 2); return true; }
    if(name == "tsb") { write(0x0c); write(n, 2); return true; }
  }
  if(size == 1 || size == 0) {
    if(name == "adc") { write(0x65); write(n, 1); return true; }
    if(name == "and") { write(0x25); write(n, 1); return true; }
    if(name == "asl") { write(0x06); write(n, 1); return true; }
    if(name == "bit") { write(0x24); write(n, 1); return true; }
    if(name == "cmp") { write(0xc5); write(n, 1); return true; }
    if(name == "cpx") { write(0xe4); write(n, 1); return true; }
    if(name == "cpy") { write(0xc4); write(n, 1); return true; }
    if(name == "dec") { write(0xc6); write(n, 1); return true; }
    if(name == "eor") { write(0x45); write(n, 1); return true; }
    if(name == "inc") { write(0xe6); write(n, 1); return true; }
    if(name == "lda") { write(0xa5); write(n, 1); return true; }
    if(name == "ldx") { write(0xa6); write(n, 1); return true; }
    if(name == "ldy") { write(0xa4); write(n, 1); return true; }
    if(name == "lsr") { write(0x46); write(n, 1); return true; }
    if(name == "ora") { write(0x05); write(n, 1); return true; }
    if(name == "rol") { write(0x26); write(n, 1); return true; }
    if(name == "ror") { write(0x66); write(n, 1); return true; }
    if(name == "sbc") { write(0xe5); write(n, 1); return true; }
    if(name == "sta") { write(0x85); write(n, 1); return true; }
    if(name == "stx") { write(0x86); write(n, 1); return true; }
    if(name == "sty") { write(0x84); write(n, 1); return true; }
    if(name == "stz") { write(0x64); write(n, 1); return true; }
    if(name == "trb") { write(0x14); write(n, 1); return true; }
    if(name == "tsb") { write(0x04); write(n, 1); return true; }
  }
  return false;
}
