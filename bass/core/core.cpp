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
    defines.reset();
    activeDefine = 0;
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
    activeDefine->value.append(line, "; ");
    return true;
  }

  //==========
  //= define =
  //==========
  if(line.beginswith("define ") && !activeDefine) {
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
    assembleFile(block);
    return true;
  }

  //==========
  //= incbin =
  //==========
  if(block.wildcard("incbin \"?*\"")) {
    block.ltrim<1>("incbin ");
    block.trim<1>("\"");
    file fp;
    if(fp.open(block, file::mode::read) == false) {
      error({ "binary file ", block, " not found" });
    }
    unsigned size = fp.size();
    for(unsigned n = 0; n < size; n++) write(fp.read());
    fp.close();
    return true;
  }

  //=======
  //= org =
  //=======
  if(block.wildcard("org ?*")) {
    block.ltrim<1>("org ");
    offset = base = eval(block);
    seek(offset);
    return true;
  }

  //========
  //= base =
  //========
  if(block.wildcard("base ?*")) {
    block.ltrim<1>("base ");
    base = eval(block);
    return true;
  }

  //=========
  //= align =
  //=========
  if(block.wildcard("align ?*")) {
    block.ltrim<1>("align ");
    unsigned align = eval(block);
    if(align == 0) return false;
    while(base % align) write(0x00);
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
    labels.append({ { activeNamespace, "::", block }, base });
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
