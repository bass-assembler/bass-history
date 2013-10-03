bool Bass::preprocessAnalyze() {
  ip = 0;
  while(ip < program.size()) {
    Instruction& i = program(ip++);
    if(!preprocessAnalyzeInstruction(i)) error("unrecognized preprocessor directive: ", i.statement);
  }
  return true;
}

bool Bass::preprocessAnalyzeInstruction(Instruction& i) {
  string s = i.statement;

  if(s.match("}") && blockStack.empty()) error("} without matching {");

  if(s.match("{")) {
    blockStack.append({"block", ip - 1});
    i.statement = "block {";
    return true;
  }

  if(s.match("}") && blockStack.last().type == "block") {
    blockStack.remove();
    i.statement = "} endblock";
    return true;
  }

  if(s.match("scope ?* {")) {
    blockStack.append({"scope", ip - 1});
    return true;
  }

  if(s.match("}") && blockStack.last().type == "scope") {
    blockStack.remove();
    i.statement = "} endscope";
    return true;
  }

  if(s.match("?*: {") || s.match("- {") || s.match("+ {")) {
    blockStack.append({"constant", ip - 1});
    return true;
  }

  if(s.match("}") && blockStack.last().type == "constant") {
    blockStack.remove();
    i.statement = "} endconstant";
    return true;
  }

  if(s.match("macro ?*(*) {")) {
    blockStack.append({"macro", ip - 1});
    return true;
  }

  if(s.match("}") && blockStack.last().type == "macro") {
    unsigned rp = blockStack.last().ip;
    program[rp].ip = ip;
    blockStack.remove();
    i.statement = "} endmacro";
    return true;
  }

  if(s.match("if ?* {")) {
    s.trim<1>("if ", " {");
    blockStack.append({"if", ip - 1});
    return true;
  }

  if(s.match("} else if ?* {")) {
    s.trim<1>("} else if ", " {");
    unsigned rp = blockStack.last().ip;
    program[rp].ip = ip - 1;
    blockStack.last().ip = ip - 1;
    return true;
  }

  if(s.match("} else {")) {
    unsigned rp = blockStack.last().ip;
    program[rp].ip = ip - 1;
    blockStack.last().ip = ip - 1;
    return true;
  }

  if(s.match("}") && blockStack.last().type == "if") {
    unsigned rp = blockStack.last().ip;
    program[rp].ip = ip - 1;
    blockStack.remove();
    i.statement = "} endif";
    return true;
  }

  if(s.match("while ?* {")) {
    s.trim<1>("while ", " {");
    blockStack.append({"while", ip - 1});
    return true;
  }

  if(s.match("}") && blockStack.last().type == "while") {
    unsigned rp = blockStack.last().ip;
    program[rp].ip = ip;
    blockStack.remove();
    i.statement = "} endwhile";
    i.ip = rp;
    return true;
  }

  return true;
}

bool Bass::preprocessExecute() {
  set<Define> stack;
  defines.append(stack);

  ip = 0;
  while(ip < program.size()) {
    Instruction& i = program(ip++);
    if(!preprocessExecuteInstruction(i)) return false;
  }

  defines.remove();
  return true;
}

bool Bass::preprocessExecuteInstruction(Instruction& i) {
  string s = i.statement;
  preprocessDefines(s);

  if(s.match("evaluate ?*(*)")) {
    lstring p = s.trim<1>("evaluate ", ")").split<1>("(");
    setDefine(p(0), evaluate(p(1)));
    return true;
  }

  if(s.match("define ?*(*)")) {
    lstring p = s.trim<1>("define ", ")").split<1>("(");
    setDefine(p(0), p(1));
    return true;
  }

  if(s.match("block {")) return true;
  if(s.match("} endblock")) return true;

  if(s.match("macro ?*(*) {")) {
    s.trim<1>("macro ", ") {");
    lstring p = s.split<1>("(");
    lstring a = p(1).empty() ? lstring{} : p(1).qsplit(",").strip();
    setMacro(p(0), a, ip);
    ip = i.ip;
    return true;
  }

  if(s.match("if ?* {")) {
    s.trim<1>("if ", " {").strip();
    bool match = evaluate(s);
    ifStack.append(match);
    if(match == false) {
      ip = i.ip;
    }
    return true;
  }

  if(s.match("} else if ?* {")) {
    if(ifStack.last()) {
      ip = i.ip;
    } else {
      s.trim<1>("} else if ", " {").strip();
      bool match = evaluate(s);
      ifStack.last() = match;
      if(match == false) {
        ip = i.ip;
      }
    }
    return true;
  }

  if(s.match("} else {")) {
    if(ifStack.last()) {
      ip = i.ip;
    } else {
      ifStack.last() = true;
    }
    return true;
  }

  if(s.match("} endif")) {
    ifStack.remove();
    return true;
  }

  if(s.match("while ?* {")) {
    s.trim<1>("while ", " {").strip();
    bool match = evaluate(s);
    if(match == false) ip = i.ip;
    return true;
  }

  if(s.match("} endwhile")) {
    ip = i.ip;
    return true;
  }

  if(s.match("?*(*)")) {
    lstring p = string{s}.rtrim<1>(")").split<1>("(");
    lstring a = p(1).empty() ? lstring{} : p(1).qsplit(",").strip();
    string name = {p(0), ":", a.size()};
    if(auto macro = macros.find({name})) {
      set<Define> stack;
      defines.append(stack);
      setDefine("#", macroInvocationCounter++);
      for(unsigned n = 0; n < a.size(); n++) {
        setDefine(macro().parameters(n), a(n));
      }

      callStack.append(ip);
      ip = macro().ip;
      return true;
    }
  }

  if(s.match("} endmacro")) {
    ip = callStack.take();
    defines.remove();
    return true;
  }

  instructions.append(i);
  instructions.last().statement = s;
  return true;
}

void Bass::preprocessDefines(string& s) {
  for(signed x = s.size() - 1, y = -1; x >= 0; x--) {
    if(s[x] == '}') y = x;
    if(s[x] == '{' && y > x) {
      string name = s.slice(x + 1, y - x - 1);
      for(signed n = defines.size() - 1; n >= 0; n--) {
        if(auto define = defines[n].find({name})) {
          s = {s.slice(0, x), define().value, s.slice(y + 1)};
          return preprocessDefines(s);
        }
      }
    }
  }
}

void Bass::setMacro(const string& name, const lstring& parameters, unsigned ip) {
  string overloadName = {name, ":", parameters.size()};
  if(auto macro = macros.find({overloadName})) {
    macro().parameters = parameters;
    macro().ip = ip;
  } else {
    macros.insert({overloadName, parameters, ip});
  }
}

void Bass::setDefine(const string& name, const string& value) {
  for(signed n = defines.size() - 1; n >= 0; n--) {
    if(auto define = defines[n].find({name})) {
      define().value = value;
      return;
    }
  }
  defines.last().insert({name, value});
}
