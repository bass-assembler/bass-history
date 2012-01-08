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
      error({ "undefined label: ", name });
    }

    throw "unrecognized token";
  };

  try {
    const char *t = s;
    return fixedpoint::eval(t);
  } catch(const char *e) {
    error({ e, ": ", s });
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

          //<intrinsics>
          if(name == "$") {  //pc
            line = string(substr(line, 0, x), "0x", hex(pc()), substr(line, y + 1));
            return evalMacros(line);
          }
          if(name == "@") {  //origin
            line = string(substr(line, 0, x), "0x", hex(origin), substr(line, y + 1));
            return evalMacros(line);
          }
          if(name.wildcard("+*") || name.wildcard("-*")) {  //relative anonymous labels
            signed offset = (name == "-" ? -1 : name == "+" ? +1 : integer(name));
            if(offset == 0) error("invalid anonymous label index");
            if(offset > 0) offset--;  //side-effect of positive labels not having been encountered yet
            line = string(substr(line, 0, x), "anonymous::relative", relativeLabelCounter + offset, substr(line, y + 1));
            return evalMacros(line);
          }
          //</intrinsics>

          if(!name.position("::")) name = { activeNamespace, "::", name };

          lstring header = name.split<1>(" "), args;
          if(header(1, "") != "") args = header[1].split(",");

          for(auto &macro : macros) {
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
  unsigned counter = 0;
  for(auto &arg : macro.args) {
    line.replace(string("{", arg, "}"), args[counter++]);
  }
}
