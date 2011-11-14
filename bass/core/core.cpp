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
    initialize(pass);
    endian = Endian::LSB;
    origin = 0;
    base = 0;
    for(unsigned n = 0; n < 256; n++) table[n] = n;
    macros.reset();
    activeNamespace = "global";
    activeLabel = "#invalid";
    macroNestingCounter = 0;
    macroExpandCounter = 1;
    relativeLabelCounter = 1;
    conditionalState = Conditional::Matching;
    stackPC.reset();
    stackConditional.reset();
    stackConditional.push(conditionalState);
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

void Bass::initialize(unsigned pass) {
}

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
  filedata.transform("\r\t", "  ");

  lstring lines = filedata.split("\n");
  for(auto &line : lines) {
    blockNumber() = 1;
    if(auto position = line.position("//")) line[position()] = 0;  //strip comments
    while(auto position = line.qposition("  ")) line.qreplace("  ", " ");
    line.qreplace(", ", ",");
    line.trim(" ");

    while(true) {
      evalMacros(line);
      lstring block = line.qsplit<1>(";");
      line.ltrim<1>(string(block[0], ";"));
      block[0].trim(" ");
      if(assembleBlock(block[0]) == false) error({ "unknown command:", block[0] });
      blockNumber()++;
      if(block.size() == 1) break;
    }

    lineNumber()++;
  }

  if(stackConditional.size() != 1) error("if without matching endif");
  if(macroNestingCounter) error("macro without matching endmacro");

  fileName.pull();
  lineNumber.pull();
  blockNumber.pull();
}

bool Bass::assembleBlock(const string &block_) {
  if(block_ == "") return true;
  string block = block_;

  //================
  //= conditionals =
  //================

  if(block.wildcard("if ?*")) {
    block.ltrim<1>("if ");
    stackConditional.push(conditionalState);
    conditionalState = eval(block) ? Conditional::Matching : Conditional::NotYetMatched;
    for(auto &item : stackConditional) {
      if(item != Conditional::Matching) conditionalState = Conditional::AlreadyMatched;
    }
    return true;
  }

  if(block.wildcard("elseif ?*")) {
    if(conditionalState != Conditional::NotYetMatched) {
      conditionalState = Conditional::AlreadyMatched;
      return true;
    }
    block.ltrim<1>("elseif ");
    conditionalState = eval(block) ? Conditional::Matching : Conditional::NotYetMatched;
    return true;
  }

  if(block == "else") {
    if(conditionalState != Conditional::NotYetMatched) {
      conditionalState = Conditional::AlreadyMatched;
      return true;
    }
    conditionalState = Conditional::Matching;
    return true;
  }

  if(block == "endif") {
    conditionalState = stackConditional.pull();
    if(stackConditional.size() == 0) error("endif without matching if");
    return true;
  }

  if(conditionalState != Conditional::Matching) {
    return true;
  }

  //==========
  //= macros =
  //==========

  if(block == "endmacro") {
    if(macroNestingCounter == 0) error("endmacro without matching macro");
    if(--macroNestingCounter) {
      activeMacro.value.append(block, "; ");
      return true;
    }
    activeMacro.value.rtrim<1>("; ");
    setMacro(activeMacro.name, activeMacro.args, activeMacro.value);
    return true;
  }

  if(macroNestingCounter) {
    if(block.beginswith("macro ")) macroNestingCounter++;
    activeMacro.value.append(block, "; ");
    return true;
  }

  if(block.beginswith("macro ")) {
    block.ltrim<1>("macro ");
    lstring header = block.split<2>(" "), args;  //header[0] = name, header[1] = params
    if(header(1, "") != "") args = header[1].split(",");
    activeMacro.name = header[0];
    activeMacro.args = args;
    activeMacro.value = "";
    macroNestingCounter++;
    return true;
  }

  //===========
  //= defines =
  //===========

  if(block.wildcard("define '?*' ?*")) {
    block.ltrim<1>("define ");
    table[block[1]] = eval((const char*)block + 3);
    return true;
  }

  if(block.wildcard("define ?* ?*")) {
    block.ltrim<1>("define ");
    lstring part = block.split<1>(" ");
    if(!part[0].position("::")) part[0] = { activeNamespace, "::", part[0] };
    setMacro(part[0], lstring(), part[1]);
    return true;
  }

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
    lstring part = block.qsplit(",");
    part[0].trim<1>("\"");
    file fp;
    if(fp.open({ dir(fileName()), part[0] }, file::mode::read) == false) {
      error({ "binary file ", part[0], " not found" });
    }
    unsigned offset = 0, length = fp.size() - offset;
    if(part.size() >= 2) offset = eval(part[1]);
    if(part.size() >= 3) length = eval(part[2]);
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

  //==========
  //= pushpc =
  //==========
  if(block == "pushpc") {
    stackPC.push(origin);
    return true;
  }

  //==========
  //= pullpc =
  //==========
  if(block == "pullpc") {
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
    lstring list = block.split(",");
    unsigned length = eval(list[0]);
    unsigned byte = eval(list(1, "0x00"));
    while(length--) write(byte);
    return true;
  }

  //========
  //= seek =
  //========
  if(block.wildcard("seek ?*")) {
    block.ltrim<1>("seek ");
    origin += eval(block);
    seek(origin);
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
    lstring list = block.qsplit(",");
    for(auto &item : list) {
      if(item.wildcard("\"*\"") == false) {
        write(eval(item), size);
      } else {
        item.trim<1>("\"");
        for(auto &n : item) write(table[n], size);
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
      if(activeLabel == "#invalid") error("sub-label without matching label");
      block = { activeLabel, block };
    } else {
      activeLabel = block;
    }
    setLabel({ activeNamespace, "::", block }, pc());
    return true;
  }

  //============================
  //= relative anonymous label =
  //============================
  if(block == "~" || block == "-" || block == "+") {
    if(pass == 1) setLabel({ "anonymous::relative", relativeLabelCounter }, pc());
    relativeLabelCounter++;
    return true;
  }

  //=============
  //= namespace =
  //=============
  if(block.wildcard("namespace ?*")) {
    block.ltrim<1>("namespace ");
    activeNamespace = block;
    activeLabel = "#invalid";
    return true;
  }

  //=========
  //= print =
  //=========
  if(block.wildcard("print \"?*\"")) {
    if(pass == 2) {
      block.ltrim<1>("print ");
      block.trim<1>("\"");
      print(block, "\n");
    }
    return true;
  }

  //===========
  //= warning =
  //===========
  if(block.wildcard("warning \"?*\"")) {
    if(pass == 2) {
      block.ltrim<1>("warning ");
      block.trim<1>("\"");
      warning(block);
    }
    return true;
  }

  //=========
  //= error =
  //=========
  if(block.wildcard("error \"?*\"")) {
    if(pass == 2) {
      block.ltrim<1>("error ");
      block.trim<1>("\"");
      error(block);
    }
    return true;
  }

  return false;
}

void Bass::setMacro(const string &name_, const lstring &args, const string &value) {
  string name = name_;
  if(!name.position("::")) name = { activeNamespace, "::", name };

  for(auto &macro : macros) {
    if(name == macro.name) {
      macro.value = value;
      return;
    }
  }

  macros.append({ name, args, value });
}

void Bass::setLabel(const string &name, unsigned offset) {
  //labels cannot be redeclared
  for(auto &label : labels) {
    if(name == label.name) {
      if(pass == 1) error({ "Label ", name, " has already been declared" });
    }
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
