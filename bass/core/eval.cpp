int64_t Bass::eval(const string &s) {
  if(s == "+") {
    string name = { "+", positiveLabelCounter + 0 };
    foreach(label, labels) if(name == label.name) return label.offset;
    if(pass == 1) return 0u;
    warning("eval:unmatched + label");
    return 0;
  }

  if(s == "-") {
    string name = { "-", negativeLabelCounter - 1 };
    foreach(label, labels) if(name == label.name) return label.offset;
    if(pass == 1) return 0u;
    warning("eval:unmatched - label");
    return 0;
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

    //label
    if(*s == ':' || *s == '.' || *s == '_' || (*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z')) {
      const char *start = s;
      while(*s == ':' || *s == '.' || *s == '_' || (*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z') || (*s >= '0' && *s <= '9')) s++;
      string name = substr(start, 0, s - start);
      if(name.beginswith(".")) name = string(activeLabel, name);
      if(!name.position("::")) name = string(activeNamespace, "::", name);
      foreach(label, labels) if(name == label.name) return label.offset;
      if(pass == 1) return 0u;  //labels may not be defined yet on first pass
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

void Bass::evalDefines(string &line) {
  unsigned length = line.length();
  for(unsigned x = 0; x < length; x++) {
    if(line[x] == '{') {
      for(unsigned y = x + 1; y < length; y++) {
        if(line[y] == '}') {
          string name = substr(line, x + 1, y - x - 1);
          if(!name.position("::")) name = { activeNamespace, "::", name };

          lstring header, args;
          header.split<1>(" ", name);
          if(header[1] != "") args.split(",", header[1]);

          foreach(define, defines) {
            if(header[0] == define.name && args.size() == define.args.size()) {
              string result;
              evalParams(result, define, args);
              line = string(substr(line, 0, x), result, substr(line, y + 1));
              return evalDefines(line);
            }
          }
          break;
        }
      }
    }
  }
}

void Bass::evalParams(string &line, Bass::Define &define, lstring &args) {
  line = define.value;
  foreach(arg, define.args, n) {
    line.replace(string("{", arg, "}"), args[n]);
  }
}
