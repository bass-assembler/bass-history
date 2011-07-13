bool BassX86::assembleBlock(const string &block) {
  if(Bass::assembleBlock(block) == true) return true;

  lstring part, byte;
  part.split<1>(" ", block);
  string name = part[0], args = part[1];
  part.split<1>(",", args);

  tuple<unsigned, unsigned> immediate;

  foreach(f, family) if(args.wildcard(f.pattern)) {
    foreach(o, f.opcode) if(name == o.name) switch(o.mode) {
    case Mode::Implied:
      writePrefix(o);
      return true;
    case Mode::ModRM8:
    case Mode::ModRM32:
      if(effectiveAddress(o, part[0]) == false) continue;
      if(reg(o, part[1]) == false) continue;
      writeEffectiveAddress(o);
      return true;
    case Mode::ModRM8Reverse:
    case Mode::ModRM32Reverse:
      if(effectiveAddress(o, part[1]) == false) continue;
      if(reg(o, part[0]) == false) continue;
      writeEffectiveAddress(o);
      return true;
    case Mode::EA8:
    case Mode::EA32:
      if(effectiveAddress(o, part[0]) == false) continue;
      info.r = o.param0;
      writeEffectiveAddress(o);
      return true;
    case Mode::EA8_IMM8:
      if(effectiveAddress(o, part[0]) == false) continue;
      info.r = o.param0;
      writeEffectiveAddress(o);
      write(eval(part[1]), 1);
      return true;
    case Mode::EA32_IMM32:
      if(effectiveAddress(o, part[0]) == false) continue;
      info.r = o.param0;
      immediate = size(part[1]);
      if(get<0>(immediate) == 8) continue;
      writeEffectiveAddress(o);
      write(get<1>(immediate), get<0>(immediate) == 16 ? 2 : 4);
      return true;
/*
    case Mode::Immediate8:
      if(size && size != 8) continue;
      writePrefix(o);
      write(eval(args), 1);
      return true;
    case Mode::Immediate16:
      if(size && size != 16) continue;
      writePrefix(o);
      write(eval(args), 2);
      return true;
    case Mode::Immediate32:
      if(size && size != 32) continue;
      writePrefix(o);
      write(eval(args), 4);
      return true;
    case Mode::Immediate8S:
      if(size && size != 8) continue;
      if(isRegister(part[1])) continue;
      writePrefix(o);
      write(eval(part[1]), 1);
      return true;
    case Mode::Immediate16S:
      if(size && size != 16) continue;
      if(isRegister(part[1])) continue;
      writePrefix(o);
      write(eval(part[1]), 2);
      return true;
    case Mode::Immediate32S:
      if(size && size != 32) continue;
      if(isRegister(part[1])) continue;
      writePrefix(o);
      write(eval(part[1]), 4);
      return true;
*/
    case Mode::R32:
      if(auto p = reg16(args)) {
        write(0x66);
        write(o.prefix + p());
        return true;
      }
      if(auto p = reg32(args)) {
        write(o.prefix + p());
        return true;
      }
      continue;
/*
    case Mode::Relative8:
      if(size && size != 8) continue;
      writePrefix(o);
      addr = eval(args) - (pc() + 1);
      if(pass == 2 && (addr <   -128 || addr >   +127)) error("branch out of bounds");
      write(addr, 1);
      return true;
    case Mode::Relative16:
      if(size && size != 16) continue;
      writePrefix(o);
      addr = eval(args) - (pc() + 2);
      if(pass == 2 && (addr < -32768 || addr > +32767)) error("branch out of bounds");
      write(addr, 2);
      return true;
    case Mode::Relative32:
      if(size && size != 32) continue;
      writePrefix(o);
      addr = eval(args) - (pc() + 4);
      write(addr, 4);
      return true;
*/
    }
  }

  return false;
}

unsigned BassX86::prefixLength(const Opcode &o) const {
  if(o.prefix >> 24) return 4;
  if(o.prefix >> 16) return 3;
  if(o.prefix >>  8) return 2;
                     return 1;
}

void BassX86::writePrefix(const Opcode &o) {
  if(o.prefix >> 24) write(o.prefix >> 24);
  if(o.prefix >> 16) write(o.prefix >> 16);
  if(o.prefix >>  8) write(o.prefix >>  8);
                     write(o.prefix >>  0);
}

