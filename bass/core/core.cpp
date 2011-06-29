#include "eval.cpp"
#include "../arch/snes-cpu/snes-cpu.cpp"

bool Bass::open(const string &filename) {
  if(output.open(filename, file::mode::readwrite) == false) {
    if(output.open(filename, file::mode::write) == false) return false;
  }
  return true;
}

bool Bass::assemble(const string &filename) {
  for(pass = 1; pass <= 2; pass++) {
    endian = Endian::LSB;
    offset = 0;
    base = 0;
    for(unsigned n = 0; n < 256; n++) table[n] = n;
    macros.reset();
    defines.reset();
    activeMacro = 0;
    activeNamespace = "global";
    activeLabel = "global";
    negativeLabelCounter = 1;
    positiveLabelCounter = 1;
    try {
      assembleFile(filename);
    } catch(const char*) {
      return false;
    }
  }
  return true;
}

void Bass::close() {
  if(output.open()) output.close();
}

//internal

void Bass::warning(const string &s) {
  print("[bass warning] ", fileName(), ":", lineNumber(), ":", blockNumber(), ":\n> ", s, "\n");
}

void Bass::error(const string &s) {
  print("[bass error] ", fileName(), ":", lineNumber(), ":", blockNumber(), ":\n> ", s, "\n");
  throw "";
}

void Bass::assembleFile(const string &filename) {
  fileName.push(filename);
  lineNumber.push(1);
  blockNumber.push(1);

  string filedata;
  if(filedata.readfile(filename) == false) {
    error({ "source file ", filename, " not found" });
  }

  filedata.transform("\r", " ");
  filedata.transform("\t", " ");

  lstring lines;
  lines.split("\n", filedata);
  foreach(line, lines) {
    blockNumber() = 1;
    if(auto position = line.position("//")) line[position()] = 0;  //strip comments
    while(auto position = line.qposition("  ")) line.qreplace("  ", " ");
    line.qreplace(", ", ",");
    line.trim(" ");
    if(assembleLine(line) == false) {
      evalMacros(line);
      evalDefines(line);
      lstring blocks;
      blocks.qsplit(";", line);
      foreach(block, blocks) {
        block.trim(" ");
        if(assembleBlock(block) == false) {
          error({ "unknown command:", block });
        }
        blockNumber()++;
      }
    }
    lineNumber()++;
  }

  fileName.pull();
  lineNumber.pull();
  blockNumber.pull();
}

bool Bass::assembleLine(const string &line) {
  if(line == "") return true;

  if(activeMacro && line != "endmacro") {
    activeMacro->value.append(line, "; ");
    return true;
  }

  lstring part;
  part.qsplit(" ", line);

  //==========
  //= define =
  //==========
  if(part[0] == "define" && part.size() >= 2) {
    part.split<2>(" ", line);

    //define 'A' 0x41
    if(part[1].wildcard("'?'")) {
      uint8_t index = part[1][1];
      table[index] = eval(part[2]);
      return true;
    }

    setDefine(part[1], part[2]);
    return true;
  }

  //=========
  //= macro =
  //=========
  if(part[0] == "macro" && part.size() >= 2) {
    if(activeMacro) error("already constructing macro");

    part.split<1>(" ", line);
    part[1].rtrim<1>(")");
    lstring subpart;
    subpart.split<1>("(", part[1]);
    lstring args;
    if(subpart[1] != "") args.split(",", subpart[1]);
    if(!subpart[0].position("::")) subpart[0] = { activeNamespace, "::", subpart[0] };
    foreach(macro, macros) {
      if(subpart[0] == macro.name) {
        macro.args = args;
        macro.value = "";
        activeMacro = &macro;
        return true;
      }
    }
    macros.append({ subpart[0], args, "" });
    activeMacro = &macros[macros.size() - 1];
    return true;
  }

  //============
  //= endmacro =
  //============
  if(line == "endmacro" && part[0] == "endmacro" && part.size() == 1 && activeMacro) {
    activeMacro->value.rtrim<1>("; ");
    activeMacro = 0;
    return true;
  }

  return false;
}

