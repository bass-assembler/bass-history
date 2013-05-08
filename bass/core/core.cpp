#include "eval.cpp"

bool Bass::open(const string& filename, Bass::FileMode mode) {
  close();
  if(pass == 0 && !filename) return true;  //filename not required on command-line

  if(mode == FileMode::Auto) {
    if(options.overwrite) mode = FileMode::Create;
  }
  if(mode == FileMode::Modify) {
    output.open(filename, file::mode::readwrite);
  }
  if(output.open() == false) {
    output.open(filename, file::mode::write);
  }
  if(output.open() == true) {
    origin = 0;
    base = 0;
    return true;
  }
  return false;
}

bool Bass::assemble(const string& filename) {
  for(pass = 1; pass <= 2; pass++) {
    initialize(pass);
    endian = Endian::LSB;
    origin = 0;
    base = 0;
    for(unsigned n = 0; n < 256; n++) table[n] = n;
    macros.reset();
    for(auto& macro : defaultMacros) macros.insert(macro);
    activeNamespace = "global";
    activeLabel = "global";
    macroDepth = 0;
    macroExpandCounter = 1;
    macroReturnStack.reset();
    lastLabelCounter = 1;
    nextLabelCounter = 1;
    conditionStack.reset();
    conditionStack.append(Condition::Matching);
    stack.reset();
    lineNumber.reset();
    blockNumber.reset();
    try {
      assembleFile(filename);
    } catch(...) {
      return false;
    }
  }
  return true;
}

void Bass::close() {
  if(output.open()) output.close();
}

Bass::Bass() {
  pass = 0;
  options.caseInsensitive = false;
  options.overwrite = false;
}

//internal

void Bass::initialize(unsigned pass) {
}

template<typename... Args> void Bass::warning(Args&&... args) {
  string s = string(std::forward<Args>(args)...);
  print("[bass warning] ", fileName.last(), ":", lineNumber.last(), ":", blockNumber.last(), ":\n> ", s, "\n");
}

template<typename... Args> void Bass::error(Args&&... args) {
  string s = string(std::forward<Args>(args)...);
  print("[bass error] ", fileName.last(), ":", lineNumber.last(), ":", blockNumber.last(), ":\n> ", s, "\n");
  struct bass_parser_error{};
  throw bass_parser_error{};
}

unsigned Bass::pc() const {
  return origin + base;
}

void Bass::seek(unsigned offset) {
  if(output.open() == false) error("output file not opened\n");
  output.seek(offset);
}

void Bass::write(uint64_t data, unsigned length) {
  if(output.open() == false) error("output file not opened\n");
  if(pass == 2) {
    if(endian == Endian::LSB) output.writel(data, length);
    if(endian == Endian::MSB) output.writem(data, length);
  }
  origin += length;
}

string Bass::qualifyMacro(string name, unsigned args) {
  if(name.beginswith("#")) name = {"self::", name.ltrim<1>("#")};

  string ns;
  if(auto position = name.position("::")) {
    ns = name.substring(0, position());
    name = name.substring(position() + 2);
  } else {
    ns = activeNamespace;
  }

  if(ns == "self") ns = {"@", macroRecursion() - 1};
  return {ns, "::", name};
}

void Bass::setMacro(string name, const lstring& args, const string& value) {
  name = qualifyMacro(name, args.size());
  macros.insert({name, args, value});
}

string Bass::qualifyLabel(string name) {
  if(name.beginswith("#")) name = {"self::", name.ltrim<1>("#")};

  string ns;
  if(auto position = name.position("::")) {
    ns = name.substring(0, position());
    name = name.substring(position() + 2);
  } else {
    ns = activeNamespace;
  }

  if(name.beginswith(".")) name = {activeLabel, name};
  if(ns == "self") ns = {"@", macroExpandCounter};
  return {ns, "::", name};
}

void Bass::setLabel(string name, unsigned offset) {
  name = qualifyLabel(name);
  if(labels.find(name)) {
    if(pass == 1) error("label has already been declared: ", name);
  }
  labels.insert({name, offset});
}

void Bass::assembleFile(const string& filename) {
  fileName.append(filename);
  lineNumber.append(1);
  blockNumber.append(1);

  if(!file::exists(filename)) error("source file not found: ", filename);
  string source = file::read(filename);
  source.transform("\r\t", "  ");  //treat ignored whitespace characters as spaces
  assembleSource(source);

  fileName.remove();
  lineNumber.remove();
  blockNumber.remove();
}

