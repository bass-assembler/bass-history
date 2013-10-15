void Bass::setMacro(const string& name, const lstring& parameters, unsigned ip, bool scoped, bool local) {
  if(stackFrame.size() == 0) return;
  auto& macros = stackFrame[local ? stackFrame.size() - 1 : 0].macros;

  string scopedName = {name, ":", parameters.size()};
  if(scope.size()) scopedName = {scope.merge("."), ".", scopedName};

  if(auto macro = macros.find({scopedName})) {
    macro().parameters = parameters;
    macro().ip = ip;
    macro().scoped = scoped;
  } else {
    macros.insert({scopedName, parameters, ip, scoped});
  }
}

optional<Bass::Macro&> Bass::findMacro(const string& name, bool local) {
  if(stackFrame.size() == 0) return false;
  auto& macros = stackFrame[local ? stackFrame.size() - 1 : 0].macros;

  lstring s = scope;
  while(true) {
    string scopedName = {s.merge("."), s.size() ? "." : "", name};
    if(auto macro = macros.find({scopedName})) {
      return {true, macro()};
    }
    if(s.empty()) break;
    s.removelast();
  }

  return false;
}

optional<Bass::Macro&> Bass::findMacro(const string& name) {
  if(auto macro = findMacro(name, true)) return macro();
  if(auto macro = findMacro(name, false)) return macro();
  return false;
}

void Bass::setDefine(const string& name, const string& value, bool local) {
  if(stackFrame.size() == 0) return;
  auto& defines = stackFrame[local ? stackFrame.size() - 1 : 0].defines;

  string scopedName = name;
  if(scope.size()) scopedName = {scope.merge("."), ".", name};

  if(auto define = defines.find({scopedName})) {
    define().value = value;
  } else {
    defines.insert({scopedName, value});
  }
}

optional<Bass::Define&> Bass::findDefine(const string& name, bool local) {
  if(stackFrame.size() == 0) return false;
  auto& defines = stackFrame[local ? stackFrame.size() - 1 : 0].defines;

  lstring s = scope;
  while(true) {
    string scopedName = {s.merge("."), s.size() ? "." : "", name};
    if(auto define = defines.find({scopedName})) {
      return {true, define()};
    }
    if(s.empty()) break;
    s.removelast();
  }

  return false;
}

optional<Bass::Define&> Bass::findDefine(const string& name) {
  if(auto define = findDefine(name, true)) return define();
  if(auto define = findDefine(name, false)) return define();
  return false;
}

void Bass::setVariable(const string& name, int64_t value, bool local) {
  if(stackFrame.size() == 0) return;
  auto& variables = stackFrame[local ? stackFrame.size() - 1 : 0].variables;

  string scopedName = name;
  if(scope.size()) scopedName = {scope.merge("."), ".", name};

  if(auto variable = variables.find({scopedName})) {
    variable().value = value;
  } else {
    variables.insert({scopedName, value});
  }
}

optional<Bass::Variable&> Bass::findVariable(const string& name, bool local) {
  if(stackFrame.size() == 0) return false;
  auto& variables = stackFrame[local ? stackFrame.size() - 1 : 0].variables;

  lstring s = scope;
  while(true) {
    string scopedName = {s.merge("."), s.size() ? "." : "", name};
    if(auto variable = variables.find({scopedName})) {
      return {true, variable()};
    }
    if(s.empty()) break;
    s.removelast();
  }

  return false;
}

optional<Bass::Variable&> Bass::findVariable(const string& name) {
  if(auto variable = findVariable(name, true)) return variable();
  if(auto variable = findVariable(name, false)) return variable();
  return false;
}

void Bass::setConstant(const string& name, int64_t value) {
  string scopedName = name;
  if(scope.size()) scopedName = {scope.merge("."), ".", name};

  if(auto constant = constants.find({scopedName})) {
    if(queryPhase()) error("constant cannot be modified: ", scopedName);
    constant().value = value;
  } else {
    constants.insert({scopedName, value});
  }
}

optional<Bass::Constant&> Bass::findConstant(const string& name) {
  lstring s = scope;
  while(true) {
    string scopedName = {s.merge("."), s.size() ? "." : "", name};
    if(auto constant = constants.find({scopedName})) {
      return {true, constant()};
    }
    if(s.empty()) break;
    s.removelast();
  }

  return false;
}

void Bass::evaluateDefines(string& s) {
  for(signed x = s.size() - 1, y = -1; x >= 0; x--) {
    if(s[x] == '}') y = x;
    if(s[x] == '{' && y > x) {
      string name = s.slice(x + 1, y - x - 1);

      if(name.match("defined ?*")) {
        name.ltrim<1>("defined ").strip();
        s = {s.slice(0, x), findDefine(name) ? 1 : 0, s.slice(y + 1)};
        return evaluateDefines(s);
      }

      if(auto define = findDefine(name)) {
        s = {s.slice(0, x), define().value, s.slice(y + 1)};
        return evaluateDefines(s);
      }
    }
  }
}

string Bass::filepath() {
  return dir(sourceFilenames[activeInstruction->fileNumber]);
}

string Bass::text(string s) {
  if(!s.match("\"*\"")) warning("string value is unqouted: ", s);
  s.trim<1>("\"");
  s.replace("\\n", "\n");
  s.replace("\\q", "\"");
  s.replace("\\\\", "\\");
  return s;
}

int64_t Bass::character(string s) {
  if(s[0] != '\'') goto unknown;
  if(s[2] == '\'') return s[1];
  if(s[3] != '\'') goto unknown;
  if(s[1] != '\\') goto unknown;
  if(s[2] == 'n') return '\n';
  if(s[2] == 'q') return '\"';
  if(s[2] == '\\') return '\\';

unknown:
  warning("unrecognized character constant: ", s);
  return 0;
}
