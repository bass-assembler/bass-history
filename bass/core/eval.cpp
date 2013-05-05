int64_t Bass::eval(string s) {
  fixedpoint::eval_fallback = [this](const char*& s) -> int64_t {
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
      const char* start = s;
      while(*s == ':' || *s == '.' || *s == '_' || (*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z') || (*s >= '0' && *s <= '9')) s++;
      string name = substr(start, 0, s - start);
      name = qualifyLabel(name);
      if(const auto& label = labels.find(name)) return label().offset;
      if(pass == 1) return pc();  //labels may not be defined yet on first pass
      error("undefined label: ", name);
    }

    throw "unrecognized token";
  };

  try {
    const char* t = s;
    return fixedpoint::eval(t);
  } catch(const char* errorMessage) {
    error(errorMessage, ": ", s);
    return 0;
  }
}

void Bass::evalMacros(string& line) {
  //evaluate the deepest macro first, so that macros can be arguments to other macros
  //example: {add {add 1,2},{add 3,4}} == 10
  unsigned depth = 0, maxDepth = 0;
  for(unsigned x = 0; x < line.size(); x++) {
    if(line[x] == '{') depth++;
    if(depth >= maxDepth) maxDepth = depth;
    if(line[x] == '}') depth--;
  }

  depth = 0;
  for(unsigned x = 0; x < line.size(); x++) {
    if(line[x] == '{') depth++;
    if(line[x] == '}') depth--;

    if(line[x] == '{' && depth == maxDepth) {
      signed counter = 0;  //count instances of { and } to find matching tags
      for(unsigned y = x; y < line.size(); y++) {
        if(line[y] == '{') counter++;
        if(line[y] == '}') counter--;

        if(line[y] == '}' && counter == 0) {
          string name = substr(line, x + 1, y - x - 1);
          line = {substr(line, 0, x), evalMacro(name), substr(line, y + 1)};
          return evalMacros(line);
        }
      }
    }
  }
}

string Bass::evalMacro(string& name) {
  if(name == "$") {  //pc
    return {"0x", hex(pc())};
  }

  if(name == "@") {  //origin
    return {"0x", hex(origin)};
  }

  if(name.wildcard("eval ?*")) {
    name.ltrim<1>("eval ");
    evalMacros(name);
    return eval(name);
  }

  if(name.wildcard("hex ?*")) {
    name.ltrim<1>("hex ");
    evalMacros(name);
    return {"0x", hex(eval(name))};
  }

  if(name.wildcard("defined ?*")) {  //definition test
    name.ltrim<1>("defined ");
    name = qualifyMacro(name, 0);
    if(const auto& macro = macros.find(name)) return "1";
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
  if(part(1)) args = part(1).qsplit(",").strip();
  name = qualifyMacro(part(0), args.size());
  return assembleMacro(name, args);
}
