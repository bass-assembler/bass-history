void Bass::assembleInitialize() {
  ip = 0;
  origin = 0;
  base = 0;
}

bool Bass::assemblePhase() {
  while(ip < instructions.size()) {
    Instruction& i = instructions(ip++);
    if(!assembleInstruction(i)) return false;
  }

  return true;
}

bool Bass::assembleInstruction(Instruction& i) {
  string s = i.statement;

  if(s.match("?*:")) {
    s.rtrim<1>(":");

    if(auto variable = variables.find({s})) {
      if(!writePhase()) error("label ", s, " has already been declared\n");
      variable().value = pc();
    } else {
      Variable v;
      v.name = s;
      v.value = pc();
      variables.insert(v);
    }

    return true;
  }

  if(s.match("endian ?*")) {
    s.ltrim<1>("endian ");
    if(s == "lsb") { endian = Endian::LSB; return true; }
    if(s == "msb") { endian = Endian::MSB; return true; }
    error("invalid endian mode");
  }

  if(s.match("origin ?*")) {
    s.ltrim<1>("origin ");
    origin = evaluate(s);
    return true;
  }

  if(s.match("base ?*")) {
    s.ltrim<1>("base ");
    base = evaluate(s);
    return true;
  }

  unsigned dataLength = 0;
  if(s.beginswith("db ")) dataLength = 1;
  if(s.beginswith("dw ")) dataLength = 2;
  if(s.beginswith("dl ")) dataLength = 3;
  if(s.beginswith("dd ")) dataLength = 4;
  if(s.beginswith("dq ")) dataLength = 8;
  if(dataLength) {
    s = s.slice(3);  //remove prefix
    lstring p = s.qsplit(",").strip();
    for(auto& t : p) {
      if(t.match("\"*\"")) {
        t = text(t);
        for(auto& b : t) write(b, dataLength);
      } else {
        write(evaluate(t), dataLength);
      }
    }
    return true;
  }

  if(s.match("print ?*")) {
    s.ltrim<1>("print ");
    if(writePhase()) {
      lstring p = s.qsplit(",").strip();
      for(auto& t : p) print(text(t));
    }
    return true;
  }

  return false;
}

string Bass::text(string s) {
  if(s.match("\"*\"")) {
    s.trim<1>("\"");
    s.replace("\\n", "\n");
    s.replace("\\q", "\"");
    s.replace("\\\\", "\\");
    return s;
  } else {
    return evaluate(s);
  }
}
