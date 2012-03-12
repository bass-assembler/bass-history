#define arch(name) static string Arch_##name
#include "snes-cpu.arch"
#include "snes-smp.arch"
#undef arch

void BassTable::initialize(unsigned pass) {
  bitval = 0;
  bitpos = 0;
  table.reset();
}

bool BassTable::assembleBlock(const string &block) {
  if(Bass::assembleBlock(block) == true) return true;

  if(block.beginswith("arch")) {
    if(block.beginswith("arch ")) {
      string name = substr(block, 5), data;
      if(0);
      else if(name == "snes.cpu") data = Arch_snes_cpu;
      else if(name == "snes.smp") data = Arch_snes_smp;
      else {
        name.trim<1>("\"");
        name = { dir(fileName.last()), name };
        if(data.readfile(name) == false) error({ "arch: file \"", name, "\" not found" });
      }
      table.reset();
      parseTable(data);
      return true;
    } else if(block == "arch.reset") {
      table.reset();
      return true;
    } else if(block.wildcard("arch.append \"*\"")) {
      string data = block;
      data.ltrim<1>("arch.append ");
      data.trim<1>("\"");
      parseTable(data);
      return true;
    }
  }

  unsigned pc = this->pc();

  for(auto &opcode : table) {
    if(tokenize(block, opcode.pattern) == false) continue;

    lstring args;
    tokenize(args, block, opcode.pattern);
    if(args.size() != opcode.number.size()) continue;

    bool mismatch = false;
    for(auto &format : opcode.format) {
      if(format.type != Format::Type::Absolute) continue;
      if(format.match == Format::Match::Weak) continue;
      unsigned bits = bitLength(args[format.argument]);
      if(bits != opcode.number[format.argument].bits) {
        if(bits == 0 && format.match == Format::Match::Strong) continue;
        mismatch = true;
        break;
      }
    }
    if(mismatch) continue;

    for(auto &format : opcode.format) {
      switch(format.type) {
      case Format::Type::Static: {
        writeBits(format.data, format.bits);
      } break;
      case Format::Type::Absolute: {
        unsigned data = eval(args[format.argument]);
        writeBits(data, opcode.number[format.argument].bits);
      } break;
      case Format::Type::Relative: {
        signed data = eval(args[format.argument]) - (pc + format.displacement);
        unsigned bits = opcode.number[format.argument].bits;
        signed min = -(1 << (bits - 1)), max = +(1 << (bits - 1)) - 1;
        if(data < min || data > max) error("branch out of bounds");
        writeBits(data, opcode.number[format.argument].bits);
      } break;
      case Format::Type::Repeat: {
        unsigned data = eval(args[format.argument]);
        for(unsigned n = 0; n < data; n++) writeBits(format.data, opcode.number[format.argument].bits);
      } break;
      }
    }

    return true;
  }

  return false;
}

unsigned BassTable::bitLength(string &text) const {
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
      if(*p == '0' || *p == '1') { p++; length += 1; continue; }
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
      Format format = { Format::Type::Static };
      format.data = hex((const char*)item + 1);
      format.bits = (item.length() - 1) * 4;
      opcode.format.append(format);
    }

    if(item[0] == '%') {
      Format format = { Format::Type::Static };
      format.data = binary((const char*)item + 1);
      format.bits = (item.length() - 1);
      opcode.format.append(format);
    }

    if(item[0] == '!') {
      Format format = { Format::Type::Absolute, Format::Match::Exact };
      format.argument = item[1] - 'a';
      opcode.format.append(format);
    }

    if(item[0] == '=') {
      Format format = { Format::Type::Absolute, Format::Match::Strong };
      format.argument = item[1] - 'a';
      opcode.format.append(format);
    }

    if(item[0] == '~') {
      Format format = { Format::Type::Absolute, Format::Match::Weak };
      format.argument = item[1] - 'a';
      opcode.format.append(format);
    }

    if(item[0] == '+') {
      Format format = { Format::Type::Relative };
      format.argument = item[2] - 'a';
      format.displacement = +(item[1] - '0');
      opcode.format.append(format);
    }

    if(item[0] == '-') {
      Format format = { Format::Type::Relative };
      format.argument = item[2] - 'a';
      format.displacement = -(item[1] - '0');
      opcode.format.append(format);
    }

    if(item[0] == '*') {
      Format format = { Format::Type::Repeat };
      format.argument = item[1] - 'a';
      format.data = hex((const char*)item + 3);
      opcode.format.append(format);
    }
  }
}
