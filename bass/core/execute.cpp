bool Bass::execute() {
  macros.reset();
  defines.reset();
  callStack.reset();
  ifStack.reset();
  ip = 0;
  macroInvocationCounter = 0;

  initialize();

  defines.append(coreDefines);

  for(auto& variable : variables) {
    if(!variable.constant) variable.valid = false;
  }

  while(ip < program.size()) {
    Instruction& i = program(ip++);
    if(!executeInstruction(i)) return false;
  }

  defines.remove();
  return true;
}

bool Bass::executeInstruction(Instruction& i) {
  activeInstruction = &i;
  string s = i.statement;
  evaluateDefines(s);

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

  return assemble(s);
}