bool BassX86::isRegister(const string &s) const {
  if(s == "al") return true;
  if(s == "bl") return true;
  if(s == "cl") return true;
  if(s == "dl") return true;
  if(s == "ah") return true;
  if(s == "bh") return true;
  if(s == "ch") return true;
  if(s == "dh") return true;

  if(s == "ax") return true;
  if(s == "bx") return true;
  if(s == "cx") return true;
  if(s == "dx") return true;
  if(s == "sp") return true;
  if(s == "bp") return true;
  if(s == "si") return true;
  if(s == "di") return true;

  if(s == "eax") return true;
  if(s == "ebx") return true;
  if(s == "ecx") return true;
  if(s == "edx") return true;
  if(s == "esp") return true;
  if(s == "ebp") return true;
  if(s == "esi") return true;
  if(s == "edi") return true;

  return false;
}

optional<unsigned> BassX86::reg8(const string &s) const {
  if(s == "al") return { true, 0 };
  if(s == "cl") return { true, 1 };
  if(s == "dl") return { true, 2 };
  if(s == "bl") return { true, 3 };
  if(s == "ah") return { true, 4 };
  if(s == "ch") return { true, 5 };
  if(s == "dh") return { true, 6 };
  if(s == "bh") return { true, 7 };
  return { false, 0 };
}

optional<unsigned> BassX86::reg16(const string &s) const {
  if(s == "ax") return { true, 0 };
  if(s == "cx") return { true, 1 };
  if(s == "dx") return { true, 2 };
  if(s == "bx") return { true, 3 };
  if(s == "sp") return { true, 4 };
  if(s == "bp") return { true, 5 };
  if(s == "si") return { true, 6 };
  if(s == "di") return { true, 7 };
  return { false, 0 };
}

optional<unsigned> BassX86::reg32(const string &s) const {
  if(s == "eax") return { true, 0 };
  if(s == "ecx") return { true, 1 };
  if(s == "edx") return { true, 2 };
  if(s == "ebx") return { true, 3 };
  if(s == "esp") return { true, 4 };
  if(s == "ebp") return { true, 5 };
  if(s == "esi") return { true, 6 };
  if(s == "edi") return { true, 7 };
  return { false, 0 };
}

optional<unsigned> BassX86::reg32m(string s, optional<unsigned> &multiplier) const {
       if(s.endswith("*1")) { multiplier = { true, 0 }; s.rtrim<1>("*1"); }
  else if(s.endswith("*2")) { multiplier = { true, 1 }; s.rtrim<1>("*2"); }
  else if(s.endswith("*4")) { multiplier = { true, 2 }; s.rtrim<1>("*4"); }
  else if(s.endswith("*8")) { multiplier = { true, 3 }; s.rtrim<1>("*8"); }
  else multiplier = { false, 0 };

  if(s == "eax") return { true, 0 };
  if(s == "ecx") return { true, 1 };
  if(s == "edx") return { true, 2 };
  if(s == "ebx") return { true, 3 };
  if(s == "esp") return { true, 4 };
  if(s == "ebp") return { true, 5 };
  if(s == "esi") return { true, 6 };
  if(s == "edi") return { true, 7 };
  return { false, 0 };
}

tuple<unsigned, unsigned> BassX86::size(const string &s) {
  if(s.beginswith("<")) return tuple<unsigned, unsigned>( 8u, eval((const char*)s + 1));
  if(s.beginswith(">")) return tuple<unsigned, unsigned>(16u, eval((const char*)s + 1));
  if(s.beginswith("^")) return tuple<unsigned, unsigned>(32u, eval((const char*)s + 1));
  if(s.wildcard("0x??"      )) return tuple<unsigned, unsigned>( 8u, eval(s));
  if(s.wildcard("0x????"    )) return tuple<unsigned, unsigned>(16u, eval(s));
  if(s.wildcard("0x????????")) return tuple<unsigned, unsigned>(32u, eval(s));
  return tuple<unsigned, unsigned>(0u, eval(s));  //unknown size (assume 32, then 16, then 8)
}