unsigned Bass::macroRecursion() const {
  return 1 + macroReturnStack.size();
}

optional<string> Bass::assembleMacro(const string& name, const lstring& args) {
  if(const auto& match = macros.find(name)) {
    const auto& macro = match();

    conditionStack.append(Condition::Matching);
    fileName.append({"{macro ", macro.name, "}"});
    lineNumber.append(1);
    blockNumber.append(1);

    for(unsigned n = 0; n < args.size(); n++) {
      string name = {"@", macroRecursion(), "::", macro.args[n]};
      string value = {"return ", args[n]};
      evalMacros(value);
      setMacro(name, {}, value);
    }

    macroExpandCounter++;
    macroReturnStack.append("");
    assembleSource(macro.value);

    fileName.remove();
    lineNumber.remove();
    blockNumber.remove();
    conditionStack.take();

    return {true, macroReturnStack.take()};
  }

  return false;
}

void Bass::assembleSource(const string& source) {
  lstring lines = source.split("\n");
  for(auto& line : lines) {
    if(auto position = line.qposition("//")) line.resize(position());  //strip comments
    if(options.caseInsensitive) line.qlower();

    blockNumber.last() = 1;

    lstring blocks = line.qsplit(";");
    for(auto& block : blocks) {
      if(assembleBlock(block.strip()) == false) error("unknown command: ", block);
      blockNumber.last()++;
    }

    lineNumber.last()++;
  }

  if(fileName.size() == 1) {
    if(conditionStack.size() != 1) error("if without matching endif");
    if(macroDepth) error("macro without matching endmacro");
  }
}

