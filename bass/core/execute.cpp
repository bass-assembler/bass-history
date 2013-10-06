bool Bass::execute() {
  defines.reset();
  macros.reset();
  callStack.reset();
  ifStack.reset();
  ip = 0;
  macroInvocationCounter = 0;

  initialize();

  defines.append(coreDefines);

  while(ip < program.size()) {
    Instruction& i = program(ip++);
    if(!executeInstruction(i)) error("unrecognized directive: ", i.statement);
  }

  defines.remove();
  return true;
}

bool Bass::executeInstruction(Instruction& i) {
  activeInstruction = &i;
  string s = i.statement;
  evaluateDefines(s);

  if(s.match("define ?*(*)")) {
    lstring p = s.trim<1>("define ", ")").split<1>("(");
    setDefine(p(0), p(1));
    return true;
  }

  if(s.match("evaluate ?*(*)")) {
    lstring p = s.trim<1>("evaluate ", ")").split<1>("(");
    setDefine(p(0), evaluate(p(1)));
    return true;
  }

  if(s.match("block {")) return true;
  if(s.match("} endblock")) return true;

  if(s.match("macro ?*(*) {") || s.match("macro ?*(*): {")) {
    bool scoped = s.endswith("): {");
    s.trim<1>("macro ", scoped ? "): {" : ") {");
    lstring p = s.split<1>("(");
    lstring a = p(1).empty() ? lstring{} : p(1).qsplit(",").strip();
    setMacro(p(0), a, ip, scoped);
    ip = i.ip;
    return true;
  }

  if(s.match("if ?* {")) {
    s.trim<1>("if ", " {").strip();
    bool match = evaluate(s, Evaluation::Strict);
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
      bool match = evaluate(s, Evaluation::Strict);
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
    bool match = evaluate(s, Evaluation::Strict);
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
    if(auto macro = findMacro({name})) {
      hashset<Define> stack;
      defines.append(stack);
      setDefine("#", macroInvocationCounter++);
      for(unsigned n = 0; n < a.size(); n++) {
        setDefine(macro().parameters(n), a(n));
      }

      callStack.append({ip, macro().scoped});
      ip = macro().ip;
      if(macro().scoped) scope.append(p(0));
      return true;
    }
  }

  if(s.match("} endmacro")) {
    ip = callStack.last().ip;
    if(callStack.last().scoped) scope.remove();
    callStack.remove();
    defines.remove();
    return true;
  }

  return assemble(s);
}
