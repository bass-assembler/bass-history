#include "eval.cpp"

bool Bass::open(const string &filename) {
  close();
  if(options.overwrite == false) {
    output.open(filename, file::mode::readwrite);
  }
  if(output.open() == false) {
    output.open(filename, file::mode::write);
  }
  return output.open();
}

bool Bass::assemble(const string &filename) {
  for(pass = 1; pass <= 2; pass++) {
    endian = Endian::LSB;
    origin = 0;
    base = 0;
    for(unsigned n = 0; n < 256; n++) table[n] = n;
    defines.reset();
    activeDefine = 0;
    activeNamespace = "global";
    activeLabel = "global";
    defineExpandCounter = 1;
    negativeLabelCounter = 1;
    positiveLabelCounter = 1;
    stackPC.reset();
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

Bass::Bass() {
  options.caseInsensitive = false;
  options.overwrite = false;
}

//internal

void Bass::warning(const string &s) {
  print("[bass warning] ", fileName(), ":", lineNumber(), ":", blockNumber(), ":\n> ", s, "\n");
}

void Bass::error(const string &s) {
  print("[bass error] ", fileName(), ":", lineNumber(), ":", blockNumber(), ":\n> ", s, "\n");
  throw "";
}

unsigned Bass::pc() const {
  return origin + base;
}

void Bass::assembleFile(const string &filename) {
  fileName.push(filename);
  lineNumber.push(1);
  blockNumber.push(1);

  string filedata;
  if(filedata.readfile(filename) == false) {
    error({ "source file ", filename, " not found" });
  }

  if(options.caseInsensitive) filedata.qlower();

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

  //==========
  //= enddef =
  //==========
  if(line == "enddef" && activeDefine) {
    activeDefine->value.rtrim<1>("; ");
    activeDefine = 0;
    return true;
  }

  //=====================
  //= multi-line define =
  //=====================
  if(activeDefine) {
    if(line.beginswith("define ")) error("recursive defines are not supported");
    activeDefine->value.append(line, "; ");
    return true;
  }

  //==========
  //= define =
  //==========
  if(line.beginswith("define ")) {
    lstring part;
    part.split<1>("=", line);  //part[0] = define name [params...], part[1] = value
    part[0].trim(" ");
    if(part.size() == 2) {
      //single-line
      part[1].trim(" ");
      lstring header, args;
      header.split<2>(" ", part[0]);  //header[1] = name, header[2] = params
      if(header[2] != "") args.split(",", header[2]);
      setDefine(header[1], args, part[1]);
      return true;
    } else {
      //multi-line
      lstring header, args;
      header.split<2>(" ", line);  //header[1] = name, header[2] = params
      if(header[2] != "") args.split(",", header[2]);
      setDefine(header[1], args, "");
      activeDefine = &defines[defines.size() - 1];
      return true;
    }
  }

  return false;
}

bool Bass::assembleBlock(const string &s) {
  if(s == "") return true;
  string block = s;

  //==========
  //= endian =
  //==========
  if(block == "endian lsb") { endian = Endian::LSB; return true; }
  if(block == "endian msb") { endian = Endian::MSB; return true; }

  //==========
  //= incsrc =
  //==========
  if(block.wildcard("incsrc \"?*\"")) {
    block.ltrim<1>("incsrc ");
    block.trim<1>("\"");
    assembleFile({ dir(fileName()), block });
    return true;
  }

  //==========
  //= incbin =
  //==========
  if(block.wildcard("incbin \"?*\"*")) {
    block.ltrim<1>("incbin ");
    lstring part;
    part.qsplit(",", block);
    part[0].trim<1>("\"");
    file fp;
    if(fp.open({ dir(fileName()), part[0] }, file::mode::read) == false) {
      error({ "binary file ", part[0], " not found" });
    }
    unsigned offset = part.size() >= 2 ? eval(part[1]) : 0u;
    unsigned length = part.size() >= 3 ? eval(part[2]) : fp.size() - offset;
    if(length > fp.size()) error({ "binary file ", part[0], " include length exceeds file size "});
    fp.seek(offset);
    for(unsigned n = 0; n < length; n++) write(fp.read());
    fp.close();
    return true;
  }

  //=======
  //= org =
  //=======
  if(block.wildcard("org ?*")) {
    block.ltrim<1>("org ");
    origin = eval(block);
    seek(origin);
    return true;
  }

  //========
  //= base =
  //========
  if(block.wildcard("base ?*")) {
    block.ltrim<1>("base ");
    base = eval(block) - origin;
    return true;
  }

  //===========
  //= enqueue =
  //===========
  if(block == "enqueue pc") {
    stackPC.push(origin);
    return true;
  }

  //===========
  //= dequeue =
  //===========
  if(block == "dequeue pc") {
    if(stackPC.size() == 0) error("stack is empty");
    origin = stackPC.pull();
    seek(origin);
    return true;
  }

  //=========
  //= align =
  //=========
  if(block.wildcard("align ?*")) {
    block.ltrim<1>("align ");
    unsigned align = eval(block);
    if(align == 0) return false;
    while(pc() % align) write(0x00);
    return true;
  }

  //========
  //= fill =
  //========
  if(block.wildcard("fill ?*")) {
    block.ltrim<1>("fill ");
    lstring list;
    list.split(",", block);
    unsigned length = eval(list[0]);
    unsigned byte = list[1] == "" ? 0x00 : eval(list[1]);
    while(length--) write(byte);
    return true;
  }

  //======
  //= db =
  //======
  unsigned size = 0;
  if(block.beginswith("db ")) { block.ltrim<1>("db "); size = 1; }
  if(block.beginswith("dw ")) { block.ltrim<1>("dw "); size = 2; }
  if(block.beginswith("dl ")) { block.ltrim<1>("dl "); size = 3; }
  if(block.beginswith("dd ")) { block.ltrim<1>("dd "); size = 4; }
  if(block.beginswith("dq ")) { block.ltrim<1>("dq "); size = 8; }
  if(size != 0) {
    lstring list;
    list.qsplit(",", block);
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
  if(block.endswith(":") && !block.position(" ")) {
    block.rtrim<1>(":");
    if(block.beginswith(".")) {
      block = { activeLabel, block };
    } else {
      activeLabel = block;
    }
    setLabel({ activeNamespace, "::", block }, pc());
    return true;
  }

  //===========
  //= + label =
  //===========
  if(block == "+") {
    if(pass == 1) setLabel({ "+", positiveLabelCounter }, pc());
    positiveLabelCounter++;
    return true;
  }

  //===========
  //= - label =
  //===========
  if(block == "-") {
    if(pass == 1) setLabel({ "-", negativeLabelCounter }, pc());
    negativeLabelCounter++;
    return true;
  }

  //=============
  //= namespace =
  //=============
  if(block.wildcard("namespace ?*")) {
    block.ltrim<1>("namespace ");
    activeNamespace = block;
    return true;
  }

  //=========
  //= print =
  //=========
  if(block.wildcard("print ?*")) {
    if(pass == 2) {
      block.ltrim<1>("print ");
      lstring list;
      list.qsplit(",", block);
      foreach(item, list) {
        if(item == "pc") {
          print("0x", hex(pc()));
        } else if(item == "origin") {
          print("0x", hex(origin));
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

void Bass::setDefine(const string &name_, const lstring &args, const string &value) {
  string name = name_;

  if(name.wildcard("'?'") && args.size() == 0) {
    table[name[1]] = eval(value);
    return;
  }

  if(!name.position("::")) name = { activeNamespace, "::", name };

  foreach(define, defines) {
    if(name == define.name) {
      define.value = value;
      return;
    }
  }
  defines.append({ name, args, value });
}

void Bass::setLabel(const string &name, unsigned offset) {
  if(pass == 1) foreach(label, labels) {
    if(name == label.name) error({ "Label ", name, " has already been declared" });
  }
  labels.append({ name, offset });
}

void Bass::seek(unsigned offset) {
  output.seek(offset);
}

void Bass::write(uint64_t data, unsigned length) {
  if(pass == 2) {
    if(endian == Endian::LSB) output.writel(data, length);
    if(endian == Endian::MSB) output.writem(data, length);
  }
  origin += length;
}