bool Bass::assembleBlock(const string &block) {
  if(block == "") return true;

  lstring part;
  part.qsplit(" ", block);

  //==========
  //= endian =
  //==========
  if(part[0] == "endian" && part.size() == 2) {
    if(part[1] == "lsb") { endian = Endian::LSB; return true; }
    if(part[1] == "msb") { endian = Endian::MSB; return true; }
  }

  //==========
  //= incsrc =
  //==========
  if(part[0] == "incsrc" && part.size() == 2) {
    part[1].trim<1>("\"");
    assembleFile(part[1]);
    return true;
  }

  //==========
  //= incbin =
  //==========
  if(part[0] == "incbin" && part.size() == 2) {
    part[1].trim<1>("\"");
    file fp;
    if(fp.open(part[1], file::mode::read) == false) {
      error({ "binary file ", part[1], " not found" });
    }
    unsigned size = fp.size();
    for(unsigned n = 0; n < size; n++) write(fp.read());
    fp.close();
    return true;
  }

  //=======
  //= org =
  //=======
  if(part[0] == "org" && part.size() == 2) {
    offset = base = eval(part[1]);
    seek(offset);
    return true;
  }

  //========
  //= base =
  //========
  if(part[0] == "base" && part.size() == 2) {
    base = eval(part[1]);
    return true;
  }

  //=========
  //= align =
  //=========
  if(part[0] == "base" && part.size() == 2) {
    unsigned align = eval(part[1]);
    while(base % align) write(0x00);
    return true;
  }

  //========
  //= fill =
  //========
  if(part[0] == "fill" && part.size() >= 2) {
    lstring list;
    list.split(",", part[1]);
    unsigned length = eval(list[0]);
    unsigned byte = list[1] == "" ? 0x00 : eval(list[1]);
    while(length--) write(byte);
    return true;
  }

  //======
  //= db =
  //======
  unsigned size = 0;
  if(part[0] == "db" && part.size() == 2) size = 1;
  if(part[0] == "dw" && part.size() == 2) size = 2;
  if(part[0] == "dl" && part.size() == 2) size = 3;
  if(part[0] == "dd" && part.size() == 2) size = 4;
  if(part[0] == "dq" && part.size() == 2) size = 8;
  if(size != 0) {
    lstring list;
    list.qsplit(",", part[1]);
    foreach(item, list) {
      if(item.wildcard("\"*\"") == false) {
        write(eval(item), size);
      } else {
        item.trim<1>("\"");
        foreach(n, item) write(table[n], size);
      }
    }
    return true;
  }

  //=========
  //= label =
  //=========
  if(part[0].endswith(":") && part.size() == 1) {
    part[0].rtrim<1>(":");
    if(part[0].beginswith(".")) {
      part[0] = string(activeLabel, part[0]);
    } else {
      activeLabel = part[0];
    }
    labels.append({ { activeNamespace, "::", part[0] }, base });
    return true;
  }

  //===========
  //= + label =
  //===========
  if(block == "+") {
    if(pass == 1) labels.append({ { "+", positiveLabelCounter }, base });
    positiveLabelCounter++;
    return true;
  }

  //===========
  //= - label =
  //===========
  if(block == "-") {
    if(pass == 1) labels.append({ { "-", negativeLabelCounter }, base });
    negativeLabelCounter++;
    return true;
  }

  //=============
  //= namespace =
  //=============
  if(part[0] == "namespace" && part.size() == 2) {
    activeNamespace = part[1];
    return true;
  }

  //=========
  //= print =
  //=========
  if(part[0] == "print" && part.size() == 2) {
    if(pass == 2) {
      lstring list;
      list.qsplit(",", part[1]);
      foreach(item, list) {
        if(item == "org") {
          print("0x", hex(offset));
        } else if(item == "base") {
          print("0x", hex(base));
        } else {
          item.trim<1>("\"");
          print(item);
        }
      }
      print("\n");
    }
    return true;
  }

  return false;
}

void Bass::setDefine(const string &name_, const string &value) {
  string name = name_;
  if(!name.position("::")) name = { activeNamespace, "::", name };

  foreach(define, defines) {
    if(name == define.name) {
      define.value = value;
      return;
    }
  }
  defines.append({ name, value });
}

void Bass::seek(unsigned offset) {
  output.seek(offset);
}

void Bass::write(uint64_t data, unsigned length) {
  if(pass == 2) {
    if(endian == Endian::LSB) output.writel(data, length);
    if(endian == Endian::MSB) output.writem(data, length);
  }
  offset += length;
  base += length;
}
