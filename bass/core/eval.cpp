int64_t Bass::eval(const string &s) {
  int offset = 0;
  if(s == "-") offset = -1; if(s == "--") offset = -2; if(s == "---") offset = -3;
  if(s == "+") offset = +1; if(s == "++") offset = +2; if(s == "+++") offset = +3;
  if(offset) {
    string name = offset < 0
    ? string("-", negativeLabelCounter + offset)
    : string("+", positiveLabelCounter + offset - 1);
    foreach(label, labels) if(name == label.name) return label.offset;
    if(pass == 1) return pc();
    error("eval:unmatched - label");
  }

  nall::eval_fallback = [this](const char *&s) -> int64_t {
    //hexadecimal
    if(*s == '$') {
      s++;
      int64_t value = 0;
      while(true) {
        if(*s >= '0' && *s <= '9') { value = value * 16 + (*s++ - '0'     ); continue; }
        if(*s >= 'A' && *s <= 'F') { value = value * 16 + (*s++ - 'A' + 10); continue; }
        if(*s >= 'a' && *s <= 'f') { value = value * 16 + (*s++ - 'a' + 10); continue; }
        return value;
      }
    }

    //binary
    if(*s == '%') {
      s++;
      int64_t value = 0;
      while(true) {
        if(*s == '0' || *s == '1') { value = value * 2 + (*s++ - '0'); continue; }
        return value;
      }
    }

    //label
    if(*s == ':' || *s == '.' || *s == '_' || (*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z')) {
      const char *start = s;
      while(*s == ':' || *s == '.' || *s == '_' || (*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z') || (*s >= '0' && *s <= '9')) s++;
      string name = substr(start, 0, s - start);
      if(name.beginswith(".")) name = string(activeLabel, name);
      if(!name.position("::")) name = string(activeNamespace, "::", name);
      foreach(label, labels) if(name == label.name) return label.offset;
      if(pass == 1) return pc();  //labels may not be defined yet on first pass
      throw "unknown label";
    }

    throw "unknown token";
  };

  try {
    const char *t = s;
    return nall::eval(t);
  } catch(const char *e) {
    warning({ "eval:", e });
    return 0;
  }
}

void Bass::evalMacros(string &line) {
  //only evaluate macros from the first block
  unsigned length = line.length();
  if(auto p = line.qposition(";")) length = p();

  for(unsigned x = 0; x < length; x++) {
    if(line[x] == '{') {
      signed counter = 1;
      for(unsigned y = x + 1; y < length; y++) {
        if(line[y] == '{') counter++;
        if(line[y] == '}') counter--;
        if(line[y] == '}' && counter == 0) {
          string name = substr(line, x + 1, y - x - 1);
          if(!name.position("::")) name = { activeNamespace, "::", name };

          lstring header, args;
          header.split<1>(" ", name);
          if(header[1] != "") args.split(",", header[1]);

          foreach(macro, macros) {
            if(header[0] == macro.name && args.size() == macro.args.size()) {
              string result;
              evalParams(result, macro, args);
              line = string(substr(line, 0, x), result, substr(line, y + 1));
              macroExpandCounter++;
              return evalMacros(line);
            }
          }
          break;
        }
      }
    }
  }
}

void Bass::evalParams(string &line, Bass::Macro &macro, lstring &args) {
  line = macro.value;
  line.replace("{#}", string("_", decimal(macroExpandCounter)));
  foreach(arg, macro.args, n) {
    line.replace(string("{", arg, "}"), args[n]);
  }
}
