inline BassX86::Flag operator|(BassX86::Flag x, BassX86::Flag y) {
  return (BassX86::Flag)((unsigned)x | (unsigned)y);
}

inline BassX86::Flag operator&(BassX86::Flag x, BassX86::Flag y) {
  return (BassX86::Flag)((unsigned)x & (unsigned)y);
}

bool BassX86::assembleBlock(const string &block) {
  if(Bass::assembleBlock(block) == true) return true;

  lstring part, byte;
  part.split<1>(" ", block);
  string name = part[0], args = part[1];
  part.split<2>(",", args);

  signed addr;
  info.os0 = detectSize(part[0]);
  info.os1 = detectSize(part[1]);
  info.os2 = detectSize(part[2]);

  foreach(f, family) if(args.wildcard(f.pattern)) {
    foreach(o, f.opcode) if(name == o.name && isCorrectSize(o)) switch(o.mode) {
    case Mode::Implied:
      writePrefix(o);
      return true;

    case Mode::Register:
      if(auto p = verifyRegister(o.operand0, part[0])) {
        if(info.rp) write(0x66);
        write(o.prefix + p());
        return true;
      }
      continue;

    case Mode::AccumulatorImmediate:
      if(o.operand0 == Size::Byte && part[0] != "al") continue;
      if(o.operand0 == Size::Pair && part[0] != "ax" && part[0] != "eax") continue;
      if(isRegister(part[1])) continue;
      if(part[0] == "al") {
        writePrefix(o);
        write(eval(part[1]), 1);
      } else if(part[0] == "ax") {
        if(o.operand0 == Size::Pair) writeR(Size::Word);
        writePrefix(o);
        write(eval(part[1]), 2);
      } else if(part[0] == "eax") {
        if(o.operand0 == Size::Pair) writeR(Size::Long);
        writePrefix(o);
        write(eval(part[1]), 4);
      }
      return true;

    case Mode::Effective:
      if(effectiveAddress(o.operand0, part[0]) == false) continue;
      info.r = flagR(o);
      if(o.operand0 == Size::Pair) writeR(info.os0);
      writeEffectiveAddress(o);
      return true;

    case Mode::Relative:
      if(info.os0 ==  Size::Byte) {
        writePrefix(o);
        addr = eval(args) - (pc() + 1);
        if(pass == 2 && (addr < -128 || addr > +127)) error("branch out of bounds");
        write(addr, 1);
      } else if(info.os0 == Size::Word) {
        writeR(Size::Word);
        writePrefix(o);
        addr = eval(args) - (pc() + 2);
        if(pass == 2 && (addr < -32768 || addr > +32767)) error("branch out of bounds");
        write(addr, 2);
      } else {
        writeR(Size::Long);
        writePrefix(o);
        addr = eval(args) - (pc() + 4);
        write(addr, 4);
      }
      return true;

    case Mode::EffectiveRegister:
      if(effectiveAddress(o.operand0, part[0]) == false) continue;
      if(auto p = verifyRegister(o.operand1, part[1])) {
        info.r = p();
        writeEffectiveAddress(o);
        return true;
      }
      continue;

    case Mode::RegisterEffective:
      if(effectiveAddress(o.operand1, part[1]) == false) continue;
      if(auto p = verifyRegister(o.operand0, part[0])) {
        info.r = p();
        writeEffectiveAddress(o);
        return true;
      }
      continue;

    case Mode::Immediate:
      if(o.operand0 == Size::Pair) writeR(info.os0);
      writePrefix(o);
      write(eval(args), info.os0 == Size::Byte ? 1 : info.os0 == Size::Word ? 2 : 4);
      return true;

    case Mode::EffectiveImmediate:
      if(effectiveAddress(o.operand0, part[0]) == false) continue;
      info.r = flagR(o);
      if(o.operand0 == Size::Pair) writeR(info.os0);
      writeEffectiveAddress(o);
      if(o.operand1 == Size::ExactByte) {
        write(eval(part[1]), 1);
      } else {
        write(eval(part[1]), o.operand1 == Size::Byte ? 1 : info.os1 == Size::Word ? 2 : 4);
      }
      return true;

    case Mode::RegisterEffectiveImmediate:
      if(effectiveAddress(o.operand1, part[1]) == false) continue;
      if(auto p = verifyRegister(o.operand0, part[0])) {
        if(info.os2 == Size::Byte && o.operand2 != Size::ExactByte) continue;
        info.r = p();
        writeEffectiveAddress(o);
        write(eval(part[2]), info.os2 == Size::Byte ? 1 : info.os2 == Size::Word ? 2 : 4);
        return true;
      }
      continue;

    }
  }

  return false;
}