bool Bass::assembleBlock(string& block) {
  //================
  //= conditionals =
  //================
  if(macroDepth == 0) {
    //do not evaluate macros inside unmatched conditional blocks (prevent infinite recursion)
    if((conditionStack.last() == Condition::Matching)
    || (conditionStack.last() == Condition::NotYetMatched && block.wildcard("elseif ?*"))
    ) evalMacros(block);

    if(block.wildcard("if ?*")) {
      if(conditionStack.last() != Condition::Matching) {
        conditionStack.append(Condition::AlreadyMatched);
        return true;
      }
      conditionStack.append(eval(block.ltrim<1>("if ")) ? Condition::Matching : Condition::NotYetMatched);
      return true;
    }

    if(block.wildcard("elseif ?*")) {
      if(conditionStack.last() != Condition::NotYetMatched) {
        conditionStack.last() = Condition::AlreadyMatched;
        return true;
      }
      conditionStack.last() = eval(block.ltrim<1>("elseif ")) ? Condition::Matching : Condition::NotYetMatched;
      return true;
    }

    if(block == "else") {
      if(conditionStack.last() != Condition::NotYetMatched) {
        conditionStack.last() = Condition::AlreadyMatched;
        return true;
      }
      conditionStack.last() = Condition::Matching;
      return true;
    }

    if(block == "endif") {
      conditionStack.take();
      if(conditionStack.size() == 0) error("endif without matching if");
      return true;
    }
  }

  if(conditionStack.last() != Condition::Matching) {
    return true;
  }

  if(!block) {
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
    lstring header = block.split<1>(" "), args;  //header[0] = name, header[1] = params
    if(header(1)) args = header(1).split(",").strip();
    activeMacro.name = header(0);
    activeMacro.args = args;
    activeMacro.value = "";
    macroDepth++;
    return true;
  }

  if(block.beginswith("return")) {
    if(macroReturnStack.size() == 0) error("return statements outside of macros are not allowed");
    block.ltrim<1>("return");
    block.ltrim<1>(" ");  //argument is optional
    macroReturnStack.last() = block;
    return true;
  }

  //===========
  //= defines =
  //===========
  if(block.wildcard("define '?*' ?*")) {
    block.ltrim<1>("define ");
    lstring part = block.qsplit<1>(" ").strip();
    part(0).trim<1>("'");
    if(part(0)[0] == '\\') {
      if(part(0) == "\\\\") part(0) = "\\";
      else if(part(0) == "\\0") part(0) = "\0";
      else if(part(0) == "\\a") part(0) = "\'";
      else if(part(0) == "\\q") part(0) = "\"";
      else if(part(0) == "\\t") part(0) = "\t";
      else if(part(0) == "\\r") part(0) = "\r";
      else if(part(0) == "\\n") part(0) = "\n";
      else warning("unrecognized escape sequence");
    }
    table[part(0)[0]] = eval(part(1));
    return true;
  }

  if(block.wildcard("define ?*")) {
    block.ltrim<1>("define ");
    lstring part = block.split<1>(" ");
    setMacro(part(0), {}, {"return ", part(1)});
    return true;
  }

  if(block.wildcard("eval ?* ?*")) {
    block.ltrim<1>("eval ");
    lstring part = block.split<1>(" ");
    setMacro(part(0), {}, {"return ", eval(part(1))});
    return true;
  }

  //==========
  //= output =
  //==========
  if(block.wildcard("output \"?*\"*")) {
    block.ltrim<1>("output ");
    lstring part = block.qsplit(",").strip();
    part[0].trim<1>("\"");
    FileMode mode = FileMode::Auto;
    if(part(1) == "create") mode = FileMode::Create;
    if(part(1) == "modify") mode = FileMode::Modify;
    if(open({dir(fileName.last()), part[0]}, mode) == false) error("unable to open output file: ", part[0]);
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
    block.trim<1>("incsrc \"", "\"");
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
    if(!fp.open()) error("binary file not found: ", part[0]);
    unsigned offset = 0, length = fp.size() - offset;
    if(part.size() >= 2) offset = eval(part[1]);
    if(part.size() >= 3) length = eval(part[2]);
    if(length > fp.size()) error("binary file include length exceeds file size");
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
    lstring list = block.split(",").strip();
    for(auto& item : list) {
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
        error("unrecognized push argument: ", item);
      }
    }
    return true;
  }

  //========
  //= pull =
  //========
  if(block.wildcard("pull ?*")) {
    block.ltrim<1>("pull ");
    lstring list = block.split(",").strip();
    for(auto& item : list) {
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
        error("unrecognized pull argument: ", item);
      }
    }
    return true;
  }

  //=========
  //= align =
  //=========
  if(block.wildcard("align ?*")) {
    block.ltrim<1>("align ");
    lstring list = block.split(",").strip();
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
    lstring list = block.split(",").strip();
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

  //=========
  //= write =
  //=========
  if(block.wildcard("write \"*\"")) {
    block.ltrim<1>("write ");
    lstring list = block.qsplit(",").strip();
    for(auto& item : list) {
      if(item.wildcard("\"*\"") == false) {
        write(eval(item));
      } else {
        evalText(block);
        for(auto byte : block) write(byte);
      }
    }
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
    lstring list = block.qsplit(",").strip();
    for(auto& item : list) {
      if(item.wildcard("\"*\"") == false) {
        write(eval(item), size);
      } else {
        evalText(item);
        for(auto byte : item) write(table[byte], size);
      }
    }
    return true;
  }

  //============
  //= constant =
  //============
  if(block.wildcard("constant ?* ?*")) {
    block.ltrim<1>("constant ");
    lstring part = block.split<1>(" ").strip();
    setLabel(part(0), eval(part(1)));
    return true;
  }

  //=========
  //= label =
  //=========
  if(block.endswith(":") && !block.position(" ")) {
    string name = block.rtrim<1>(":");
    if(!name.position(".")) activeLabel = string{name}.ltrim<1>("#").split<1>("::").last();
    setLabel(name, pc());
    return true;
  }

  //====================
  //= anonymous labels =
  //====================
  if(block == "-") {
    if(pass == 1) setLabel({"@relativeLast", lastLabelCounter}, pc());
    lastLabelCounter++;
    return true;
  }

  if(block == "+") {
    if(pass == 1) setLabel({"@relativeNext", nextLabelCounter}, pc());
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
    activeLabel = "global";
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

  //=========
  //= print =
  //=========
  if(block.beginswith("print ")) {
    if(pass == 2) {
      block.ltrim<1>("print ");
      print(evalText(block));
    }
    return true;
  }

  //===========
  //= warning =
  //===========
  if(block.wildcard("warning \"*\"")) {
    if(pass == 2) {
      block.ltrim<1>("warning ");
      warning(evalText(block));
    }
    return true;
  }

  //=========
  //= error =
  //=========
  if(block.wildcard("error \"*\"")) {
    if(pass == 2) {
      block.ltrim<1>("error ");
      error(evalText(block));
    }
    return true;
  }

  return false;
}
