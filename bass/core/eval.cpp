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
      if(name.beginswith(".")) name = string(activeLabel, name);
      if(!name.position("::")) name = string(activeNamespace, "::", name);
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

void Bass::evalBlock(string &block) {
  //if block contains more than one statement, push subsequent statements into activeLine
  //this allows expansion of macros into more than one statement
  lstring blocks = block.qsplit<1>(";");
  if(blocks.size() > 1) {
    block = blocks[0];
    activeLine.last().insert(blockNumber.last() + 1, blocks[1]);
  }

  //remove/collapse excess space
  while(block.qposition("  ")) block.qreplace("  ", " ");
  block.qreplace(", ", ",");
  block.strip();
}

void Bass::evalMacros(string &line) {
  evalBlock(line);  //evalMacros() is recursive; split any additional statements from previous evaluation(s)
  unsigned length = line.length();

  for(unsigned x = 0; x < length; x++) {
    if(line[x] == '{') {
      signed counter = 1;  //count instances of { and } to find matching tags
      for(unsigned y = x + 1; y < length; y++) {
        if(line[y] == '{') counter++;
        if(line[y] == '}') counter--;

        if(line[y] == '}' && counter == 0) {
          string name = substr(line, x + 1, y - x - 1);

          //<intrinsics>
          if(name == "$") {  //pc
            line = {substr(line, 0, x), "0x", hex(pc()), substr(line, y + 1)};
            return evalMacros(line);
          }

          if(name == "@") {  //origin
            line = {substr(line, 0, x), "0x", hex(origin), substr(line, y + 1)};
            return evalMacros(line);
          }

          if(name.wildcard("-*")) {  //anonymous last label
            signed offset = (name == "-" ? -1 : integer(name));
            if(offset >= 0) error("invalid anonymous label index");
            line = {substr(line, 0, x), "anonymous::relativeLast", lastLabelCounter + offset - 0, substr(line, y + 1)};
            return evalMacros(line);
          }

          if(name.wildcard("+*")) {  //anonymous next label
            signed offset = (name == "+" ? +1 : integer(name));
            if(offset <= 0) error("invalid anonymous label index");
            line = {substr(line, 0, x), "anonymous::relativeNext", nextLabelCounter + offset - 1, substr(line, y + 1)};
            return evalMacros(line);
          }

          if(name.wildcard("defined ?*")) {  //definition test
            name.ltrim<1>("defined ");
            if(!name.position("::")) name = {activeNamespace, "::", name};
            unsigned found = 0;
            for(auto &macro : macros) {
              if(macro.name == name) {
                found = 1;
                break;
              }
            }
            line = {substr(line, 0, x), found, substr(line, y + 1)};
            return evalMacros(line);
          }
          //</intrinsics>

          lstring part = name.split<1>(" "), args;
          name = part(0);
          if(!name.position("::")) name = {activeNamespace, "::", name};
          if(!part(1).empty()) args = part(1).qsplit(",");
          for(auto &arg : args) arg.trim();

          for(auto &macro : macros) {
            if(name == macro.name && args.size() == macro.args.size()) {
              string result;
              evalParams(result, macro, args);
              string ldata = substr(line, 0, x);
              string rdata = substr(line, y + 1);
              line = {ldata, result, rdata};
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
  unsigned counter = 0;
  for(auto &arg : macro.args) {
    string substitution = args(counter++);

    if(substitution.wildcard("{eval ?*}")) {
      //evaluate macro argument prior to substitution, rather than after
      string name = string{substitution}.trim<1>("{eval ", "}");
      if(!name.position("::")) name = {activeNamespace, "::", name};
      for(auto &macro : macros) {
        if(name == macro.name && macro.args.size() == 0) {
          substitution = macro.value;
          break;
        }
      }
    }

    line.replace(string("{", arg, "}"), substitution);
  }
}
