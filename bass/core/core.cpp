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
    for(auto &macro : defaultMacros) macros.append(macro);
    activeNamespace = "global";
    activeLabel = "#invalid";
    macroDepth = 0;
    macroExpandCounter = 1;
    macroRecursionCounter = 1;
    lastLabelCounter = 1;
    nextLabelCounter = 1;
    condition = Condition::Matching;
    conditionStack.reset();
    conditionStack.append(condition);
    stack.reset();
    lineNumber.reset();
    blockNumber.reset();
    activeLine.reset();
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
  print("[bass warning] ", fileName.last(), ":", lineNumber.last(), ":", blockNumber.last(), ":\n> ", s, "\n");
}

void Bass::error(const string &s) {
  print("[bass error] ", fileName.last(), ":", lineNumber.last(), ":", blockNumber.last(), ":\n> ", s, "\n");
  throw "";
}

unsigned Bass::pc() const {
  return origin + base;
}

string Bass::qualifyMacro(string name) {
  if(!name.position("::")) {
    name = {activeNamespace, "::", name};
  }
  if(name.wildcard("self::?*")) {
    name.ltrim<1>("self::");
    return {"#", macroRecursionCounter, "::", name};
  }
  return name;
}

string Bass::qualifyLabel(string name) {
  if(name.beginswith(".")) {
    if(activeLabel == "#invalid") error("sub-label without matching label");
    name = {activeLabel, name};
  }
  if(!name.position("::")) {
    name = {activeNamespace, "::", name};
  }
  if(name.wildcard("self::?*")) {
    name.ltrim<1>("self::");
    return {"#", macroExpandCounter, "::", name};
  }
  return name;
}

void Bass::assembleFile(const string &filename) {
  fileName.append(filename);
  lineNumber.append(1);
  blockNumber.append(1);
  activeLine.append("");

  if(!file::exists(filename)) error({"source file ", filename, " not found"});
  string source = file::read(filename);
  source.transform("\r\t", "  ");  //treat ignored whitespace characters as spaces
  assembleSource(source);

  fileName.remove();
  lineNumber.remove();
  blockNumber.remove();
  activeLine.remove();
}

void Bass::assembleMacro(const string &name, const lstring &args) {
  for(auto &match : macros) {
    if(name == match.name && args.size() == match.args.size()) {
      Macro macro = match;  //copy macro; as setMacro() invalidates our iterator

      fileName.append({"{macro ", macro.name, "}"});
      lineNumber.append(1);
      blockNumber.append(1);
      activeLine.append("");

      for(unsigned n = 0; n < args.size(); n++) {
        string name = {"#", macroRecursionCounter + 1, "::", macro.args[n]};
        string value = args[n];
        evalDefines(value);
        setMacro(name, lstring(), value);
      }

      macroExpandCounter++;
      macroRecursionCounter++;
      assembleSource(macro.value);
      macroRecursionCounter--;

      fileName.remove();
      lineNumber.remove();
      blockNumber.remove();
      activeLine.remove();

      return;
    }
  }

  error({"macro not found: ", name});
}

void Bass::assembleSource(const string &source) {
  lstring lines = source.split("\n");
  for(auto &line : lines) {
    if(auto position = line.qposition("//")) line[position()] = 0;  //strip comments
    if(options.caseInsensitive) line.qlower();

    blockNumber.last() = 1;
    activeLine.last() = line;

    lstring blocks = activeLine.last().qsplit(";");
    for(auto &block : blocks) {
      if(block.strip().empty()) continue;
      if(assembleBlock(block) == false) error({"unknown command: ", block});
      blockNumber.last()++;
    }

    lineNumber.last()++;
  }

  if(fileName.size() == 1) {
    if(conditionStack.size() != 1) error("if without matching endif");
    if(macroDepth) error("macro without matching endmacro");
  }
}

