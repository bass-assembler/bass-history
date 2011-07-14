bool BassX86::assembleBlock(const string &block) {
  if(Bass::assembleBlock(block) == true) return true;

  lstring part, byte;
  part.split<1>(" ", block);
  string name = part[0], args = part[1];
  part.split<1>(",", args);

  signed addr;

       if(name.endswith("<")) { name.rtrim<1>("<"); info.ps =  8; }
  else if(name.endswith(">")) { name.rtrim<1>(">"); info.ps = 16; }
  else if(name.endswith("^")) { name.rtrim<1>("^"); info.ps = 32; }
  else info.ps = 0;

  foreach(f, family) if(args.wildcard(f.pattern)) {
    foreach(o, f.opcode) if(name == o.name) switch(o.mode) {
    case Mode::Implied:
      writePrefix(o);
      return true;

    case Mode::EffectiveByteRegister:
    case Mode::EffectiveWordRegister:
      if(effectiveAddress(o, part[0]) == false) continue;
      if(reg(o, part[1]) == false) continue;
      writeEffectiveAddress(o);
      return true;

    case Mode::RegisterEffectiveByte:
    case Mode::RegisterEffectiveWord:
      if(effectiveAddress(o, part[1]) == false) continue;
      if(reg(o, part[0]) == false) continue;
      writeEffectiveAddress(o);
      return true;

    case Mode::EffectiveByte:
      if(info.ps && info.ps !=  8) continue;
      if(effectiveAddress(o, part[0]) == false) continue;
      info.r = o.param0;
      writeEffectiveAddress(o);
      return true;

    case Mode::EffectiveWord:
      if(info.ps && info.ps != 16 && info.ps != 32) continue;
      if(effectiveAddress(o, part[0]) == false) continue;
      info.r = o.param0;
      if(info.ps == 16) info.rp = true;
      writeEffectiveAddress(o);
      return true;

    case Mode::EffectiveByteImmediate:
      if(info.ps && info.ps != 8) continue;
      if(effectiveAddress(o, part[0]) == false) continue;
      info.r = o.param0;
      writeEffectiveAddress(o);
      write(eval(part[1]), 1);
      return true;

    case Mode::EffectiveWordImmediate:
      if(info.ps && info.ps != 16 && info.ps != 32) continue;
      if(size(part[1]) && size(part[1]) != 16 && size(part[1]) != 32) continue;
      if(effectiveAddress(o, part[0]) == false) continue;
      info.r = o.param0;
      if(info.ps == 16) info.rp = true;
      writeEffectiveAddress(o);
      write(eval(part[1]), info.ps == 16 ? 2 : 4);
      return true;

    case Mode::EffectiveWordImmediateByte:
      if(info.ps && info.ps != 16 && info.ps != 32) continue;
      if(size(part[1]) && size(part[1]) != 8) continue;
      if(effectiveAddress(o, part[0]) == false) continue;
      info.r = o.param0;
      if(info.ps == 16) info.rp = true;
      writeEffectiveAddress(o);
      write(eval(part[1]), 1);
      return true;

    case Mode::ImmediateByte:
      if(size(args) && size(args) != 8) continue;
      writePrefix(o);
      write(eval(args), 1);
      return true;

    case Mode::ImmediateWord:
      if(size(args) && size(args) != 16 && size(args) != 32) continue;
      info.rp = (size(args) == 16);
      if(info.rp) write(0x66);
      writePrefix(o);
      write(eval(args), info.rp ? 2 : 4);
      return true;

    case Mode::AccumulatorImmediateByte:
      if(part[0] != "al") continue;
      if(size(part[1]) && size(part[1]) !=  8) continue;
      if(isRegister(part[1])) continue;
      writePrefix(o);
      write(eval(part[1]), 1);
      return true;

    case Mode::AccumulatorImmediateWord:
      if(part[0] != "ax" && part[0] != "eax") continue;
      if(size(part[1]) && size(part[1]) != 16 && size(part[1]) != 32) continue;
      if(isRegister(part[1])) continue;
      info.rp = (part[0] == "ax");
      if(info.rp) write(0x66);
      writePrefix(o);
      write(eval(part[1]), info.rp ? 2 : 4);
      return true;

    case Mode::RegisterWord:
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

    case Mode::RelativeByte:
      if(info.ps && info.ps !=  8) continue;
      writePrefix(o);
      addr = eval(args) - (pc() + 1);
      if(pass == 2 && (addr <   -128 || addr >   +127)) error("branch out of bounds");
      write(addr, 1);
      return true;

    case Mode::RelativeWord:
      if(info.ps && info.ps != 16 && info.ps != 32) continue;
      if(info.ps == 16) write(0x66);
      writePrefix(o);
      if(info.ps == 16) {
        addr = eval(args) - (pc() + 2);
        if(pass == 2 && (addr < -32768 || addr > +32767)) error("branch out of bounds");
        write(addr, 2);
      } else {
        addr = eval(args) - (pc() + 4);
        write(addr, 4);
      }
      return true;
    }
  }

  return false;
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

unsigned BassX86::size(const string &s) {
  if(s.beginswith("<")) return  8;
  if(s.beginswith(">")) return 16;
  if(s.beginswith("^")) return 32;
  if(s.wildcard("0x??"      )) return  8;
  if(s.wildcard("0x????"    )) return 16;
  if(s.wildcard("0x????????")) return 32;
  return 0;
}

int64_t BassX86::eval(const string &s) {
  if(s.beginswith("<")) return Bass::eval((const char*)s + 1);
  if(s.beginswith(">")) return Bass::eval((const char*)s + 1);
  if(s.beginswith("^")) return Bass::eval((const char*)s + 1);
  return Bass::eval(s);
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

  unsigned ds;
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
            ds = size(ea), info.mod = (ds == 8 ? 1 : 2), info.m = 4, info.sib = true, info.ds = (ds == 8 ? 1 : 4), info.dd = eval(ea);
          }
        } else {
          //[reg+displacement]
          ds = size(ea), info.mod = (ds == 8 ? 1 : 2), info.m = info.r0, info.ds = (ds == 8 ? 1 : 4), info.dd = eval(ea);
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
        ds = size(ea), info.mod = (ds == 8 ? 1 : 2), info.ds = (ds == 8 ? 1 : 2), info.dd = eval(ea);
      }
    } else {
      //[displacement]
      ds = size(ea);
      if(ds != 16) {
        //[displacement] {M32}
        info.mod = 0, info.m = 5, info.ds = 4, info.dd = eval(ea);
      } else {
        //[displacement] {M16}
        info.mp = true, info.mod = 0, info.m = 6, info.ds = 2, info.dd = eval(ea);
      }
    }
  } else {
    //reg
    if(o.flag == Flag::Byte) {
      if(auto p = reg8(ea)) {
        info.mod = 3, info.m = p();
      } else return false;
    } else if(o.flag == Flag::Word) {
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
  if(o.flag == Flag::Byte) {
    if(auto p = reg8(r)) {
      info.r = p();
    } else return false;
  } else if(o.flag == Flag::Word) {
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
    {     0x00, "add    *,*  ", Mode::EffectiveByteRegister, Flag::Byte },
    {     0x01, "add    *,*  ", Mode::EffectiveWordRegister, Flag::Word },
    {     0x02, "add    *,*  ", Mode::RegisterEffectiveByte, Flag::Byte },
    {     0x03, "add    *,*  ", Mode::RegisterEffectiveWord, Flag::Word },
    {     0x04, "add    *,*  ", Mode::AccumulatorImmediateByte },
    {     0x05, "add    *,*  ", Mode::AccumulatorImmediateWord },

    {     0x06, "push   es   ", Mode::Implied },
    {     0x07, "pop    es   ", Mode::Implied },

    {     0x08, "or     *,*  ", Mode::EffectiveByteRegister, Flag::Byte },
    {     0x09, "or     *,*  ", Mode::EffectiveWordRegister, Flag::Word },
    {     0x0a, "or     *,*  ", Mode::RegisterEffectiveByte, Flag::Byte },
    {     0x0b, "or     *,*  ", Mode::RegisterEffectiveWord, Flag::Word },
    {     0x0c, "or     *,*  ", Mode::AccumulatorImmediateByte },
    {     0x0d, "or     *,*  ", Mode::AccumulatorImmediateWord },

    {     0x0e, "push   cs   ", Mode::Implied },
    //0f = opcode extensions (pop cs removed after 8086)

    {     0x10, "adc    *,*  ", Mode::EffectiveByteRegister, Flag::Byte },
    {     0x11, "adc    *,*  ", Mode::EffectiveWordRegister, Flag::Word },
    {     0x12, "adc    *,*  ", Mode::RegisterEffectiveByte, Flag::Byte },
    {     0x13, "adc    *,*  ", Mode::RegisterEffectiveWord, Flag::Word },
    {     0x14, "adc    *,*  ", Mode::AccumulatorImmediateByte },
    {     0x15, "adc    *,*  ", Mode::AccumulatorImmediateWord },

    {     0x16, "push   ss   ", Mode::Implied },
    {     0x17, "pop    ss   ", Mode::Implied },

    {     0x18, "sbb    *,*  ", Mode::EffectiveByteRegister, Flag::Byte },
    {     0x19, "sbb    *,*  ", Mode::EffectiveWordRegister, Flag::Word },
    {     0x1a, "sbb    *,*  ", Mode::RegisterEffectiveByte, Flag::Byte },
    {     0x1b, "sbb    *,*  ", Mode::RegisterEffectiveWord, Flag::Word },
    {     0x1c, "sbb    *,*  ", Mode::AccumulatorImmediateByte },
    {     0x1d, "sbb    *,*  ", Mode::AccumulatorImmediateWord },

    {     0x1e, "push   ds   ", Mode::Implied },
    {     0x1f, "pop    ds   ", Mode::Implied },

    {     0x20, "and    *,*  ", Mode::EffectiveByteRegister, Flag::Byte },
    {     0x21, "and    *,*  ", Mode::EffectiveWordRegister, Flag::Word },
    {     0x22, "and    *,*  ", Mode::RegisterEffectiveByte, Flag::Byte },
    {     0x23, "and    *,*  ", Mode::RegisterEffectiveWord, Flag::Word },
    {     0x24, "and    *,*  ", Mode::AccumulatorImmediateByte },
    {     0x25, "and    *,*  ", Mode::AccumulatorImmediateWord },

    {     0x26, "es          ", Mode::Implied, Flag::Prefix },
    {     0x27, "daa         ", Mode::Implied },

    {     0x28, "sub    *,*  ", Mode::EffectiveByteRegister, Flag::Byte },
    {     0x29, "sub    *,*  ", Mode::EffectiveWordRegister, Flag::Word },
    {     0x2a, "sub    *,*  ", Mode::RegisterEffectiveByte, Flag::Byte },
    {     0x2b, "sub    *,*  ", Mode::RegisterEffectiveWord, Flag::Word },
    {     0x2c, "sub    *,*  ", Mode::AccumulatorImmediateByte },
    {     0x2d, "sub    *,*  ", Mode::AccumulatorImmediateWord },

    {     0x2e, "cs          ", Mode::Implied, Flag::Prefix },
    {     0x2f, "das         ", Mode::Implied },

    {     0x30, "xor    *,*  ", Mode::EffectiveByteRegister, Flag::Byte },
    {     0x31, "xor    *,*  ", Mode::EffectiveWordRegister, Flag::Word },
    {     0x32, "xor    *,*  ", Mode::RegisterEffectiveByte, Flag::Byte },
    {     0x33, "xor    *,*  ", Mode::RegisterEffectiveWord, Flag::Word },
    {     0x34, "xor    *,*  ", Mode::AccumulatorImmediateByte },
    {     0x35, "xor    *,*  ", Mode::AccumulatorImmediateWord },

    {     0x36, "ss          ", Mode::Implied, Flag::Prefix },
    {     0x37, "aaa         ", Mode::Implied },

    {     0x38, "cmp    *,*  ", Mode::EffectiveByteRegister, Flag::Byte },
    {     0x39, "cmp    *,*  ", Mode::EffectiveWordRegister, Flag::Word },
    {     0x3a, "cmp    *,*  ", Mode::RegisterEffectiveByte, Flag::Byte },
    {     0x3b, "cmp    *,*  ", Mode::RegisterEffectiveWord, Flag::Word },
    {     0x3c, "cmp    *,*  ", Mode::AccumulatorImmediateByte },
    {     0x3d, "cmp    *,*  ", Mode::AccumulatorImmediateWord },

    {     0x3e, "ds          ", Mode::Implied, Flag::Prefix },
    {     0x3f, "aas         ", Mode::Implied },

    {     0x40, "inc    *    ", Mode::RegisterWord },
    {     0x48, "dec    *    ", Mode::RegisterWord },
    {     0x50, "push   *    ", Mode::RegisterWord },
    {     0x58, "pop    *    ", Mode::RegisterWord },

    {     0x60, "pushad      ", Mode::Implied },
    {     0x61, "popad       ", Mode::Implied },

    {     0x62, "bound  *,*  ", Mode::RegisterEffectiveWord, Flag::Word },
  //{     0x63, "arpl   *,*  ", Mode::EffectiveWordRegister, Flag::Word },

    {     0x64, "fs          ", Mode::Implied, Flag::Prefix },
    {     0x65, "gs          ", Mode::Implied, Flag::Prefix },

    {     0x68, "push   *    ", Mode::ImmediateWord },
    {     0x6a, "push   *    ", Mode::ImmediateByte },

    {     0x70, "jo     *    ", Mode::RelativeByte },
    {     0x71, "jno    *    ", Mode::RelativeByte },
    {     0x72, "jc     *    ", Mode::RelativeByte },
    {     0x73, "jnc    *    ", Mode::RelativeByte },
    {     0x74, "jz     *    ", Mode::RelativeByte },
    {     0x75, "jnz    *    ", Mode::RelativeByte },
    {     0x76, "jna    *    ", Mode::RelativeByte },
    {     0x77, "ja     *    ", Mode::RelativeByte },
    {     0x78, "js     *    ", Mode::RelativeByte },
    {     0x79, "jns    *    ", Mode::RelativeByte },
    {     0x7a, "jp     *    ", Mode::RelativeByte },
    {     0x7b, "jnp    *    ", Mode::RelativeByte },
    {     0x7c, "jl     *    ", Mode::RelativeByte },
    {     0x7d, "jnl    *    ", Mode::RelativeByte },
    {     0x7e, "jng    *    ", Mode::RelativeByte },
    {     0x7f, "jg     *    ", Mode::RelativeByte },

    {     0x80, "add    *,*  ", Mode::EffectiveByteImmediate, Flag::Byte, 0 },
    {     0x80, "or     *,*  ", Mode::EffectiveByteImmediate, Flag::Byte, 1 },
    {     0x80, "adc    *,*  ", Mode::EffectiveByteImmediate, Flag::Byte, 2 },
    {     0x80, "sbb    *,*  ", Mode::EffectiveByteImmediate, Flag::Byte, 3 },
    {     0x80, "and    *,*  ", Mode::EffectiveByteImmediate, Flag::Byte, 4 },
    {     0x80, "sub    *,*  ", Mode::EffectiveByteImmediate, Flag::Byte, 5 },
    {     0x80, "xor    *,*  ", Mode::EffectiveByteImmediate, Flag::Byte, 6 },
    {     0x80, "cmp    *,*  ", Mode::EffectiveByteImmediate, Flag::Byte, 7 },

    {     0x81, "add    *,*  ", Mode::EffectiveWordImmediate, Flag::Word, 0 },
    {     0x81, "or     *,*  ", Mode::EffectiveWordImmediate, Flag::Word, 1 },
    {     0x81, "adc    *,*  ", Mode::EffectiveWordImmediate, Flag::Word, 2 },
    {     0x81, "sbb    *,*  ", Mode::EffectiveWordImmediate, Flag::Word, 3 },
    {     0x81, "and    *,*  ", Mode::EffectiveWordImmediate, Flag::Word, 4 },
    {     0x81, "sub    *,*  ", Mode::EffectiveWordImmediate, Flag::Word, 5 },
    {     0x81, "xor    *,*  ", Mode::EffectiveWordImmediate, Flag::Word, 6 },
    {     0x81, "cmp    *,*  ", Mode::EffectiveWordImmediate, Flag::Word, 7 },

    {     0x83, "add    *,*  ", Mode::EffectiveWordImmediateByte, Flag::Word, 0 },
    {     0x83, "or     *,*  ", Mode::EffectiveWordImmediateByte, Flag::Word, 1 },
    {     0x83, "adc    *,*  ", Mode::EffectiveWordImmediateByte, Flag::Word, 2 },
    {     0x83, "sbb    *,*  ", Mode::EffectiveWordImmediateByte, Flag::Word, 3 },
    {     0x83, "and    *,*  ", Mode::EffectiveWordImmediateByte, Flag::Word, 4 },
    {     0x83, "sub    *,*  ", Mode::EffectiveWordImmediateByte, Flag::Word, 5 },
    {     0x83, "xor    *,*  ", Mode::EffectiveWordImmediateByte, Flag::Word, 6 },
    {     0x83, "cmp    *,*  ", Mode::EffectiveWordImmediateByte, Flag::Word, 7 },

    {     0x90, "nop         ", Mode::Implied },

    {     0x9c, "pushfd      ", Mode::Implied },
    {     0x9d, "popfd       ", Mode::Implied },
    {     0x9e, "sahf        ", Mode::Implied },
    {     0x9f, "lahf        ", Mode::Implied },

    {     0xe0, "loopnz *    ", Mode::RelativeByte },
    {     0xe9, "jmp    *    ", Mode::RelativeWord },
    {     0xeb, "jmp    *    ", Mode::RelativeByte },

    {     0xf0, "lock        ", Mode::Implied, Flag::Prefix },
    {     0xf2, "repnz       ", Mode::Implied, Flag::Prefix },
    {     0xf3, "repz        ", Mode::Implied, Flag::Prefix },

    {     0xf4, "hlt         ", Mode::Implied },
    {     0xf5, "cmc         ", Mode::Implied },

    {     0xf8, "clc         ", Mode::Implied },
    {     0xf9, "stc         ", Mode::Implied },
    {     0xfa, "cli         ", Mode::Implied },
    {     0xfb, "sti         ", Mode::Implied },
    {     0xfc, "cld         ", Mode::Implied },
    {     0xfd, "std         ", Mode::Implied },

    {     0xfe, "inc    *    ", Mode::EffectiveByte, Flag::Byte, 0 },
    {     0xfe, "dec    *    ", Mode::EffectiveByte, Flag::Byte, 1 },
    //2-7 = undefined

    {     0xff, "inc    *    ", Mode::EffectiveWord, Flag::Word, 0 },
    {     0xff, "dec    *    ", Mode::EffectiveWord, Flag::Word, 1 },
    {     0xff, "call   *    ", Mode::EffectiveWord, Flag::Word, 2 },
    //3 = callf
    {     0xff, "jmp    *    ", Mode::EffectiveWord, Flag::Word, 4 },
    //5 = jmpf
    {     0xff, "push   *    ", Mode::EffectiveWord, Flag::Word, 6 },
    //7 = undefined

    {   0x0f80, "jo     *    ", Mode::RelativeWord },
    {   0x0f81, "jno    *    ", Mode::RelativeWord },
    {   0x0f82, "jc     *    ", Mode::RelativeWord },
    {   0x0f83, "jnc    *    ", Mode::RelativeWord },
    {   0x0f84, "jz     *    ", Mode::RelativeWord },
    {   0x0f85, "jnz    *    ", Mode::RelativeWord },
    {   0x0f86, "jna    *    ", Mode::RelativeWord },
    {   0x0f87, "ja     *    ", Mode::RelativeWord },
    {   0x0f88, "js     *    ", Mode::RelativeWord },
    {   0x0f89, "jns    *    ", Mode::RelativeWord },
    {   0x0f8a, "jp     *    ", Mode::RelativeWord },
    {   0x0f8b, "jnp    *    ", Mode::RelativeWord },
    {   0x0f8c, "jl     *    ", Mode::RelativeWord },
    {   0x0f8d, "jnl    *    ", Mode::RelativeWord },
    {   0x0f8e, "jng    *    ", Mode::RelativeWord },
    {   0x0f8f, "jg     *    ", Mode::RelativeWord },
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

  //sort all opcodes by their order in Mode list
  foreach(y, table) {
    foreach(x, table) {
      if(&x != &y && (unsigned)x.mode > (unsigned)y.mode) swap(x, y);
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