bool BassX86::effectiveAddress(const Opcode &o, string ea) {
  info.rp = false;
  info.mp = false;
  info.seg = 0x00;
  info.mod = 0;
  info.m = 0;
  info.sib = false;
  info.multiplier = 0;
  info.r0 = 0;
  info.r1 = 0;
  info.ds = 0;
  info.dd = 0;

  tuple<unsigned, unsigned> immediate;

  bool indirect = ea.wildcard("[*]");
  if(indirect) {
    ea.ltrim<1>("[");
    ea.rtrim<1>("]");

         if(ea.beginswith("es:")) { ea.ltrim<1>("es:"); info.seg = 0x26; }
    else if(ea.beginswith("cs:")) { ea.ltrim<1>("cs:"); info.seg = 0x2e; }
    else if(ea.beginswith("ss:")) { ea.ltrim<1>("ss:"); info.seg = 0x36; }
    else if(ea.beginswith("ds:")) { ea.ltrim<1>("ds:"); info.seg = 0x3e; }
    else if(ea.beginswith("fs:")) { ea.ltrim<1>("fs:"); info.seg = 0x64; }
    else if(ea.beginswith("gs:")) { ea.ltrim<1>("gs:"); info.seg = 0x65; }

    optional<unsigned> multiplier = { false, 0 };

    lstring part;
    part.split("+", ea);

    if(auto p = reg32m(part[0], multiplier)) {
      //[reg...] {M32}
      if(multiplier) info.multiplier = multiplier();

      info.r0 = p();
      if(part.size() == 1) {
        if(multiplier == false) {
          //[reg]
          info.mod = 0, info.m = info.r0;
        } else {
          //[reg*multiplier] {SIB}
          info.mod = 0, info.m = 4, info.sib = true, info.r1 = 5, info.ds = 4, info.dd = 0;
        }
      } else {
        ea.ltrim<1>(part[0]);
        ea.ltrim<1>("+");
        if(auto p = reg32(part[1])) {
          //[reg+reg...] {SIB}
          info.r1 = p();
          if(part.size() == 2) {
            //[reg+reg] {SIB}
            if(info.r0 == 4) return false;
            if(info.r1 != 5) {
              info.mod = 0, info.m = 4, info.sib = true;
            } else {
              info.mod = 1, info.m = 4, info.sib = true, info.ds = 1, info.dd = 0;
            }
          } else {
            //[reg+reg+displacement] {SIB}
            ea.ltrim<1>(part[1]);
            ea.ltrim<1>("+");
            if(info.r0 == 4) return false;
            immediate = size(ea), info.mod = (get<0>(immediate) == 8 ? 1 : 2), info.m = 4, info.sib = true, info.ds = (get<0>(immediate) == 8 ? 1 : 4), info.dd = get<1>(immediate);
          }
        } else {
          //[reg+displacement]
          immediate = size(ea), info.mod = (get<0>(immediate) == 8 ? 1 : 2), info.m = info.r0, info.ds = (get<0>(immediate) == 8 ? 1 : 4), info.dd = get<1>(immediate);
        }
      }
    } else if(auto p = reg16(part[0])) {
      //[reg...] {M16}
      info.mp = true;
           if(ea.beginswith("bx+si")) { ea.ltrim<1>("bx+si"); info.m = 0; }
      else if(ea.beginswith("bx+di")) { ea.ltrim<1>("bx+di"); info.m = 1; }
      else if(ea.beginswith("bp+si")) { ea.ltrim<1>("bp+si"); info.m = 2; }
      else if(ea.beginswith("bp+di")) { ea.ltrim<1>("bp+di"); info.m = 3; }
      else if(ea.beginswith("si"   )) { ea.ltrim<1>("si"   ); info.m = 4; }
      else if(ea.beginswith("di"   )) { ea.ltrim<1>("di"   ); info.m = 5; }
      else if(ea.beginswith("bp"   )) { ea.ltrim<1>("bp"   ); info.m = 6; }
      else if(ea.beginswith("bx"   )) { ea.ltrim<1>("bx"   ); info.m = 7; }
      else return false;

      if(ea.beginswith("+") == false) {
        //[reg]
        if(info.m != 6) {
          info.mod = 0;
        } else {
          info.mod = 1, info.ds = 1, info.dd = 0;
        }
      } else {
        //[reg+displacement]
        ea.ltrim<1>("+");
        immediate = size(ea);
        info.mod = (get<0>(immediate) == 8 ? 1 : 2), info.ds = (get<0>(immediate) == 8 ? 1 : 2), info.dd = get<1>(immediate);
      }
    } else {
      //[displacement]
      immediate = size(ea);
      if(get<0>(immediate) != 16) {
        //[displacement] {M32}
        info.mod = 0, info.m = 5, info.ds = 4, info.dd = get<1>(immediate);
      } else {
        //[displacement] {M16}
        info.mp = true, info.mod = 0, info.m = 6, info.ds = 2, info.dd = get<1>(immediate);
      }
    }
  } else {
    //reg
    if(o.flag == Flag::Short) {
      if(auto p = reg8(ea)) {
        info.mod = 3, info.m = p();
      } else return false;
    } else if(o.flag == Flag::Long) {
      if(auto p = reg32(ea)) {
        info.mod = 3, info.m = p();
      } else if(auto p = reg16(ea)) {
        info.rp = true, info.mod = 3, info.m = p();
      } else return false;
    }
  }

  return true;
}