void BassX86::writeR(Size size) {
  if(cpu.bits == 16 && size == Size::Long) write(0x66);
  if(cpu.bits == 32 && size == Size::Word) write(0x66);
}

void BassX86::writeM(Size size) {
  if(cpu.bits == 16 && size == Size::Long) write(0x66);
  if(cpu.bits == 32 && size == Size::Word) write(0x66);
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

  if(s.wildcard("[*]")) return true;

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

optional<unsigned> BassX86::regs(const string &s) const {
  if(s == "es") return { 0, true };
  if(s == "cs") return { 1, true };
  if(s == "ss") return { 2, true };
  if(s == "ds") return { 3, true };
  if(s == "fs") return { 4, true };
  if(s == "gs") return { 5, true };
  if(s == "s6") return { 6, true };
  if(s == "s7") return { 7, true };
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

unsigned BassX86::flagR(const Opcode &o) {
  switch(o.flag & Flag::RM) {
  case Flag::R0: return 0;
  case Flag::R1: return 1;
  case Flag::R2: return 2;
  case Flag::R3: return 3;
  case Flag::R4: return 4;
  case Flag::R5: return 5;
  case Flag::R6: return 6;
  case Flag::R7: return 7;
  }
  throw;
}

bool BassX86::isCorrectSize(const Opcode &o) {
  bool os0 = true, os1 = true;

  switch(o.operand0) {
  case Size::Byte: os0 = (info.os0 == Size::None || info.os0 == Size::Byte); break;
  case Size::Word: os0 = (info.os0 == Size::None || info.os0 == Size::Word); break;
  case Size::Pair: os0 = (info.os0 == Size::None || info.os0 == Size::Word || info.os0 == Size::Long); break;
  case Size::Long: os0 = (info.os0 == Size::None || info.os0 == Size::Long); break;
  case Size::ExactByte: os0 = (info.os0 == Size::Byte); break;
  case Size::ExactWord: os0 = (info.os0 == Size::Word); break;
  case Size::ExactLong: os0 = (info.os0 == Size::Long); break;
  }

  switch(o.operand1) {
  case Size::Byte: os1 = (info.os1 == Size::None || info.os1 == Size::Byte); break;
  case Size::Word: os1 = (info.os1 == Size::None || info.os1 == Size::Word); break;
  case Size::Pair: os1 = (info.os1 == Size::None || info.os1 == Size::Word || info.os1 == Size::Long); break;
  case Size::Long: os1 = (info.os1 == Size::None || info.os1 == Size::Long); break;
  case Size::ExactByte: os1 = (info.os1 == Size::Byte); break;
  case Size::ExactWord: os1 = (info.os1 == Size::Word); break;
  case Size::ExactLong: os1 = (info.os1 == Size::Long); break;
  }

  return os0 && os1;
}

BassX86::Size BassX86::detectSize(string &s) {
  if(s.beginswith("<")) { s.ltrim<1>("<"); return Size::Byte; }
  if(s.beginswith(">")) { s.ltrim<1>(">"); return Size::Word; }
  if(s.beginswith("^")) { s.ltrim<1>("^"); return Size::Long; }
  return Size::None;
}

optional<unsigned> BassX86::verifyRegister(Size size, const string &s) {
  info.rp = false;
  if(size == Size::Byte || size == Size::ExactByte) return reg8(s);
  if(size == Size::Word || size == Size::ExactWord) return reg16(s);
  if(size == Size::Long || size == Size::ExactLong) return reg32(s);
  if(size == Size::Pair) {
    if(auto p = reg16(s)) {
      if(cpu.bits == 32) info.rp = true;
      return p;
    }
    if(auto p = reg32(s)) {
      if(cpu.bits == 16) info.rp = true;
      return p;
    }
  }
  return { false, 0 };
}

bool BassX86::effectiveAddress(Size os, const string &ea_) {
  string ea = ea_;
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
    //register
    if(auto p = verifyRegister(os, ea)) {
      info.mod = 3, info.m = p();
    } else return false;
  }

  return true;
}

bool BassX86::reg(const Opcode &o, const string &r) {
  if(o.operand0 == Size::Byte) {
    if(auto p = reg8(r)) {
      info.r = p();
    } else return false;
  } else if(o.operand0 == Size::Pair) {
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
  cpu.bits = 32;

  Opcode table[] = {
    {     0x00, "add    *,*  ", Mode::EffectiveRegister, Flag::None, Size::Byte, Size::Byte },
    {     0x01, "add    *,*  ", Mode::EffectiveRegister, Flag::None, Size::Pair, Size::Pair },
    {     0x02, "add    *,*  ", Mode::RegisterEffective, Flag::None, Size::Byte, Size::Byte },
    {     0x03, "add    *,*  ", Mode::RegisterEffective, Flag::None, Size::Pair, Size::Pair },
    {     0x04, "add    *,*  ", Mode::AccumulatorImmediate, Flag::None, Size::Byte, Size::Byte },
    {     0x05, "add    *,*  ", Mode::AccumulatorImmediate, Flag::None, Size::Pair, Size::Pair },

    {     0x06, "push   es   ", Mode::Implied },
    {     0x07, "pop    es   ", Mode::Implied },

    {     0x08, "or     *,*  ", Mode::EffectiveRegister, Flag::None, Size::Byte, Size::Byte },
    {     0x09, "or     *,*  ", Mode::EffectiveRegister, Flag::None, Size::Pair, Size::Pair },
    {     0x0a, "or     *,*  ", Mode::RegisterEffective, Flag::None, Size::Byte, Size::Byte },
    {     0x0b, "or     *,*  ", Mode::RegisterEffective, Flag::None, Size::Pair, Size::Pair },
    {     0x0c, "or     *,*  ", Mode::AccumulatorImmediate, Flag::None, Size::Byte, Size::Byte },
    {     0x0d, "or     *,*  ", Mode::AccumulatorImmediate, Flag::None, Size::Pair, Size::Pair },

    {     0x0e, "push   cs   ", Mode::Implied },
    {     0x0f, "pop    cs   ", Mode::Implied },  //8086 only

    {     0x10, "adc    *,*  ", Mode::EffectiveRegister, Flag::None, Size::Byte, Size::Byte },
    {     0x11, "adc    *,*  ", Mode::EffectiveRegister, Flag::None, Size::Pair, Size::Pair },
    {     0x12, "adc    *,*  ", Mode::RegisterEffective, Flag::None, Size::Byte, Size::Byte },
    {     0x13, "adc    *,*  ", Mode::RegisterEffective, Flag::None, Size::Pair, Size::Pair },
    {     0x14, "adc    *,*  ", Mode::AccumulatorImmediate, Flag::None, Size::Byte, Size::Byte },
    {     0x15, "adc    *,*  ", Mode::AccumulatorImmediate, Flag::None, Size::Pair, Size::Pair },

    {     0x16, "push   ss   ", Mode::Implied },
    {     0x17, "pop    ss   ", Mode::Implied },

    {     0x18, "sbb    *,*  ", Mode::EffectiveRegister, Flag::None, Size::Byte, Size::Byte },
    {     0x19, "sbb    *,*  ", Mode::EffectiveRegister, Flag::None, Size::Pair, Size::Pair },
    {     0x1a, "sbb    *,*  ", Mode::RegisterEffective, Flag::None, Size::Byte, Size::Byte },
    {     0x1b, "sbb    *,*  ", Mode::RegisterEffective, Flag::None, Size::Pair, Size::Pair },
    {     0x1c, "sbb    *,*  ", Mode::AccumulatorImmediate, Flag::None, Size::Byte, Size::Byte },
    {     0x1d, "sbb    *,*  ", Mode::AccumulatorImmediate, Flag::None, Size::Pair, Size::Pair },

    {     0x1e, "push   ds   ", Mode::Implied },
    {     0x1f, "pop    ds   ", Mode::Implied },

    {     0x20, "and    *,*  ", Mode::EffectiveRegister, Flag::None, Size::Byte, Size::Byte },
    {     0x21, "and    *,*  ", Mode::EffectiveRegister, Flag::None, Size::Pair, Size::Pair },
    {     0x22, "and    *,*  ", Mode::RegisterEffective, Flag::None, Size::Byte, Size::Byte },
    {     0x23, "and    *,*  ", Mode::RegisterEffective, Flag::None, Size::Pair, Size::Pair },
    {     0x24, "and    *,*  ", Mode::AccumulatorImmediate, Flag::None, Size::Byte, Size::Byte },
    {     0x25, "and    *,*  ", Mode::AccumulatorImmediate, Flag::None, Size::Pair, Size::Pair },

    {     0x26, "es          ", Mode::Implied, Flag::Prefix },
    {     0x27, "daa         ", Mode::Implied },

    {     0x28, "sub    *,*  ", Mode::EffectiveRegister, Flag::None, Size::Byte, Size::Byte },
    {     0x29, "sub    *,*  ", Mode::EffectiveRegister, Flag::None, Size::Pair, Size::Pair },
    {     0x2a, "sub    *,*  ", Mode::RegisterEffective, Flag::None, Size::Byte, Size::Byte },
    {     0x2b, "sub    *,*  ", Mode::RegisterEffective, Flag::None, Size::Pair, Size::Pair },
    {     0x2c, "sub    *,*  ", Mode::AccumulatorImmediate, Flag::None, Size::Byte, Size::Byte },
    {     0x2d, "sub    *,*  ", Mode::AccumulatorImmediate, Flag::None, Size::Pair, Size::Pair },

    {     0x2e, "cs          ", Mode::Implied, Flag::Prefix },
    {     0x2f, "das         ", Mode::Implied },

    {     0x30, "xor    *,*  ", Mode::EffectiveRegister, Flag::None, Size::Byte, Size::Byte },
    {     0x31, "xor    *,*  ", Mode::EffectiveRegister, Flag::None, Size::Pair, Size::Pair },
    {     0x32, "xor    *,*  ", Mode::RegisterEffective, Flag::None, Size::Byte, Size::Byte },
    {     0x33, "xor    *,*  ", Mode::RegisterEffective, Flag::None, Size::Pair, Size::Pair },
    {     0x34, "xor    *,*  ", Mode::AccumulatorImmediate, Flag::None, Size::Byte, Size::Byte },
    {     0x35, "xor    *,*  ", Mode::AccumulatorImmediate, Flag::None, Size::Pair, Size::Pair },

    {     0x36, "ss          ", Mode::Implied, Flag::Prefix },
    {     0x37, "aaa         ", Mode::Implied },

    {     0x38, "cmp    *,*  ", Mode::EffectiveRegister, Flag::None, Size::Byte, Size::Byte },
    {     0x39, "cmp    *,*  ", Mode::EffectiveRegister, Flag::None, Size::Pair, Size::Pair },
    {     0x3a, "cmp    *,*  ", Mode::RegisterEffective, Flag::None, Size::Byte, Size::Byte },
    {     0x3b, "cmp    *,*  ", Mode::RegisterEffective, Flag::None, Size::Pair, Size::Pair },
    {     0x3c, "cmp    *,*  ", Mode::AccumulatorImmediate, Flag::None, Size::Byte, Size::Byte },
    {     0x3d, "cmp    *,*  ", Mode::AccumulatorImmediate, Flag::None, Size::Pair, Size::Pair },

    {     0x3e, "ds          ", Mode::Implied, Flag::Prefix },
    {     0x3f, "aas         ", Mode::Implied },

    {     0x40, "inc    *    ", Mode::Register, Flag::None, Size::Pair },
    {     0x48, "dec    *    ", Mode::Register, Flag::None, Size::Pair },
    {     0x50, "push   *    ", Mode::Register, Flag::None, Size::Pair },
    {     0x58, "pop    *    ", Mode::Register, Flag::None, Size::Pair },

    {     0x60, "pushad      ", Mode::Implied },
    {   0x6660, "pushaw      ", Mode::Implied },
    {     0x61, "popad       ", Mode::Implied },
    {   0x6661, "popaw       ", Mode::Implied },

    {     0x62, "bound  *,*  ", Mode::RegisterEffective, Flag::None, Size::Pair, Size::Pair },
    {     0x63, "arpl   *,*  ", Mode::EffectiveRegister, Flag::None, Size::Word, Size::Word },

    {     0x64, "fs          ", Mode::Implied, Flag::Prefix },
    {     0x65, "gs          ", Mode::Implied, Flag::Prefix },

    {     0x68, "push   *    ", Mode::Immediate, Flag::None, Size::Pair },
    {     0x69, "imul   *,*,*", Mode::RegisterEffectiveImmediate, Flag::None, Size::Pair, Size::Pair, Size::Pair },
    {     0x6a, "push   *    ", Mode::Immediate, Flag::None, Size::ExactByte },
    {     0x6b, "imul   *,*,*", Mode::RegisterEffectiveImmediate, Flag::None, Size::Pair, Size::Pair, Size::ExactByte },
    {     0x6c, "insb        ", Mode::Implied },
    {   0x666d, "insw        ", Mode::Implied },
    {     0x6d, "insd        ", Mode::Implied },
    {     0x6e, "outsb       ", Mode::Implied },
    {   0x666f, "outsw       ", Mode::Implied },
    {     0x6f, "outsd       ", Mode::Implied },

    {     0x70, "jo     *    ", Mode::Relative, Flag::None, Size::ExactByte },
    {     0x71, "jno    *    ", Mode::Relative, Flag::None, Size::ExactByte },
    {     0x72, "jc     *    ", Mode::Relative, Flag::None, Size::ExactByte },
    {     0x73, "jnc    *    ", Mode::Relative, Flag::None, Size::ExactByte },
    {     0x74, "jz     *    ", Mode::Relative, Flag::None, Size::ExactByte },
    {     0x75, "jnz    *    ", Mode::Relative, Flag::None, Size::ExactByte },
    {     0x76, "jna    *    ", Mode::Relative, Flag::None, Size::ExactByte },
    {     0x77, "ja     *    ", Mode::Relative, Flag::None, Size::ExactByte },
    {     0x78, "js     *    ", Mode::Relative, Flag::None, Size::ExactByte },
    {     0x79, "jns    *    ", Mode::Relative, Flag::None, Size::ExactByte },
    {     0x7a, "jp     *    ", Mode::Relative, Flag::None, Size::ExactByte },
    {     0x7b, "jnp    *    ", Mode::Relative, Flag::None, Size::ExactByte },
    {     0x7c, "jl     *    ", Mode::Relative, Flag::None, Size::ExactByte },
    {     0x7d, "jnl    *    ", Mode::Relative, Flag::None, Size::ExactByte },
    {     0x7e, "jng    *    ", Mode::Relative, Flag::None, Size::ExactByte },
    {     0x7f, "jg     *    ", Mode::Relative, Flag::None, Size::ExactByte },

    {     0x80, "add    *,*  ", Mode::EffectiveImmediate, Flag::R0, Size::Byte, Size::Byte },
    {     0x80, "or     *,*  ", Mode::EffectiveImmediate, Flag::R1, Size::Byte, Size::Byte },
    {     0x80, "adc    *,*  ", Mode::EffectiveImmediate, Flag::R2, Size::Byte, Size::Byte },
    {     0x80, "sbb    *,*  ", Mode::EffectiveImmediate, Flag::R3, Size::Byte, Size::Byte },
    {     0x80, "and    *,*  ", Mode::EffectiveImmediate, Flag::R4, Size::Byte, Size::Byte },
    {     0x80, "sub    *,*  ", Mode::EffectiveImmediate, Flag::R5, Size::Byte, Size::Byte },
    {     0x80, "xor    *,*  ", Mode::EffectiveImmediate, Flag::R6, Size::Byte, Size::Byte },
    {     0x80, "cmp    *,*  ", Mode::EffectiveImmediate, Flag::R7, Size::Byte, Size::Byte },

    {     0x81, "add    *,*  ", Mode::EffectiveImmediate, Flag::R0, Size::Pair, Size::Pair },
    {     0x81, "or     *,*  ", Mode::EffectiveImmediate, Flag::R1, Size::Pair, Size::Pair },
    {     0x81, "adc    *,*  ", Mode::EffectiveImmediate, Flag::R2, Size::Pair, Size::Pair },
    {     0x81, "sbb    *,*  ", Mode::EffectiveImmediate, Flag::R3, Size::Pair, Size::Pair },
    {     0x81, "and    *,*  ", Mode::EffectiveImmediate, Flag::R4, Size::Pair, Size::Pair },
    {     0x81, "sub    *,*  ", Mode::EffectiveImmediate, Flag::R5, Size::Pair, Size::Pair },
    {     0x81, "xor    *,*  ", Mode::EffectiveImmediate, Flag::R6, Size::Pair, Size::Pair },
    {     0x81, "cmp    *,*  ", Mode::EffectiveImmediate, Flag::R7, Size::Pair, Size::Pair },

    //0x82 = mirror of 0x80?

    {     0x83, "add    *,*  ", Mode::EffectiveImmediate, Flag::R0, Size::Pair, Size::ExactByte },
    {     0x83, "or     *,*  ", Mode::EffectiveImmediate, Flag::R1, Size::Pair, Size::ExactByte },
    {     0x83, "adc    *,*  ", Mode::EffectiveImmediate, Flag::R2, Size::Pair, Size::ExactByte },
    {     0x83, "sbb    *,*  ", Mode::EffectiveImmediate, Flag::R3, Size::Pair, Size::ExactByte },
    {     0x83, "and    *,*  ", Mode::EffectiveImmediate, Flag::R4, Size::Pair, Size::ExactByte },
    {     0x83, "sub    *,*  ", Mode::EffectiveImmediate, Flag::R5, Size::Pair, Size::ExactByte },
    {     0x83, "xor    *,*  ", Mode::EffectiveImmediate, Flag::R6, Size::Pair, Size::ExactByte },
    {     0x83, "cmp    *,*  ", Mode::EffectiveImmediate, Flag::R7, Size::Pair, Size::ExactByte },

    {     0x84, "test   *,*  ", Mode::EffectiveRegister, Flag::None, Size::Byte, Size::Byte },
    {     0x85, "test   *,*  ", Mode::EffectiveRegister, Flag::None, Size::Pair, Size::Pair },
    {     0x86, "xchg   *,*  ", Mode::RegisterEffective, Flag::None, Size::Byte, Size::Byte },
    {     0x87, "xchg   *,*  ", Mode::RegisterEffective, Flag::None, Size::Pair, Size::Pair },
    {     0x88, "mov    *,*  ", Mode::EffectiveRegister, Flag::None, Size::Byte, Size::Byte },
    {     0x89, "mov    *,*  ", Mode::EffectiveRegister, Flag::None, Size::Pair, Size::Pair },
    {     0x8a, "mov    *,*  ", Mode::RegisterEffective, Flag::None, Size::Byte, Size::Byte },
    {     0x8b, "mov    *,*  ", Mode::RegisterEffective, Flag::None, Size::Pair, Size::Pair },
    //0x8c = mov effective,segment
    //0x8d = lea ???
    //0x8e = mov segment,effective
    {     0x8f, "pop    *    ", Mode::Effective, Flag::None, Size::Pair },

    {     0x90, "nop         ", Mode::Implied },
    {   0xf390, "pause       ", Mode::Implied },

    {     0x98, "cwde        ", Mode::Implied },
    {   0x6698, "cbw         ", Mode::Implied },
    {     0x99, "cdq         ", Mode::Implied },
    {   0x6699, "cwd         ", Mode::Implied },

    {     0x9c, "pushfd      ", Mode::Implied },
    {     0x9d, "popfd       ", Mode::Implied },
    {     0x9e, "sahf        ", Mode::Implied },
    {     0x9f, "lahf        ", Mode::Implied },

    {     0xe0, "loopnz *    ", Mode::Relative, Flag::None, Size::ExactByte },
    {     0xe9, "jmp    *    ", Mode::Relative, Flag::None, Size::Pair },
    {     0xeb, "jmp    *    ", Mode::Relative, Flag::None, Size::ExactByte },

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

    {     0xfe, "inc    *    ", Mode::Effective, Flag::R0, Size::ExactByte },
    {     0xfe, "dec    *    ", Mode::Effective, Flag::R1, Size::ExactByte },
    //Flag::R2-7 = undefined

    {     0xff, "inc    *    ", Mode::Effective, Flag::R0, Size::Pair },
    {     0xff, "dec    *    ", Mode::Effective, Flag::R1, Size::Pair },
    {     0xff, "call   *    ", Mode::Effective, Flag::R2, Size::Pair },
    //Flag::R3 = callf
    {     0xff, "jmp    *    ", Mode::Effective, Flag::R4, Size::Pair },
    //Flag::R5 = jmpf
    {     0xff, "push   *    ", Mode::Effective, Flag::R6, Size::Pair },
    //Flag::R7 = undefined

    {   0x0f80, "jo     *    ", Mode::Relative, Flag::None, Size::Pair },
    {   0x0f81, "jno    *    ", Mode::Relative, Flag::None, Size::Pair },
    {   0x0f82, "jc     *    ", Mode::Relative, Flag::None, Size::Pair },
    {   0x0f83, "jnc    *    ", Mode::Relative, Flag::None, Size::Pair },
    {   0x0f84, "jz     *    ", Mode::Relative, Flag::None, Size::Pair },
    {   0x0f85, "jnz    *    ", Mode::Relative, Flag::None, Size::Pair },
    {   0x0f86, "jna    *    ", Mode::Relative, Flag::None, Size::Pair },
    {   0x0f87, "ja     *    ", Mode::Relative, Flag::None, Size::Pair },
    {   0x0f88, "js     *    ", Mode::Relative, Flag::None, Size::Pair },
    {   0x0f89, "jns    *    ", Mode::Relative, Flag::None, Size::Pair },
    {   0x0f8a, "jp     *    ", Mode::Relative, Flag::None, Size::Pair },
    {   0x0f8b, "jnp    *    ", Mode::Relative, Flag::None, Size::Pair },
    {   0x0f8c, "jl     *    ", Mode::Relative, Flag::None, Size::Pair },
    {   0x0f8d, "jnl    *    ", Mode::Relative, Flag::None, Size::Pair },
    {   0x0f8e, "jng    *    ", Mode::Relative, Flag::None, Size::Pair },
    {   0x0f8f, "jg     *    ", Mode::Relative, Flag::None, Size::Pair },
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
