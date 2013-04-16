int64_t Bass::eval(const string &s) {
  fixedpoint::eval_fallback = [this](const char *&s) -> int64_t {
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
      name = qualifyLabel(name);
      for(auto &label : labels) if(name == label.name) return label.offset;
      if(pass == 1) return pc();  //labels may not be defined yet on first pass
      error({"undefined label: ", name});
    }

    throw "unrecognized token";
  };

  try {
    const char *t = s;
    return fixedpoint::eval(t);
  } catch(const char *errorMessage) {
    error({errorMessage, ": ", s});
    return 0;
  }
}

//return true if this block was a macro that was evaluated fully
//otherwise, return false so that assembleBlock() will parse it
bool Bass::evalMacros(string &block) {
  if(block.wildcard("{*}")) {
    lstring part = string{block}.trim<1>("{", "}").split<1>(" "), args;
    string name = qualifyMacro(part(0));
    if(part.size() >= 2) args = part(1).split(",");

    assembleMacro(name, args);
    return true;
  }

  evalDefines(block);
  return false;
}

void Bass::evalDefines(string &line) {
  unsigned length = line.length();

  for(unsigned x = 0; x < length; x++) {
    if(line[x] == '{') {
      signed counter = 1;  //count instances of { and } to find matching tags
      for(unsigned y = x + 1; y < length; y++) {
        if(line[y] == '{') counter++;
        if(line[y] == '}') counter--;

        if(line[y] == '}' && counter == 0) {
          string name = substr(line, x + 1, y - x - 1);
          line = {substr(line, 0, x), evalDefine(name), substr(line, y + 1)};
          return evalDefines(line);
        }
      }
    }
  }
}

string Bass::evalDefine(string &name) {
  if(name == "$") {  //pc
    return {"0x", hex(pc())};
  }

  if(name == "@") {  //origin
    return {"0x", hex(origin)};
  }

  if(name.wildcard("eval ?*")) {
    name.ltrim<1>("eval ");
    evalDefines(name);
    return eval(name);
  }

  if(name.wildcard("hex ?*")) {
    name.ltrim<1>("hex ");
    evalDefines(name);
    return {"0x", hex(eval(name))};
  }

  if(name.wildcard("defined ?*")) {  //definition test
    name.ltrim<1>("defined ");
    name = qualifyMacro(name);
    for(auto &macro : macros) {
      if(macro.name == name) return "1";
    }
    return "0";
  }

  if(name.wildcard("-*")) {  //anonymous last label
    signed offset = (name == "-" ? -1 : integer(name));
    if(offset >= 0) error("invalid anonymous label index");
    return {"anonymous::relativeLast", lastLabelCounter + offset - 0};
  }

  if(name.wildcard("+*")) {  //anonymous next label
    signed offset = (name == "+" ? +1 : integer(name));
    if(offset <= 0) error("invalid anonymous label index");
    return {"anonymous::relativeNext", nextLabelCounter + offset - 1};
  }

  lstring part = name.split<1>(" "), args;
  name = qualifyMacro(part(0));
  if(!part(1).empty()) args = part(1).qsplit(",");
  for(auto &arg : args) arg.trim();

  for(auto &macro : macros) {
    if(name == macro.name && args.size() == macro.args.size()) {
      if(args.size() > 0) error("macro define evaluation may not contain macro arguments");
      return macro.value;
    }
  }

  error({"unknown define: ", name});
}
