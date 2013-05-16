bool Bass::assembleDirective(string& block) {
  //================
  //= conditionals =
  //================
  if(macroDepth == 0) {
    //do not evaluate macros inside unmatched conditional blocks (prevent infinite recursion)
    if((conditionStack.last() == Condition::Matching)
    || (conditionStack.last() == Condition::NotYetMatched && block.match("elseif ?*"))
    ) evalMacros(block);

    if(block.match("if ?*")) {
      if(conditionStack.last() != Condition::Matching) {
        conditionStack.append(Condition::AlreadyMatched);
        return true;
      }
      conditionStack.append(eval(block.ltrim<1>("if ")) ? Condition::Matching : Condition::NotYetMatched);
      return true;
    }

    if(block.match("elseif ?*")) {
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

    if(block.match("while ?*")) {
      if(conditionStack.last() != Condition::Matching) {
        conditionStack.append(Condition::AlreadyMatched);
        return true;
      }
      bool valid = eval(block.ltrim<1>("while "));
      conditionStack.append(valid ? Condition::Matching : Condition::AlreadyMatched);
      if(valid) stack.append({contexts.last().line, ":", contexts.last().block});
      return true;
    }

    if(block == "endwhile") {
      auto result = conditionStack.take();
      if(result == Condition::Matching) {
        lstring offset = stack.take().split<1>(":");
        contexts.last().line = decimal(offset(0));
        contexts.last().block = decimal(offset(1));
        sync = true;
      }
      if(conditionStack.size() == 0) error("endwhile without matching while");
      return true;
    }
  }

  if(conditionStack.last() != Condition::Matching) {
    return true;
  }

  if(!block) {
    return true;
  }

  //=============
  //= functions =
  //=============
  if(block == "endfunction") {
    if(macroDepth == 0) error("endfunction without matching function");
    if(--macroDepth) {
      activeMacro.value.append(block, "; ");
      return true;
    }
    activeMacro.value.rtrim<1>("; ");
    setMacro(activeMacro.name, activeMacro.args, activeMacro.value);
    return true;
  }

  if(macroDepth) {
    if(block.beginswith("function ")) macroDepth++;
    activeMacro.value.append(block, "; ");
    return true;
  }

  if(block.beginswith("function ")) {
    block.ltrim<1>("function ");
    lstring part = block.split<1>(" ");
    lstring args = part(1) ? part(1).split(",").strip() : lstring{};
    activeMacro.name = part(0);
    activeMacro.args = args;
    activeMacro.value = "";
    macroDepth++;
    return true;
  }

  if(block.beginswith("return")) {
    if(macroReturnStack.size() == 0) error("return statements outside of functions are not allowed");
    block.ltrim<1>("return");
    block.ltrim<1>(" ");  //argument is optional
    macroReturnStack.last() = block;
    return true;
  }

  //===========
  //= defines =
  //===========
  if(block.match("define '?*' ?*")) {
    block.ltrim<1>("define ");
    lstring part = block.qsplit<1>(" ").strip();
    char value = eval(part(0));
    table[value] = eval(part(1));
    return true;
  }

  if(block.match("define ?*")) {
    block.ltrim<1>("define ");
    lstring part = block.split<1>(" ");
    setMacro(part(0), {}, {"return ", part(1)});
    return true;
  }

  if(block.match("eval ?* ?*")) {
    block.ltrim<1>("eval ");
    lstring part = block.split<1>(" ");
    setMacro(part(0), {}, {"return ", eval(part(1))});
    return true;
  }

  //==========
  //= output =
  //==========
  if(block.match("output \"?*\"*")) {
    block.ltrim<1>("output ");
    lstring part = block.qsplit(",").strip();
    part[0].trim<1>("\"");
    FileMode mode = FileMode::Auto;
    if(part(1) == "create") mode = FileMode::Create;
    if(part(1) == "modify") mode = FileMode::Modify;
    if(open({dir(sourceFiles.last()), part[0]}, mode) == false) error("unable to open output file: ", part[0]);
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
  if(block.match("incsrc \"?*\"")) {
    block.trim<1>("incsrc \"", "\"");
    assembleFile({dir(sourceFiles.last()), block});
    return true;
  }

  //============
  //= incbinas =
  //============
  string incbinName;
  if(block.match("incbinas ?*")) {
    block.ltrim<1>("incbinas ");
    lstring part = block.qsplit(",").strip();
    incbinName = part.take(0);

    //fallthrough to incbin
    block = {"incbin ", part.concatenate(",")};
  }

  //==========
  //= incbin =
  //==========
  if(block.match("incbin \"?*\"*")) {
    block.ltrim<1>("incbin ");
    lstring part = block.qsplit(",").strip();
    part[0].trim<1>("\"");
    file fp({dir(sourceFiles.last()), part[0]}, file::mode::read);
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
  if(block.match("origin ?*")) {
    block.ltrim<1>("origin ");
    origin = eval(block);
    seek(origin);
    return true;
   }

  //========
  //= base =
  //========
  if(block.match("base ?*")) {
    block.ltrim<1>("base ");
    base = eval(block) - origin;
    return true;
  }

  //========
  //= push =
  //========
  if(block.match("push ?*")) {
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
      } else if(item.match("\"*\"")) {
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
  if(block.match("pull ?*")) {
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
  if(block.match("align ?*")) {
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
  if(block.match("fill ?*")) {
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
  if(block.match("seek ?*")) {
    block.ltrim<1>("seek ");
    origin += eval(block);
    seek(origin);
    return true;
  }

  //=========
  //= write =
  //=========
  if(block.match("write \"*\"")) {
    block.ltrim<1>("write ");
    lstring list = block.qsplit(",").strip();
    for(auto& item : list) {
      if(item.match("\"*\"") == false) {
        write(eval(item));
      } else {
        item = decodeText(item);
        for(auto byte : item) write(byte);
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
      if(item.match("\"*\"") == false) {
        write(eval(item), size);
      } else {
        item = decodeText(item);
        for(auto byte : item) write(table[byte], size);
      }
    }
    return true;
  }

  //============
  //= constant =
  //============
  if(block.match("constant ?* ?*")) {
    block.ltrim<1>("constant ");
    lstring part = block.split<1>(" ").strip();
    setLabel(part(0), eval(part(1)));
    return true;
  }

  //=========
  //= label =
  //=========
  if(block.endswith(":") && !block.find(" ")) {
    string name = block.rtrim<1>(":");
    if(!name.find(".")) activeLabel = string{name}.ltrim<1>("#").split<1>("::").last();
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
  if(block.match("namespace ?*")) {
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
      lstring list = block.qsplit(",").strip();
      for(auto& item : list) {
        if(item.match("\"*\"") == false) {
          print(eval(item));
        } else {
          print(decodeText(item));
        }
      }
    }
    return true;
  }

  //===========
  //= warning =
  //===========
  if(block.match("warning \"*\"")) {
    if(pass == 2) {
      block.ltrim<1>("warning ");
      warning(decodeText(block));
    }
    return true;
  }

  //=========
  //= error =
  //=========
  if(block.match("error \"*\"")) {
    if(pass == 2) {
      block.ltrim<1>("error ");
      error(decodeText(block));
    }
    return true;
  }

  return false;
}
