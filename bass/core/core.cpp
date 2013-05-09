#include "directives.cpp"
#include "intrinsics.cpp"
#include "eval.cpp"
#include "text.cpp"

bool Bass::open(const string& filename, Bass::FileMode mode) {
  close();
  if(pass == 0 && !filename) return true;  //filename not required on command-line

  if(mode == FileMode::Auto) {
    if(options.overwrite) mode = FileMode::Create;
  }
  if(mode == FileMode::Modify) {
    output.open(filename, file::mode::readwrite);
  }
  if(output.open() == false) {
    output.open(filename, file::mode::write);
  }
  if(output.open() == true) {
    origin = 0;
    base = 0;
    return true;
  }
  return false;
}

bool Bass::assemble(const string& filename) {
  for(pass = 1; pass <= 2; pass++) {
    initialize(pass);
    endian = Endian::LSB;
    origin = 0;
    base = 0;
    for(unsigned n = 0; n < 256; n++) table[n] = n;
    sourceFiles.reset();
    contexts.reset();
    macros.reset();
    for(auto& macro : defaultMacros) macros.insert(macro);
    activeNamespace = "global";
    activeLabel = "global";
    macroDepth = 0;
    macroExpandCounter = 1;
    macroReturnStack.reset();
    lastLabelCounter = 1;
    nextLabelCounter = 1;
    conditionStack.reset();
    conditionStack.append(Condition::Matching);
    stack.reset();
    try {
      assembleFile(filename);
    } catch(...) {
      return false;
    }
  }
  return true;
}

void Bass::close() {
  if(output.open()) output.close();
}

Bass::Bass() {
  pass = 0;
  options.overwrite = false;
}

//internal

void Bass::initialize(unsigned pass) {
}

template<typename... Args> void Bass::warning(Args&&... args) {
  string s = string(std::forward<Args>(args)...);
  print("warning: ", s, "\n");
  const auto& context = contexts.last();
  print(context.name, ":", 1 + context.line, ":", 1 + context.block, "\n");
}

template<typename... Args> void Bass::error(Args&&... args) {
  string s = string(std::forward<Args>(args)...);
  print("error: ", s, "\n");
  for(signed index = contexts.size() - 1; index >= 0; index--) {
    const auto& context = contexts[index];
    print("  called from ", context.name, ":", 1 + context.line, ":", 1 + context.block, "\n");
  }
  struct bass_parser_error{};
  throw bass_parser_error{};
}

unsigned Bass::pc() const {
  return origin + base;
}

void Bass::seek(unsigned offset) {
  if(output.open() == false) error("output file not opened\n");
  output.seek(offset);
}

void Bass::write(uint64_t data, unsigned length) {
  if(output.open() == false) error("output file not opened\n");
  if(pass == 2) {
    if(endian == Endian::LSB) output.writel(data, length);
    if(endian == Endian::MSB) output.writem(data, length);
  }
  origin += length;
}

string Bass::qualifyMacro(string name, unsigned args) {
  if(name.beginswith("@")) name = {"bass::", name.ltrim<1>("@")};
  if(name.beginswith("#")) name = {"self::", name.ltrim<1>("#")};

  string ns;
  if(auto position = name.find("::")) {
    ns = name.slice(0, position());
    name = name.slice(position() + 2);
  } else {
    ns = activeNamespace;
  }

  if(ns == "self") ns = {"!", macroRecursion()};
  return {ns, "::", name, "{", args, "}"};
}

void Bass::setMacro(string name, const lstring& args, const string& value) {
  name = qualifyMacro(name, args.size());
  macros.insert({name, args, value});
}

string Bass::qualifyLabel(string name) {
  if(name.beginswith("#")) name = {"self::", name.ltrim<1>("#")};

  string ns;
  if(auto position = name.find("::")) {
    ns = name.slice(0, position());
    name = name.slice(position() + 2);
  } else {
    ns = activeNamespace;
  }

  if(name.beginswith(".")) name = {activeLabel, name};
  if(ns == "self") ns = {"!", macroExpandCounter};
  return {ns, "::", name};
}

void Bass::setLabel(string name, unsigned offset) {
  name = qualifyLabel(name);
  if(labels.find(name)) {
    if(pass == 1) error("label has already been declared: ", name);
  }
  labels.insert({name, offset});
}

void Bass::assembleFile(const string& filename) {
  if(!file::exists(filename)) error("source file not found: ", filename);
  sourceFiles.append(filename);
  string source = file::read(filename);
  source.transform("\r\t", "  ");  //treat ignored whitespace characters as spaces
  assembleSource(filename, source);
  sourceFiles.remove();
}

unsigned Bass::macroRecursion() const {
  return macroReturnStack.size();
}

optional<string> Bass::assembleMacro(const string& name, const lstring& args) {
  if(const auto& match = macros.find(name)) {
    const auto& macro = match();

    conditionStack.append(Condition::Matching);
    macroExpandCounter++;
    macroReturnStack.append("");

    for(unsigned n = 0; n < args.size(); n++) {
      string name = {"!", macroRecursion(), "::", macro.args[n]};
      string value = args[n];
      value = {"return ", value};
      evalMacros(value);
      setMacro(name, {}, value);
    }

    assembleSource({"{function ", macro.name, "}"}, macro.value);
    conditionStack.take();
    return {true, macroReturnStack.take()};
  }

  return false;
}

void Bass::assembleSource(const string& name, const string& source) {
  Context context;
  lstring lines = source.split("\n");
  for(auto& line : lines) {
    if(auto position = line.qfind("//")) line.resize(position());  //strip comments
    context.source.append(line.qsplit(";").strip());
  }
  context.name = name;
  context.line = 0;
  context.block = 0;
  contexts.append(context);

  #define self contexts.last()
  loop:
  sync = false;
  while(self.line < self.source.size()) {
    while(self.block < self.source[self.line].size()) {
      string block = self.source[self.line][self.block];
      if(assembleDirective(block) == false) error("unknown command: ", block);
      if(sync) goto loop;
      self.block++;
    }
    self.block = 0;
    self.line++;
  }
  #undef self

  contexts.remove();
  if(contexts.size() == 0) {
    if(conditionStack.size() != 1) error("if without matching endif");
    if(macroDepth) error("function without matching endfunction");
  }
}
