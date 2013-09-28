bool Bass::preprocessAnalyze() {
  unsigned ip = 0;
  while(ip < program.size()) {
    Instruction& i = program(ip++);
    string s = i.statement;

    if(s.match("function ?* {")) {
      s.trim<1>("function ", " {");
      blockStack.append({"function", ip - 1});
      i.statement = {s, ":"};
      continue;
    }

    if(s.match("}") && blockStack.last().type == "function") {
      blockStack.remove();
      i.statement = "} endfunction";
      continue;
    }

    if(s.match("macro ?*(*) {")) {
      blockStack.append({"macro", ip - 1});
      continue;
    }

    if(s.match("}") && blockStack.last().type == "macro") {
      unsigned rp = blockStack.last().ip;
      program[rp].ip[0] = ip;
      blockStack.remove();
      i.statement = "} endmacro";
      continue;
    }

    if(s.match("if ?* {")) {
      s.trim<1>("if ", " {");
      blockStack.append({"if", ip - 1});
      i.expression = Eval::parse(s);
      continue;
    }

    if(s.match("} else if ?* {")) {
      s.trim<1>("} else if ", " {");
      unsigned rp = blockStack.last().ip;
      program[rp].ip[0] = ip - 1;
      blockStack.last().ip = ip - 1;
      i.expression = Eval::parse(s);
      continue;
    }

    if(s.match("} else {")) {
      unsigned rp = blockStack.last().ip;
      program[rp].ip[0] = ip - 1;
      blockStack.last().ip = ip - 1;
      continue;
    }

    if(s.match("}") && blockStack.last().type == "if") {
      unsigned rp = blockStack.last().ip;
      program[rp].ip[0] = ip - 1;
      blockStack.remove();
      i.statement = "} endif";
      continue;
    }

    if(s.match("while ?* {")) {
      s.trim<1>("while ", " {");
      blockStack.append({"while", ip - 1});
      i.expression = Eval::parse(s);
      continue;
    }

    if(s.match("}") && blockStack.last().type == "while") {
      unsigned rp = blockStack.last().ip;
      program[rp].ip[0] = ip;
      blockStack.remove();
      i.statement = "} endwhile";
      i.ip[0] = rp;
      continue;
    }

    if(s.match("?* := ?*")) {
      lstring p = s.split<1>(" := ");
      i.statement = {"variable ", p(0)};
      i.expression = Eval::parse(p(1));
      continue;
    }
  }

  return true;
}

bool Bass::preprocessExecute() {
  while(ip < program.size()) {
    Instruction& i = program(ip++);

    string s = i.statement;
    preprocessDefines(s);

    if(s.match("?*(*)")) {
      lstring p = string{s}.rtrim<1>(")").split<1>("(");
      lstring a = p(1).empty() ? lstring{} : p(1).qsplit(",").strip();
      string name = {p(0), ":", a.size()};
      if(auto macro = macros.find({name})) {
        hashset<Define>& context = contexts(contexts.size());
        for(unsigned n = 0; n < a.size(); n++) {
          Define define;
          define.name = macro().parameters(n);
          define.value = a(n);
          context.insert(define);
        }

        callStack.append(ip);
        ip = macro().ip;
        macroInvocationCounter++;
        continue;
      }
    }

    if(s.match("} endfunction")) {
      continue;
    }

    if(s.match("macro ?*(*) {")) {
      s.trim<1>("macro ", ") {");
      lstring p = s.split<1>("(");
      lstring a = p(1).empty() ? lstring{} : p(1).qsplit(",").strip();
      string name = {p(0), ":", a.size()};

      if(auto macro = macros.find({name})) {
        macro().parameters = a;
        macro().ip = ip;
      } else {
        Macro m;
        m.name = {p(0), ":", a.size()};
        m.parameters = a;
        m.ip = ip;
        macros.insert(m);
      }

      ip = i.ip[0];
      continue;
    }

    if(s.match("} endmacro")) {
      ip = callStack.take();
      contexts.remove();
      continue;
    }

    if(s.match("define ?*(*)")) {
      s.trim<1>("define ", ")");
      lstring p = s.split<1>("(");

      Define define;
      define.name = p(0);
      define.value = p(1);
      defines.insert(define);
      continue;
    }

    if(s.match("if ?* {")) {
      bool match = evaluate(i.expression);
      ifStack.append(match);
      if(match == false) {
        ip = i.ip[0];
      }
      continue;
    }

    if(s.match("} else if ?* {")) {
      if(ifStack.last()) {
        ip = i.ip[0];
      } else {
        bool match = evaluate(i.expression);
        ifStack.last() = match;
        if(match == false) {
          ip = i.ip[0];
        }
      }
      continue;
    }

    if(s.match("} else {")) {
      if(ifStack.last()) {
        ip = i.ip[0];
      } else {
        ifStack.last() = true;
      }
      continue;
    }

    if(s.match("} endif")) {
      ifStack.remove();
      continue;
    }

    if(s.match("while ?* {")) {
      bool match = evaluate(i.expression);
      if(match == false) ip = i.ip[0];
      continue;
    }

    if(s.match("} endwhile")) {
      ip = i.ip[0];
      continue;
    }

    if(s.match("variable ?*")) {
      s.ltrim<1>("variable ");
      if(auto variable = variables.find({s})) {
        variable().value = evaluate(i.expression);
      } else {
        Variable v;
        v.name = s;
        v.value = evaluate(i.expression);
        variables.insert(v);
      }
      continue;
    }

    instructions.append(i);
    instructions.last().statement = s;
  }

  return true;
}

void Bass::preprocessDefines(string& s) {
  unsigned depth = 0, maxDepth = 0;
  for(unsigned x = 0; x < s.size(); x++) {
    if(s[x] == '{') depth++;
    if(s[x] == '}') depth--;
    if(depth >= maxDepth) maxDepth = depth;
  }

  for(unsigned x = 0, depth = 0; x < s.size(); x++) {
    if(s[x] == '{') depth++;
    if(s[x] == '}') depth--;
    if(s[x] == '{' && depth == maxDepth) {
      signed counter = 0;
      for(unsigned y = x; y < s.size(); y++) {
        if(s[y] == '{') counter++;
        if(s[y] == '}') counter--;
        if(s[y] == '}' && counter == 0) {
          string name = s.slice(x + 1, y - x - 1);
          if(name == "#") {
            s = {s.slice(0, x), macroInvocationCounter, s.slice(y + 1)};
            return preprocessDefines(s);
          }
          if(contexts.size()) {
            if(auto define = contexts.last().find({name})) {
              s = {s.slice(0, x), define().value, s.slice(y + 1)};
              return preprocessDefines(s);
            }
          }
          if(auto define = defines.find({name})) {
            s = {s.slice(0, x), define().value, s.slice(y + 1)};
            return preprocessDefines(s);
          }
        }
      }
    }
  }
}
