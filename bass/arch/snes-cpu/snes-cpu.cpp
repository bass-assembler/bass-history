int64_t BassSnesCpu::eval(const string &s) {
  if(s[0] == '<') return Bass::eval((const char*)s + 1);
  if(s[0] == '>') return Bass::eval((const char*)s + 1);
  if(s[0] == '^') return Bass::eval((const char*)s + 1);
  return Bass::eval(s);
}

void BassSnesCpu::seek(unsigned offset) {
  switch(mapper) {
  case Mapper::LoROM: offset = ((offset & 0x7f0000) >> 1) | (offset & 0x7fff); break;
  case Mapper::HiROM: offset = ((offset & 0x3fffff)); break;
  }
  Bass::seek(offset);
}

bool BassSnesCpu::assembleBlock(const string &block_) {
  string block = block_;
  if(Bass::assembleBlock(block) == true) return true;

  signed relative, repeat;
  bool priority = false;

  lstring part;
  part.split<1>(" ", block);
  string name = part[0], param = part[1];

  if(name == "mapper") {
    if(param == "none ") { mapper = Mapper::None;  return true; }
    if(param == "lorom") { mapper = Mapper::LoROM; return true; }
    if(param == "hirom") { mapper = Mapper::HiROM; return true; }
    error("invalid mapper ID");
  }

  static auto isbyte = [&]() {
    if(param.wildcard("$????") || param.wildcard("$??????")) return false;
    if(!priority && param[0] != '<' && !param.wildcard("$??")) return false;
    return true;
  };

  static auto isword = [&]() {
    if(param.wildcard("$??") || param.wildcard("$??????")) return false;
    if(!priority && param[0] != '>' && !param.wildcard("$????")) return false;
    return true;
  };

  static auto islong = [&]() {
    if(param.wildcard("$??") || param.wildcard("$????")) return false;
    if(!priority && param[0] != '^' && !param.wildcard("$??????")) return false;
    return true;
  };

  foreach(f, family) if(param.wildcard(f.pattern)) {
    foreach(o, f.opcode) if(name == o.name) {
      priority = o.priority;
      switch(o.mode) {
      case Mode::Implied:
        write(o.prefix);
        return true;
      case Mode::ImpliedRepeat:
        param.ltrim<1>("#");
        repeat = eval(param);
        while(repeat--) write(o.prefix);
        return true;
      case Mode::Immediate:
        param.ltrim<1>("#");
        write(o.prefix);
        write(eval(param));
        return true;
      case Mode::ImmediateM:
      case Mode::ImmediateX:
        param.ltrim<1>("#");
        write(o.prefix);
        write(eval(param), isword() ? 2 : 1);
        return true;
      case Mode::Direct:
        if(isbyte() == false) break;
        write(o.prefix);
        write(eval(param));
        return true;
      case Mode::DirectX:
        param.rtrim<1>("+x");
        if(isbyte() == false) break;
        write(o.prefix);
        write(eval(param));
        return true;
      case Mode::DirectY:
        param.rtrim<1>("+y");
        if(isbyte() == false) break;
        write(o.prefix);
        write(eval(param));
        return true;
      case Mode::DirectS:
        param.rtrim<1>("+s");
        write(o.prefix);
        write(eval(param));
        return true;
      case Mode::Indirect:
        param.ltrim<1>("(");
        param.rtrim<1>(")");
        write(o.prefix);
        write(eval(param));
        return true;
      case Mode::IndirectX:
        param.ltrim<1>("(");
        param.rtrim<1>("+x)");
        write(o.prefix);
        write(eval(param));
        return true;
      case Mode::IndirectY:
        param.ltrim<1>("(");
        param.rtrim<1>(")+y");
        write(o.prefix);
        write(eval(param));
        return true;
      case Mode::IndirectSY:
        param.ltrim<1>("(");
        param.rtrim<1>("+s)+y");
        write(o.prefix);
        write(eval(param));
        return true;
      case Mode::IndirectLong:
        param.ltrim<1>("[");
        param.rtrim<1>("]");
        write(o.prefix);
        write(eval(param));
        return true;
      case Mode::IndirectLongY:
        param.ltrim<1>("[");
        param.rtrim<1>("]+y");
        write(o.prefix);
        write(eval(param));
        return true;
      case Mode::Absolute:
        if(isword() == false) break;
        write(o.prefix);
        write(eval(param), 2);
        return true;
      case Mode::AbsoluteX:
        param.rtrim<1>("+x");
        if(isword() == false) break;
        write(o.prefix);
        write(eval(param), 2);
        return true;
      case Mode::AbsoluteY:
        param.rtrim<1>("+y");
        if(isword() == false) break;
        write(o.prefix);
        write(eval(param), 2);
        return true;
      case Mode::IndirectAbsolute:
        param.ltrim<1>("(");
        param.rtrim<1>(")");
        write(o.prefix);
        write(eval(param), 2);
        return true;
      case Mode::IndirectAbsoluteX:
        param.ltrim<1>("(");
        param.rtrim<1>("+x)");
        write(o.prefix);
        write(eval(param), 2);
        return true;
      case Mode::IndirectLongAbsolute:
        param.ltrim<1>("[");
        param.rtrim<1>("]");
        write(o.prefix);
        write(eval(param), 2);
        return true;
      case Mode::Long:
        if(islong() == false) break;
        write(o.prefix);
        write(eval(param), 3);
        return true;
      case Mode::LongX:
        param.rtrim<1>("+x");
        if(islong() == false) break;
        write(o.prefix);
        write(eval(param), 3);
        return true;
      case Mode::Relative:
        if(param.wildcard("$??")) {
          relative = eval(param);
        } else {
          relative = eval(param) - (pc() + 2);
          if(pass == 2 && (relative < -128 || relative > +127)) error("branch out of bounds");
        }
        write(o.prefix);
        write(relative);
        return true;
      case Mode::RelativeLong:
        if(param.wildcard("$????")) {
          relative = eval(param);
        } else {
          relative = eval(param) - (pc() + 3);
          if(pass == 2 && (relative < -32768 || relative > +32767)) error("branch out of bounds");
        }
        write(o.prefix);
        write(relative, 2);
        return true;
      case Mode::BlockMove:
        part.split<1>(",", param);
        write(o.prefix);
        write(eval(part[1]));
        write(eval(part[0]));
        return true;
      }
    }
  }

  return false;
}

