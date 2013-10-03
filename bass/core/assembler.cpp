void Bass::assembleInitialize() {
  stack.reset();
  scope.reset();
  for(unsigned n = 0; n < 256; n++) stringTable[n] = n;
  endian = Endian::LSB;
  origin = 0;
  base = 0;
  lastLabelCounter = 1;
  nextLabelCounter = 1;
}

bool Bass::assemblePhase() {
  ip = 0;
  for(auto& variable : variables) {
    if(!variable.constant) variable.valid = false;
  }
  while(ip < instructions.size()) {
    Instruction& i = instructions(ip++);
    if(!assembleInstruction(i)) error("unrecognized directive: ", i.statement);
  }
  return true;
}

bool Bass::assembleInstruction(Instruction& i) {
  string s = i.statement;

  //scope name {
  if(s.match("scope ?* {")) {
    s.trim<1>("scope ", " {").strip();
    if(s.endswith(":")) setVariable(s.rtrim<1>(":"), pc(), true);
    scope.append(s);
    return true;
  }

  //}
  if(s.match("} endscope")) {
    scope.remove();
    return true;
  }

  //label: or label: {
  if(s.match("?*:") || s.match("?*: {")) {
    s.rtrim<1>(" {");
    s.rtrim<1>(":");
    setVariable(s, pc(), true);
    return true;
  }

  //- or - {
  if(s.match("-") || s.match("- {")) {
    setVariable({"lastLabel#", lastLabelCounter++}, pc(), true);
    return true;
  }

  //+ or + {
  if(s.match("+") || s.match("+ {")) {
    setVariable({"nextLabel#", nextLabelCounter++}, pc(), true);
    return true;
  }

  //}
  if(s.match("} endconstant")) {
    return true;
  }

  //variable name(value)
  if(s.match("variable ?*(*)")) {
    lstring p = s.trim<1>("variable ", ")").split<1>("(");
    setVariable(p(0), evaluate(p(1)));
    return true;
  }

  //constant name(value)
  if(s.match("constant ?*(*)")) {
    lstring p = s.trim<1>("constant ", ")").split<1>("(");
    setVariable(p(0), evaluate(p(1)), true);
    return true;
  }

  //output "filename" [, create]
  if(s.match("output ?*")) {
    lstring p = s.ltrim<1>("output ").qsplit(",").strip();
    string filename = {filepath(i), p.take(0).trim<1>("\"")};
    bool create = (p.size() && p(0) == "create");
    target(filename, create);
    return true;
  }

  //endian (lsb|msb)
  if(s.match("endian ?*")) {
    s.ltrim<1>("endian ");
    if(s == "lsb") { endian = Endian::LSB; return true; }
    if(s == "msb") { endian = Endian::MSB; return true; }
    error("invalid endian mode");
  }

  //origin offset
  if(s.match("origin ?*")) {
    s.ltrim<1>("origin ");
    origin = evaluate(s);
    seek(origin);
    return true;
  }

  //base offset
  if(s.match("base ?*")) {
    s.ltrim<1>("base ");
    base = evaluate(s);
    return true;
  }

  //seek displacement
  if(s.match("seek ?*")) {
    signed offset = evaluate(s.ltrim<1>("seek "));
    origin += offset;
    seek(origin);
    return true;
  }

  //push variable [, ...]
  if(s.match("push ?*")) {
    lstring p = s.ltrim<1>("push ").qsplit(",").strip();
    for(auto& t : p) {
      if(t == "origin") {
        stack.append(origin);
      } else if(t == "base") {
        stack.append(base);
      } else if(t == "pc") {
        stack.append(origin);
        stack.append(base);
      } else {
        error("unrecognized push variable: ", t);
      }
    }
    return true;
  }

  //pull variable [, ...]
  if(s.match("pull ?*")) {
    lstring p = s.ltrim<1>("pull ").qsplit(",").strip();
    for(auto& t : p) {
      if(t == "origin") {
        origin = decimal(stack.take());
        seek(origin);
      } else if(t == "base") {
        base = integer(stack.take());
      } else if(t == "pc") {
        base = integer(stack.take());
        origin = decimal(stack.take());
        seek(origin);
      } else {
        error("unrecognized pull variable: ", t);
      }
    }
    return true;
  }

  //insert [name, ] filename [, offset] [, length]
  if(s.match("insert ?*")) {
    lstring p = s.ltrim<1>("insert ").qsplit(",").strip();
    string name;
    if(!p(0).match("\"*\"")) name = p.take(0);
    string filename = {filepath(i), p.take(0).trim<1>("\"")};
    file fp;
    if(!fp.open(filename, file::mode::read)) error("file not found: ", filename);
    unsigned offset = p.size() ? evaluate(p.take(0)) : 0;
    if(offset > fp.size()) offset = fp.size();
    unsigned length = p.size() ? evaluate(p.take(0)) : 0;
    if(length == 0) length = fp.size() - offset;
    if(name) {
      setVariable({name}, pc(), true);
      setVariable({name, ".size"}, length, true);
    }
    fp.seek(offset);
    while(!fp.end() && length--) write(fp.read());
    return true;
  }

  //align to [, with]
  if(s.match("align ?*")) {
    lstring p = s.ltrim<1>("align ").qsplit(",").strip();
    unsigned align = evaluate(p(0));
    unsigned byte = evaluate(p(1, "0"));
    if(align == 0) return true;
    while(pc() % align) write(byte);
    return true;
  }

  //fill length [, with]
  if(s.match("fill ?*")) {
    lstring p = s.ltrim<1>("fill ").qsplit(",").strip();
    unsigned length = evaluate(p(0));
    unsigned byte = evaluate(p(1, "0"));
    while(length--) write(byte);
    return true;
  }

  //table.reset
  if(s.match("table.reset")) {
    for(unsigned n = 0; n < 256; n++) stringTable[n] = n;
    return true;
  }

  //table.assign 'char' [, value] [, length]
  if(s.match("table.assign ?*")) {
    lstring p = s.ltrim<1>("table.assign ").qsplit(",").strip();
    uint8_t index = evaluate(p(0));
    int64_t value = evaluate(p(1, "0"));
    int64_t length = evaluate(p(2, "1"));
    for(signed n = 0; n < length; n++) {
      stringTable[index + n] = value + n;
    }
    return true;
  }

  //d[bwldq] ("string"|variable) [, ...]
  unsigned dataLength = 0;
  if(s.beginswith("db ")) dataLength = 1;
  if(s.beginswith("dw ")) dataLength = 2;
  if(s.beginswith("dl ")) dataLength = 3;
  if(s.beginswith("dd ")) dataLength = 4;
  if(s.beginswith("dq ")) dataLength = 8;
  if(dataLength) {
    s = s.slice(3);  //remove prefix
    lstring p = s.qsplit(",").strip();
    for(auto& t : p) {
      if(t.match("\"*\"")) {
        t = text(t.trim<1>("\""));
        for(auto& b : t) write(stringTable[b], dataLength);
      } else {
        write(evaluate(t), dataLength);
      }
    }
    return true;
  }

  //print ("string"|variable) [, ...]
  if(s.match("print ?*")) {
    s.ltrim<1>("print ");
    if(writePhase()) {
      lstring p = s.qsplit(",").strip();
      for(auto& t : p) {
        if(t.match("\"*\"")) {
          t = text(t.trim<1>("\""));
          print(t);
        } else {
          print(evaluate(t));
        }
      }
    }
    return true;
  }

  //warning "string"
  if(s.match("warning \"*\"")) {
    if(writePhase()) {
      string message = s.trim<1>("warning \"", "\"");
      warning(text(message));
    }
    return true;
  }

  //error "string"
  if(s.match("error \"*\"")) {
    if(writePhase()) {
      string message = s.trim<1>("error \"", "\"");
      error(text(message));
    }
    return true;
  }

  return false;
}

string Bass::text(string s) {
  s.replace("\\n", "\n");
  s.replace("\\q", "\"");
  s.replace("\\\\", "\\");
  return s;
}

optional<int64_t> Bass::findVariable(const string& name) {
  lstring s = scope;
  while(true) {
    string scopedName = {s.merge("."), s.size() ? "." : "", name};
    if(auto variable = variables.find({scopedName})) {
      if(variable().valid) return {true, variable().value};
    }
    if(s.empty()) break;
    s.remove();
  }
  return false;
}

int64_t Bass::getVariable(const string& name) {
  if(auto variable = findVariable(name)) return variable();
  if(queryPhase()) return pc();
  error("variable not found: ", name);
}

void Bass::setVariable(const string& name, int64_t value, bool constant) {
  string scopedName = name;
  if(scope.size()) scopedName = {scope.merge("."), ".", name};

  if(auto variable = variables.find({scopedName})) {
    if(!writePhase() && variable().constant) error("constant cannot be modified: ", scopedName);
    variable().value = value;
    variable().valid = true;
  } else {
    variables.insert({scopedName, value, constant});
  }
}
