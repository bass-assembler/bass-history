void BassTable::initialize(unsigned pass) {
  bitval = 0;
  bitpos = 0;
}

bool BassTable::assembleBlock(const string &block) {
  if(Bass::assembleBlock(block) == true) return true;

  if(block.beginswith("arch ")) {
    string filename = substr(block, 5).trim<1>("\""), data;
    if(data.readfile(filename) == false) error({ "arch: file \"", filename, "\" not found" });
    table.reset();
    parseTable(data);
    return true;
  }

  unsigned pc = this->pc(), data;

  for(auto &opcode : table) {
    if(tokenize(block, opcode.pattern) == false) continue;

    lstring args;
    tokenize(args, block, opcode.pattern);
    if(args.size() != opcode.number.size()) continue;

    bool mismatch = false;
    for(unsigned n = 0; n < opcode.number.size(); n++) {
      unsigned bits = bitLength(args[n]);
      if(bits && bits != opcode.number[n].bits) { mismatch = true; break; }
    }
    if(mismatch) continue;

    for(auto &format : opcode.format) {
      switch(format.type) {
      case Format::Type::Static:
        writeBits(format.data, 8);
        break;
      case Format::Type::Absolute:
        data = eval(args[format.data]);
        writeBits(data, opcode.number[format.data].bits);
        break;
      case Format::Type::Relative:
        data = eval(args[format.data]) - (pc + format.displacement);
        writeBits(data, opcode.number[format.data].bits);
        break;
      case Format::Type::Repeat:
        data = eval(args[format.data]);
        for(unsigned n = 0; n < data - 1; n++) writeBits(format.displacement, opcode.number[format.data].bits);
        break;
      }
    }

    return true;
  }

  return false;
}

unsigned BassTable::bitLength(string &text) {
  auto hexLength = [&](const char *p) -> unsigned {
    unsigned length = 0;
    while(*p) {
      if(*p >= '0' && *p <= '9') { p++; length += 4; continue; }
      if(*p >= 'a' && *p <= 'f') { p++; length += 4; continue; }
      return 0;
    }
    return length;
  };

  auto binLength = [&](const char *p) -> unsigned {
    unsigned length = 0;
    while(*p) {
      if(*p == '0' || *p == '1') { p++; length++; continue; }
      return 0;
    }
    return length;
  };

  if(text[0] == '<') { text[0] = ' '; return  8; }
  if(text[0] == '>') { text[0] = ' '; return 16; }
  if(text[0] == '^') { text[0] = ' '; return 24; }
  if(text[0] == '?') { text[0] = ' '; return 32; }
  if(text[0] == '!') { text[0] = ' '; return 64; }
  if(text[0] == '%') return binLength((const char*)text + 1);
  if(text[0] == '$') return hexLength((const char*)text + 1);
  if(text[0] == '0' && text[1] == 'b') return binLength((const char*)text + 2);
  if(text[0] == '0' && text[1] == 'x') return hexLength((const char*)text + 2);
  return 0;
}

void BassTable::writeBits(uint64_t data, unsigned length) {
  bitval <<= length;
  bitval |= data;
  bitpos += length;

  while(bitpos >= 8) {
    write(bitval);
    bitval >>= 8;
    bitpos -= 8;
  }
}

bool BassTable::parseTable(const string &text) {
  lstring lines = text.split("\n");
  for(auto &line : lines) {
    lstring part = line.split<1>(";");
    if(part.size() != 2) continue;

    Opcode opcode;
    assembleTableLHS(opcode, part[0].trim());
    assembleTableRHS(opcode, part[1].trim());
    table.append(opcode);
  }

  return true;
}

void BassTable::assembleTableLHS(Opcode &opcode, const string &text) {
  unsigned offset = 0;

  auto length = [&] {
    unsigned length = 0;
    while(text[offset + length]) {
      char n = text[offset + length];
      if(n == '*') break;
      length++;
    }
    return length;
  };

  while(text[offset]) {
    unsigned size = length();
    opcode.prefix.append({ substr(text, offset, size), size });
    offset += size;

    if(text[offset] != '*') continue;
    unsigned bits = 10 * (text[offset + 1] - '0');
    bits += text[offset + 2] - '0';
    opcode.number.append({ bits });
    offset += 3;
  }

  for(auto &prefix : opcode.prefix) {
    opcode.pattern.append(prefix.text, "*");
  }
  opcode.pattern.rtrim<1>("*");
  if(opcode.number.size() == opcode.prefix.size()) opcode.pattern.append("*");
}

void BassTable::assembleTableRHS(Opcode &opcode, const string &text) {
  unsigned offset = 0;

  lstring list = text.split(" ");
  for(auto &item : list) {
    if(item[0] == '$' && item.length() == 3) {
      opcode.format.append({ Format::Type::Static, (unsigned)hex((const char*)item + 1) });
    }

    if(item[0] == '=') {
      opcode.format.append({ Format::Type::Absolute, item[1] - 'a' });
    }

    if(item[0] == '+') {
      opcode.format.append({ Format::Type::Relative, item[2] - 'a', +(item[1] - '0') });
    }

    if(item[0] == '-') {
      opcode.format.append({ Format::Type::Relative, item[2] - 'a', -(item[1] - '0') });
    }

    if(item[0] == '*') {
      opcode.format.append({ Format::Type::Repeat, item[1] - 'a', (unsigned)hex((const char*)item + 3) });
    }
  }
}