BassSnesCpu::BassSnesCpu() {
  linear_vector<Opcode> table = {
    { 0x00, "brk #*       ", 1, Mode::Immediate },
    { 0x01, "ora (*+x)    ", 1, Mode::IndirectX },
    { 0x02, "cop #*       ", 1, Mode::Immediate },
    { 0x03, "ora *+s      ", 1, Mode::DirectS },
    { 0x04, "tsb *        ", 0, Mode::Direct },
    { 0x05, "ora *        ", 0, Mode::Direct },
    { 0x06, "asl *        ", 0, Mode::Direct },
    { 0x07, "ora [*]      ", 1, Mode::IndirectLong },
    { 0x08, "php          ", 1, Mode::Implied },
    { 0x09, "ora #*       ", 0, Mode::ImmediateM },
    { 0x0a, "asl          ", 1, Mode::Implied },
    { 0x0b, "phd          ", 1, Mode::Implied },
    { 0x0c, "tsb *        ", 1, Mode::Absolute },
    { 0x0d, "ora *        ", 0, Mode::Absolute },
    { 0x0e, "asl *        ", 1, Mode::Absolute },
    { 0x0f, "ora *        ", 1, Mode::Long },

    { 0x10, "bpl *        ", 1, Mode::Relative },
    { 0x11, "ora (*)+y    ", 1, Mode::IndirectY },
    { 0x12, "ora (*)      ", 1, Mode::Indirect },
    { 0x13, "ora (*+s)+y  ", 1, Mode::IndirectSY },
    { 0x14, "trb *        ", 0, Mode::Direct },
    { 0x15, "ora *+x      ", 0, Mode::DirectX },
    { 0x16, "asl *+x      ", 0, Mode::DirectX },
    { 0x17, "ora [*]+y    ", 1, Mode::IndirectLongY },
    { 0x18, "clc          ", 1, Mode::Implied },
    { 0x19, "ora *+y      ", 1, Mode::AbsoluteY },
    { 0x1a, "inc          ", 1, Mode::Implied },
    { 0x1b, "tcs          ", 1, Mode::Implied },
    { 0x1c, "trb *        ", 1, Mode::Absolute },
    { 0x1d, "ora *+x      ", 0, Mode::AbsoluteX },
    { 0x1e, "asl *+x      ", 1, Mode::AbsoluteX },
    { 0x1f, "ora *+x      ", 1, Mode::LongX },

    { 0x20, "jsr *        ", 1, Mode::Absolute },
    { 0x21, "and (*+x)    ", 1, Mode::IndirectX },
    { 0x22, "jsl *        ", 1, Mode::Long },
    { 0x23, "and *+s      ", 1, Mode::DirectS },
    { 0x24, "bit *        ", 0, Mode::Direct },
    { 0x25, "and *        ", 0, Mode::Direct },
    { 0x26, "rol *        ", 0, Mode::Direct },
    { 0x27, "and [*]      ", 1, Mode::IndirectLong },
    { 0x28, "plp          ", 1, Mode::Implied },
    { 0x29, "and #*       ", 0, Mode::ImmediateM },
    { 0x2a, "rol          ", 1, Mode::Implied },
    { 0x2b, "pld          ", 1, Mode::Implied },
    { 0x2c, "bit *        ", 1, Mode::Absolute },
    { 0x2d, "and *        ", 0, Mode::Absolute },
    { 0x2e, "rol *        ", 1, Mode::Absolute },
    { 0x2f, "and *        ", 1, Mode::Long },

    { 0x30, "bmi *        ", 1, Mode::Relative },
    { 0x31, "and (*)+y    ", 1, Mode::IndirectY },
    { 0x32, "and (*)      ", 1, Mode::Indirect, },
    { 0x33, "and (*+s)+y  ", 1, Mode::IndirectSY },
    { 0x34, "bit *+x      ", 0, Mode::DirectX },
    { 0x35, "and *+x      ", 0, Mode::DirectX },
    { 0x36, "rol *+x      ", 0, Mode::DirectX },
    { 0x37, "and [*]+y    ", 1, Mode::IndirectLongY },
    { 0x38, "sec          ", 1, Mode::Implied },
    { 0x39, "and *+y      ", 1, Mode::AbsoluteY },
    { 0x3a, "dec          ", 1, Mode::Implied },
    { 0x3b, "tsc          ", 1, Mode::Implied },
    { 0x3c, "bit *+x      ", 1, Mode::AbsoluteX },
    { 0x3d, "and *+x      ", 0, Mode::AbsoluteX },
    { 0x3e, "rol *+x      ", 1, Mode::AbsoluteX },
    { 0x3f, "and *+x      ", 1, Mode::LongX },

    { 0x40, "rti          ", 1, Mode::Implied },
    { 0x41, "eor (*+x)    ", 1, Mode::IndirectX },
    { 0x42, "wdm #*       ", 1, Mode::Immediate },
    { 0x43, "eor *+s      ", 1, Mode::DirectS },
    { 0x44, "mvp *,*      ", 1, Mode::BlockMove },
    { 0x45, "eor *        ", 0, Mode::Direct },
    { 0x46, "lsr *        ", 0, Mode::Direct },
    { 0x47, "eor [*]      ", 1, Mode::IndirectLong },
    { 0x48, "pha          ", 1, Mode::Implied },
    { 0x49, "eor #*       ", 0, Mode::ImmediateM },
    { 0x4a, "lsr          ", 1, Mode::Implied },
    { 0x4b, "phk          ", 1, Mode::Implied },
    { 0x4c, "jmp *        ", 1, Mode::Absolute },
    { 0x4d, "eor *        ", 0, Mode::Absolute },
    { 0x4e, "lsr *        ", 1, Mode::Absolute },
    { 0x4f, "eor *        ", 1, Mode::Long },

    { 0x50, "bvc *        ", 1, Mode::Relative },
    { 0x51, "eor (*)+y    ", 1, Mode::IndirectY },
    { 0x52, "eor (*)      ", 1, Mode::Indirect },
    { 0x53, "eor (*+s)+y  ", 1, Mode::IndirectSY },
    { 0x54, "mvn *,*      ", 1, Mode::BlockMove },
    { 0x55, "eor *+x      ", 0, Mode::DirectX },
    { 0x56, "lsr *+x      ", 0, Mode::DirectX },
    { 0x57, "eor [*]+y    ", 1, Mode::IndirectLongY },
    { 0x58, "cli          ", 1, Mode::Implied },
    { 0x59, "eor *+y      ", 1, Mode::AbsoluteY },
    { 0x5a, "phy          ", 1, Mode::Implied },
    { 0x5b, "tcd          ", 1, Mode::Implied },
    { 0x5c, "jml *        ", 1, Mode::Long },
    { 0x5d, "eor *+x      ", 0, Mode::AbsoluteX },
    { 0x5e, "lsr *+x      ", 1, Mode::AbsoluteX },
    { 0x5f, "eor *+x      ", 1, Mode::LongX },

    { 0x60, "rts          ", 1, Mode::Implied },
    { 0x61, "adc (*+x)    ", 1, Mode::IndirectX },
    { 0x62, "per *        ", 1, Mode::Absolute },
    { 0x63, "adc *+s      ", 1, Mode::DirectS },
    { 0x64, "stz *        ", 0, Mode::Direct },
    { 0x65, "adc *        ", 0, Mode::Direct },
    { 0x66, "ror *        ", 0, Mode::Direct },
    { 0x67, "adc [*]      ", 1, Mode::IndirectLong },
    { 0x68, "pla          ", 1, Mode::Implied },
    { 0x69, "adc #*       ", 0, Mode::ImmediateM },
    { 0x6a, "ror          ", 1, Mode::Implied },
    { 0x6b, "rtl          ", 1, Mode::Implied },
    { 0x6c, "jmp (*)      ", 1, Mode::IndirectAbsolute },
    { 0x6d, "adc *        ", 0, Mode::Absolute },
    { 0x6e, "ror *        ", 1, Mode::Absolute },
    { 0x6f, "adc *        ", 1, Mode::Long },

    { 0x70, "bvs *        ", 1, Mode::Relative },
    { 0x71, "adc (*)+y    ", 1, Mode::IndirectY },
    { 0x72, "adc (*)      ", 1, Mode::Indirect },
    { 0x73, "adc (*+s)+y  ", 1, Mode::IndirectSY },
    { 0x74, "stz *+x      ", 0, Mode::DirectX },
    { 0x75, "adc *+x      ", 0, Mode::DirectX },
    { 0x76, "ror *+x      ", 0, Mode::DirectX },
    { 0x77, "adc [*]+y    ", 1, Mode::IndirectLongY },
    { 0x78, "sei          ", 1, Mode::Implied },
    { 0x79, "adc *+y      ", 1, Mode::AbsoluteY },
    { 0x7a, "ply          ", 1, Mode::Implied },
    { 0x7b, "tdc          ", 1, Mode::Implied },
    { 0x7c, "jmp (*+x)    ", 1, Mode::IndirectAbsoluteX },
    { 0x7d, "adc *+x      ", 0, Mode::AbsoluteX },
    { 0x7e, "ror *+x      ", 1, Mode::AbsoluteX },
    { 0x7f, "adc *+x      ", 1, Mode::LongX },

    { 0x80, "bra *        ", 1, Mode::Relative },
    { 0x81, "sta (*+x)    ", 1, Mode::IndirectX },
    { 0x82, "brl *        ", 1, Mode::RelativeLong },
    { 0x83, "sta *+s      ", 1, Mode::DirectS },
    { 0x84, "sty *        ", 0, Mode::Direct },
    { 0x85, "sta *        ", 0, Mode::Direct },
    { 0x86, "stx *        ", 0, Mode::Direct },
    { 0x87, "sta [*]      ", 1, Mode::IndirectLong },
    { 0x88, "dey          ", 1, Mode::Implied },
    { 0x89, "bit #*       ", 0, Mode::ImmediateM },
    { 0x8a, "txa          ", 1, Mode::Implied },
    { 0x8b, "phb          ", 1, Mode::Implied },
    { 0x8c, "sty *        ", 1, Mode::Absolute },
    { 0x8d, "sta *        ", 0, Mode::Absolute },
    { 0x8e, "stx *        ", 1, Mode::Absolute },
    { 0x8f, "sta *        ", 1, Mode::Long },

    { 0x90, "bcc *        ", 1, Mode::Relative },
    { 0x91, "sta (*)+y    ", 1, Mode::IndirectY },
    { 0x92, "sta (*)      ", 1, Mode::Indirect },
    { 0x93, "sta (*+s)+y  ", 1, Mode::IndirectSY },
    { 0x94, "sty *+x      ", 1, Mode::DirectX },
    { 0x95, "sta *+x      ", 0, Mode::DirectX },
    { 0x96, "stx *+y      ", 1, Mode::DirectY },
    { 0x97, "sta [*]+y    ", 1, Mode::IndirectLongY },
    { 0x98, "tya          ", 1, Mode::Implied },
    { 0x99, "sta *+y      ", 1, Mode::AbsoluteY },
    { 0x9a, "txs          ", 1, Mode::Implied },
    { 0x9b, "txy          ", 1, Mode::Implied },
    { 0x9c, "stz *        ", 1, Mode::Absolute },
    { 0x9d, "sta *+x      ", 0, Mode::AbsoluteX },
    { 0x9e, "stz *+x      ", 1, Mode::AbsoluteX },
    { 0x9f, "sta *+x      ", 1, Mode::LongX },

    { 0xa0, "ldy #*       ", 0, Mode::ImmediateX },
    { 0xa1, "lda (*+x)    ", 1, Mode::IndirectX },
    { 0xa2, "ldx #*       ", 0, Mode::ImmediateX },
    { 0xa3, "lda *+s      ", 1, Mode::DirectS },
    { 0xa4, "ldy *        ", 0, Mode::Direct },
    { 0xa5, "lda *        ", 0, Mode::Direct },
    { 0xa6, "ldx *        ", 0, Mode::Direct },
    { 0xa7, "lda [*]      ", 1, Mode::IndirectLong },
    { 0xa8, "tay          ", 1, Mode::Implied },
    { 0xa9, "lda #*       ", 0, Mode::ImmediateM },
    { 0xaa, "tax          ", 1, Mode::Implied },
    { 0xab, "plb          ", 1, Mode::Implied },
    { 0xac, "ldy *        ", 1, Mode::Absolute },
    { 0xad, "lda *        ", 0, Mode::Absolute },
    { 0xae, "ldx *        ", 1, Mode::Absolute },
    { 0xaf, "lda *        ", 1, Mode::Long },

    { 0xb0, "bcs *        ", 1, Mode::Relative },
    { 0xb1, "lda (*)+y    ", 1, Mode::IndirectY },
    { 0xb2, "lda (*)      ", 1, Mode::Indirect },
    { 0xb3, "lda (*+s)+y  ", 1, Mode::IndirectSY },
    { 0xb4, "ldy *+x      ", 0, Mode::DirectX },
    { 0xb5, "lda *+x      ", 0, Mode::DirectX },
    { 0xb6, "ldx *+y      ", 0, Mode::DirectY },
    { 0xb7, "lda [*]+y    ", 1, Mode::IndirectLongY },
    { 0xb8, "clv          ", 1, Mode::Implied },
    { 0xb9, "lda *+y      ", 1, Mode::AbsoluteY },
    { 0xba, "tsx          ", 1, Mode::Implied },
    { 0xbb, "tyx          ", 1, Mode::Implied },
    { 0xbc, "ldy *+x      ", 1, Mode::AbsoluteX },
    { 0xbd, "lda *+x      ", 0, Mode::AbsoluteX },
    { 0xbe, "ldx *+y      ", 1, Mode::AbsoluteY },
    { 0xbf, "lda *+x      ", 1, Mode::LongX },

    { 0xc0, "cpy #*       ", 0, Mode::ImmediateX },
    { 0xc1, "cmp (*+x)    ", 1, Mode::IndirectX },
    { 0xc2, "rep #*       ", 1, Mode::Immediate },
    { 0xc3, "cmp *+s      ", 1, Mode::DirectS },
    { 0xc4, "cpy *        ", 0, Mode::Direct },
    { 0xc5, "cmp *        ", 0, Mode::Direct },
    { 0xc6, "dec *        ", 0, Mode::Direct },
    { 0xc7, "cmp [*]      ", 1, Mode::IndirectLong },
    { 0xc8, "iny          ", 1, Mode::Implied },
    { 0xc9, "cmp #*       ", 0, Mode::ImmediateM },
    { 0xca, "dex          ", 1, Mode::Implied },
    { 0xcb, "wai          ", 1, Mode::Implied },
    { 0xcc, "cpy *        ", 1, Mode::Absolute },
    { 0xcd, "cmp *        ", 0, Mode::Absolute },
    { 0xce, "dec *        ", 1, Mode::Absolute },
    { 0xcf, "cmp *        ", 1, Mode::Long },

    { 0xd0, "bne *        ", 1, Mode::Relative },
    { 0xd1, "cmp (*)+y    ", 1, Mode::IndirectY },
    { 0xd2, "cmp (*)      ", 1, Mode::Indirect },
    { 0xd3, "cmp (*+s)+y  ", 1, Mode::IndirectSY },
    { 0xd4, "pei (*)      ", 1, Mode::Indirect },
    { 0xd5, "cmp *+x      ", 0, Mode::DirectX },
    { 0xd6, "dec *+x      ", 0, Mode::DirectX },
    { 0xd7, "cmp [*]+y    ", 1, Mode::IndirectLongY },
    { 0xd8, "cld          ", 1, Mode::Implied },
    { 0xd9, "cmp *+y      ", 1, Mode::AbsoluteY },
    { 0xda, "phx          ", 1, Mode::Implied },
    { 0xdb, "stp          ", 1, Mode::Implied },
    { 0xdc, "jmp [*]      ", 1, Mode::IndirectLongAbsolute },
    { 0xdd, "cmp *+x      ", 0, Mode::AbsoluteX },
    { 0xde, "dec *+x      ", 1, Mode::AbsoluteX },
    { 0xdf, "cmp *+x      ", 1, Mode::LongX },

    { 0xe0, "cpx #*       ", 0, Mode::ImmediateX },
    { 0xe1, "sbc (*+x)    ", 1, Mode::IndirectX },
    { 0xe2, "sep #*       ", 1, Mode::Immediate },
    { 0xe3, "sbc *+s      ", 1, Mode::DirectS },
    { 0xe4, "cpx *        ", 0, Mode::Direct },
    { 0xe5, "sbc *        ", 0, Mode::Direct },
    { 0xe6, "inc *        ", 0, Mode::Direct },
    { 0xe7, "sbc [*]      ", 1, Mode::IndirectLong },
    { 0xe8, "inx          ", 1, Mode::Implied },
    { 0xe9, "sbc #*       ", 0, Mode::ImmediateM },
    { 0xea, "nop          ", 1, Mode::Implied },
    { 0xeb, "xba          ", 1, Mode::Implied },
    { 0xec, "cpx *        ", 1, Mode::Absolute },
    { 0xed, "sbc *        ", 0, Mode::Absolute },
    { 0xee, "inc *        ", 1, Mode::Absolute },
    { 0xef, "sbc *        ", 1, Mode::Long },

    { 0xf0, "beq *        ", 1, Mode::Relative },
    { 0xf1, "sbc (*)+y    ", 1, Mode::IndirectY },
    { 0xf2, "sbc (*)      ", 1, Mode::Indirect },
    { 0xf3, "sbc (*+s)+y  ", 1, Mode::IndirectSY },
    { 0xf4, "pea *        ", 1, Mode::Absolute },
    { 0xf5, "sbc *+x      ", 0, Mode::DirectX },
    { 0xf6, "inc *+x      ", 0, Mode::DirectX },
    { 0xf7, "sbc [*]+y    ", 1, Mode::IndirectLongY },
    { 0xf8, "sed          ", 1, Mode::Implied },
    { 0xf9, "sbc *+y      ", 1, Mode::AbsoluteY },
    { 0xfa, "plx          ", 1, Mode::Implied },
    { 0xfb, "xce          ", 1, Mode::Implied },
    { 0xfc, "jsr (*+x)    ", 1, Mode::IndirectAbsoluteX },
    { 0xfd, "sbc *+x      ", 0, Mode::AbsoluteX },
    { 0xfe, "inc *+x      ", 1, Mode::AbsoluteX },
    { 0xff, "sbc *+x      ", 1, Mode::LongX },
  };

  unsigned size = table.size();
  for(unsigned n = 0; n < size; n++) {
    if(table[n].mode == Mode::Implied) {
      Opcode opcode = table[n];
      opcode.mnemonic.rtrim(" ");
      opcode.mnemonic = { opcode.mnemonic, " #*" };
      opcode.mode = Mode::ImpliedRepeat;
      table.append(opcode);
    }
  }

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