bool Bass::assembleBlock(string &block) {
  //================
  //= conditionals =
  //================
  if(macroDepth == 0) {
    if((condition == Condition::Matching)
    || (conditionStack.size() == 0 && block.wildcard("elseif ?*"))
    ) {  //do not evaluate macros inside unmatched conditional blocks (prevent infinite recursion)
      if(evalMacros(block)) return true;
    }

    if(block.wildcard("if ?*")) {
      block.ltrim<1>("if ");
      conditionStack.append(condition);
      condition = eval(block) ? Condition::Matching : Condition::NotYetMatched;
      for(auto &item : conditionStack) {
        if(item != Condition::Matching) condition = Condition::AlreadyMatched;
      }
      return true;
    }

    if(block.wildcard("elseif ?*")) {
      if(condition != Condition::NotYetMatched) {
        condition = Condition::AlreadyMatched;
        return true;
      }
      block.ltrim<1>("elseif ");
      condition = eval(block) ? Condition::Matching : Condition::NotYetMatched;
      return true;
    }

    if(block == "else") {
      if(condition != Condition::NotYetMatched) {
        condition = Condition::AlreadyMatched;
        return true;
      }
      condition = Condition::Matching;
      return true;
    }

    if(block == "endif") {
      condition = conditionStack.take();
      if(conditionStack.size() == 0) error("endif without matching if");
      return true;
    }
  }

  if(condition != Condition::Matching) {
    return true;
  }

  //==========
  //= macros =
  //==========
  if(block == "endmacro") {
    if(macroDepth == 0) error("endmacro without matching macro");
    if(--macroDepth) {
      activeMacro.value.append(block, "; ");
      return true;
    }
    activeMacro.value.rtrim<1>("; ");
    setMacro(activeMacro.name, activeMacro.args, activeMacro.value);
    return true;
  }

  if(macroDepth) {
    if(block.beginswith("macro ")) macroDepth++;
    activeMacro.value.append(block, "; ");
    return true;
  }

  if(block.beginswith("macro ")) {
    block.ltrim<1>("macro ");
    lstring header = block.split<2>(" "), args;  //header[0] = name, header[1] = params
    if(!header(1).empty()) args = header(1).split(",");
    activeMacro.name = header(0);
    activeMacro.args = args;
    activeMacro.value = "";
    macroDepth++;
    return true;
  }

  //===========
  //= defines =
  //===========
  if(block.wildcard("define '?' ?*")) {
    block.ltrim<1>("define ");
    table[block[1]] = eval((const char*)block + 3);
    return true;
  }

  if(block.wildcard("define ?*")) {
    block.ltrim<1>("define ");
    lstring part = block.split<1>(" ");
    string name = qualifyMacro(part(0));
    setMacro(name, lstring(), part(1));
    return true;
  }

  if(block.wildcard("eval ?* ?*")) {
    block.ltrim<1>("eval ");
    lstring part = block.split<1>(" ");
    string name = qualifyMacro(part(0));
    setMacro(name, lstring(), eval(part[1]));
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
    assembleFile({dir(fileName.last()), block});
    return true;
  }

  //============
  //= incbinas =
  //============
  string incbinName;
  if(block.wildcard("incbinas ?*")) {
    block.ltrim<1>("incbinas ");
    lstring part = block.qsplit(",").strip();
    incbinName = part.take(0);

    //fallthrough to incbin
    block = {"incbin ", part.concatenate(",")};
  }

  //==========
  //= incbin =
  //==========
  if(block.wildcard("incbin \"?*\"*")) {
    block.ltrim<1>("incbin ");
    lstring part = block.qsplit(",").strip();
    part[0].trim<1>("\"");
    file fp({dir(fileName.last()), part[0]}, file::mode::read);
    if(!fp.open()) error({"binary file ", part[0], " not found"});
    unsigned offset = 0, length = fp.size() - offset;
    if(part.size() >= 2) offset = eval(part[1]);
    if(part.size() >= 3) length = eval(part[2]);
    if(length > fp.size()) error({"binary file ", part[0], " include length exceeds file size"});
    if(incbinName) {  //incbinas name present; declare helper labels
      setLabel(incbinName, pc());
      setLabel({incbinName, ".size"}, length);
    }
    fp.seek(offset);
    for(unsigned n = 0; n < length; n++) write(fp.read());
    return true;
  }

  //==========
  //= origin =
  //==========
  if(block.wildcard("origin ?*")) {
    block.ltrim<1>("origin ");
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

  //========
  //= push =
  //========
  if(block.wildcard("push ?*")) {
    block.ltrim<1>("push ");
    lstring list = block.split(",");
    for(auto &item : list) {
      if(item == "origin") {
        stack.append(origin);
      } else if(item == "base") {
        stack.append(base);
      } else if(item == "pc") {
        stack.append(origin);
        stack.append(base);
      } else if(item == "namespace") {
        stack.append(activeNamespace);
      } else if(item == "label") {
        stack.append(activeLabel);
      } else if(item.wildcard("\"*\"")) {
        item.trim<1>("\"");
        stack.append(item);
      } else {
        error({"unrecognized push argument: ", item});
      }
    }
    return true;
  }

  //========
  //= pull =
  //========
  if(block.wildcard("pull ?*")) {
    block.ltrim<1>("pull ");
    lstring list = block.split(",");
    for(auto &item : list) {
      if(stack.size() == 0) error("stack is empty");
      if(item == "origin") {
        origin = decimal(stack.take());
        seek(origin);
      } else if(item == "base") {
        base = integer(stack.take());
      } else if(item == "pc") {
        base = integer(stack.take());
        origin = decimal(stack.take());
        seek(origin);
      } else if(item == "namespace") {
        activeNamespace = stack.take();
      } else if(item == "label") {
        activeLabel = stack.take();
      } else if(item == "null") {
        stack.take();
      } else {
        error({"unrecognized pull argument: ", item});
      }
    }
    return true;
  }

  //=========
  //= align =
  //=========
  if(block.wildcard("align ?*")) {
    block.ltrim<1>("align ");
    lstring list = block.split(",");
    unsigned align = eval(list[0]);
    unsigned byte = eval(list(1, "0x00"));
    if(align == 0) return false;
    while(pc() % align) write(byte);
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
    string name = block.rtrim<1>(":");
    if(!name.beginswith(".")) activeLabel = name;
    setLabel(name, pc());
    return true;
  }

  //====================
  //= anonymous labels =
  //====================
  if(block == "-") {
    if(pass == 1) setLabel({"anonymous::relativeLast", lastLabelCounter}, pc());
    lastLabelCounter++;
    return true;
  }

  if(block == "+") {
    if(pass == 1) setLabel({"anonymous::relativeNext", nextLabelCounter}, pc());
    nextLabelCounter++;
    return true;
  }

  //=============
  //= namespace =
  //=============
  if(block.wildcard("namespace ?*")) {
    block.ltrim<1>("namespace ");
    stack.append(activeLabel);
    stack.append(activeNamespace);
    activeNamespace = block;
    activeLabel = "#invalid";
    return true;
  }

  //================
  //= endnamespace =
  //================
  if(block == "endnamespace") {
    if(stack.size() < 2) error("stack is empty; possiblity: endnamespace without matching namespace");
    activeNamespace = stack.take();
    activeLabel = stack.take();
    return true;
  }

  //========
  //= echo =
  //========
  if(block.wildcard("echo \"*\"")) {
    if(pass == 2) {
      block.ltrim<1>("echo ");
      block.trim<1>("\"");
      block.replace("\\", "\n");
      block.replace("\\\\", "\\");
      print(block);
    }
    return true;
  }

  //=========
  //= print =
  //=========
  if(block.wildcard("print \"*\"")) {
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
  if(block.wildcard("warning \"*\"")) {
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
  if(block.wildcard("error \"*\"")) {
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
  name = qualifyMacro(name);

  for(auto &macro : macros) {
    if(name == macro.name) {
      macro.value = value;
      return;
    }
  }

  macros.append({name, args, value});
}

void Bass::setLabel(const string &name_, unsigned offset) {
  string name = name_;
  name = qualifyLabel(name);

  //labels cannot be redeclared
  for(auto &label : labels) {
    if(name == label.name) {
      if(pass == 1) error({"Label ", name, " has already been declared"});
    }
  }

  labels.append({name, offset});
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