bool BassX86::reg(const Opcode &o, const string &r) {
  if(o.flag == Flag::Short) {
    if(auto p = reg8(r)) {
      info.r = p();
    } else return false;
  } else if(o.flag == Flag::Long) {
    if(auto p = reg32(r)) {
      info.r = p();
    } else if(auto p = reg16(r)) {
      info.rp = true, info.r = p();
    } else return false;
  }
  return true;
}

void BassX86::writeEffectiveAddress(const Opcode &o) {
  if(info.seg) write(info.seg);
  if(info.rp) write(0x66);
  if(info.mp) write(0x67);
  writePrefix(o);
  write((info.mod << 6) | (info.r << 3) | (info.m << 0));
  if(info.sib) write((info.multiplier << 6) | (info.r0 << 3) | (info.r1 << 0));
  if(info.ds) write(info.dd, info.ds);
}

BassX86::BassX86() {
  Opcode table[] = {
    {     0x00, "add    *,*  ", Mode::ModRM8, Flag::Short },
    {     0x01, "add    *,*  ", Mode::ModRM32, Flag::Long },
    {     0x02, "add    *,*  ", Mode::ModRM8Reverse, Flag::Short },
    {     0x03, "add    *,*  ", Mode::ModRM32Reverse, Flag::Long },
    {     0x04, "add    al,* ", Mode::Immediate8S },
    {   0x6605, "add    ax,* ", Mode::Immediate16S },
    {     0x05, "add    eax,*", Mode::Immediate32S },

    {     0x06, "push   es   ", Mode::Implied },
    {     0x07, "pop    es   ", Mode::Implied },

    {     0x08, "or     *,*  ", Mode::ModRM8, Flag::Short },
    {     0x09, "or     *,*  ", Mode::ModRM32, Flag::Long },
    {     0x0a, "or     *,*  ", Mode::ModRM8Reverse, Flag::Short },
    {     0x0b, "or     *,*  ", Mode::ModRM32Reverse, Flag::Long },
    {     0x0c, "or     al,* ", Mode::Immediate8S },
    {   0x660c, "or     ax,* ", Mode::Immediate16S },
    {     0x0d, "or     eax,*", Mode::Immediate32S },
    {     0x0e, "push   cs   ", Mode::Implied },

    {     0x10, "adc    *,*  ", Mode::ModRM8, Flag::Short },
    {     0x11, "adc    *,*  ", Mode::ModRM32, Flag::Long },
    {     0x12, "adc    *,*  ", Mode::ModRM8Reverse, Flag::Short },
    {     0x13, "adc    *,*  ", Mode::ModRM32Reverse, Flag::Long },
    {     0x14, "adc    al,* ", Mode::Immediate8S },
    {     0x15, "adc    ax,* ", Mode::Immediate16S },
    {     0x15, "adc    eax,*", Mode::Immediate32S },

    {     0x16, "push   ss   ", Mode::Implied },
    {     0x17, "pop    ss   ", Mode::Implied },

    {     0x18, "sbb    *,*  ", Mode::ModRM8, Flag::Short },
    {     0x19, "sbb    *,*  ", Mode::ModRM32, Flag::Long },
    {     0x1a, "sbb    *,*  ", Mode::ModRM8Reverse, Flag::Short },
    {     0x1b, "sbb    *,*  ", Mode::ModRM32Reverse, Flag::Long },
    {     0x1c, "sbb    al,* ", Mode::Immediate8S },
    {   0x661d, "sbb    ax,* ", Mode::Immediate16S },
    {     0x1d, "sbb    eax,*", Mode::Immediate32S },

    {     0x1e, "push   ds   ", Mode::Implied },
    {     0x1f, "pop    ds   ", Mode::Implied },

    {     0x26, "es          ", Mode::Implied, Flag::Prefix },
    {     0x2e, "cs          ", Mode::Implied, Flag::Prefix },
    {     0x36, "ss          ", Mode::Implied, Flag::Prefix },
    {     0x3e, "ds          ", Mode::Implied, Flag::Prefix },

    {     0x40, "inc    *    ", Mode::R32 },
    {     0x48, "dec    *    ", Mode::R32 },
    {     0x50, "push   *    ", Mode::R32 },
    {     0x58, "pop    *    ", Mode::R32 },

    {     0x60, "pushad      ", Mode::Implied },
    {     0x61, "popad       ", Mode::Implied },

    {     0x64, "fs          ", Mode::Implied, Flag::Prefix },
    {     0x65, "gs          ", Mode::Implied, Flag::Prefix },

    {     0x68, "push   *    ", Mode::Immediate32 },
    {   0x6668, "push   *    ", Mode::Immediate16 },
    {     0x6a, "push   *    ", Mode::Immediate8 },

    {     0x70, "jo     *    ", Mode::Relative8 },
    {     0x71, "jno    *    ", Mode::Relative8 },
    {     0x72, "jc     *    ", Mode::Relative8 },
    {     0x73, "jnc    *    ", Mode::Relative8 },
    {     0x74, "jz     *    ", Mode::Relative8 },
    {     0x75, "jnz    *    ", Mode::Relative8 },
    {     0x76, "jna    *    ", Mode::Relative8 },
    {     0x77, "ja     *    ", Mode::Relative8 },
    {     0x78, "js     *    ", Mode::Relative8 },
    {     0x79, "jns    *    ", Mode::Relative8 },
    {     0x7a, "jp     *    ", Mode::Relative8 },
    {     0x7b, "jnp    *    ", Mode::Relative8 },
    {     0x7c, "jl     *    ", Mode::Relative8 },
    {     0x7d, "jnl    *    ", Mode::Relative8 },
    {     0x7e, "jng    *    ", Mode::Relative8 },
    {     0x7f, "jg     *    ", Mode::Relative8 },

    {     0x80, "add    *,*  ", Mode::EA8_IMM8, Flag::Short, 0 },
    {     0x80, "or     *,*  ", Mode::EA8_IMM8, Flag::Short, 1 },
    {     0x80, "adc    *,*  ", Mode::EA8_IMM8, Flag::Short, 2 },
    {     0x80, "sbb    *,*  ", Mode::EA8_IMM8, Flag::Short, 3 },
    {     0x80, "and    *,*  ", Mode::EA8_IMM8, Flag::Short, 4 },
    {     0x80, "sub    *,*  ", Mode::EA8_IMM8, Flag::Short, 5 },
    {     0x80, "xor    *,*  ", Mode::EA8_IMM8, Flag::Short, 6 },
    {     0x80, "cmp    *,*  ", Mode::EA8_IMM8, Flag::Short, 7 },

    {     0x81, "add    *,*  ", Mode::EA32_IMM32, Flag::Long, 0 },
    {     0x81, "or     *,*  ", Mode::EA32_IMM32, Flag::Long, 1 },
    {     0x81, "adc    *,*  ", Mode::EA32_IMM32, Flag::Long, 2 },
    {     0x81, "sbb    *,*  ", Mode::EA32_IMM32, Flag::Long, 3 },
    {     0x81, "and    *,*  ", Mode::EA32_IMM32, Flag::Long, 4 },
    {     0x81, "sub    *,*  ", Mode::EA32_IMM32, Flag::Long, 5 },
    {     0x81, "xor    *,*  ", Mode::EA32_IMM32, Flag::Long, 6 },
    {     0x81, "cmp    *,*  ", Mode::EA32_IMM32, Flag::Long, 7 },

    {     0x90, "nop         ", Mode::Implied },
    {     0xe0, "loopnz *    ", Mode::Relative8 },
    {     0xe9, "jmp    *    ", Mode::Relative32 },
    {   0x66e9, "jmp    *    ", Mode::Relative16 },
    {     0xeb, "jmp    *    ", Mode::Relative8 },

    {     0xf0, "lock        ", Mode::Implied, Flag::Prefix },
    {     0xf2, "repnz       ", Mode::Implied, Flag::Prefix },
    {     0xf3, "repz        ", Mode::Implied, Flag::Prefix },

    {     0xfe, "inc    *    ", Mode::EA8, Flag::Short, 0 },
    {     0xfe, "dec    *    ", Mode::EA8, Flag::Short, 1 },

    {     0xff, "inc    *    ", Mode::EA32, Flag::Long, 0 },
    {     0xff, "dec    *    ", Mode::EA32, Flag::Long, 0 },

    {   0x0f80, "jo     *    ", Mode::Relative32 },
    { 0x660f80, "jo     *    ", Mode::Relative16 },
    {   0x0f81, "jno    *    ", Mode::Relative32 },
    { 0x660f81, "jno    *    ", Mode::Relative16 },
    {   0x0f82, "jc     *    ", Mode::Relative32 },
    { 0x660f82, "jc     *    ", Mode::Relative16 },
    {   0x0f83, "jnc    *    ", Mode::Relative32 },
    { 0x660f83, "jnc    *    ", Mode::Relative16 },
    {   0x0f84, "jz     *    ", Mode::Relative32 },
    { 0x660f84, "jz     *    ", Mode::Relative16 },
    {   0x0f85, "jnz    *    ", Mode::Relative32 },
    { 0x660f85, "jnz    *    ", Mode::Relative16 },
    {   0x0f86, "jna    *    ", Mode::Relative32 },
    { 0x660f86, "jna    *    ", Mode::Relative16 },
    {   0x0f87, "ja     *    ", Mode::Relative32 },
    { 0x660f87, "ja     *    ", Mode::Relative16 },
    {   0x0f88, "js     *    ", Mode::Relative32 },
    { 0x660f88, "js     *    ", Mode::Relative16 },
    {   0x0f89, "jns    *    ", Mode::Relative32 },
    { 0x660f89, "jns    *    ", Mode::Relative16 },
    {   0x0f8a, "jp     *    ", Mode::Relative32 },
    { 0x660f8a, "jp     *    ", Mode::Relative16 },
    {   0x0f8b, "jnp    *    ", Mode::Relative32 },
    { 0x660f8b, "jnp    *    ", Mode::Relative16 },
    {   0x0f8c, "jl     *    ", Mode::Relative32 },
    { 0x660f8c, "jl     *    ", Mode::Relative16 },
    {   0x0f8d, "jnl    *    ", Mode::Relative32 },
    { 0x660f8d, "jnl    *    ", Mode::Relative16 },
    {   0x0f8e, "jng    *    ", Mode::Relative32 },
    { 0x660f8e, "jng    *    ", Mode::Relative16 },
    {   0x0f8f, "jg     *    ", Mode::Relative32 },
    { 0x660f8f, "jg     *    ", Mode::Relative16 },
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

  //sort all Relative32 instructions before all Relative8 instructions
  foreach(x, table) {
    foreach(y, table) {
      if(&x == &y);
      else if(x.mode == Mode::Relative32 && y.mode == Mode::Relative16) swap(x, y);
      else if(x.mode == Mode::Relative32 && y.mode == Mode::Relative8 ) swap(x, y);
      else if(x.mode == Mode::Relative16 && y.mode == Mode::Relative8 ) swap(x, y);
      else if(x.mode == Mode::EA32 && y.mode == Mode::EA8) swap(x, y);
      else if(x.mode == Mode::EA32_IMM32 && y.mode == EA8_IMM8) swap(x, y);
    }
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
