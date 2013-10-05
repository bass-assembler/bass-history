#include "evaluate.cpp"
#include "analyze.cpp"
#include "execute.cpp"
#include "assemble.cpp"

bool Bass::target(const string& filename, bool create) {
  if(targetFile.open()) targetFile.close();
  if(filename.empty()) return true;

  //cannot modify a file unless it exists
  if(!file::exists(filename)) create = true;

  if(!targetFile.open(filename, create ? file::mode::write : file::mode::modify)) {
    print("warning: unable to open target file: ", filename, "\n");
    return false;
  }

  return true;
}

bool Bass::source(const string& filename) {
  if(!file::exists(filename)) {
    print("warning: source file not found: ", filename, "\n");
    return false;
  }

  unsigned fileNumber = sourceFilenames.size();
  sourceFilenames.append(filename);

  string data = file::read(filename);
  data.transform("\t\r", "  ");

  lstring lines = data.split("\n");
  for(unsigned lineNumber = 0; lineNumber < lines.size(); lineNumber++) {
    if(auto position = lines[lineNumber].find("//")) lines[lineNumber].resize(position());  //remove comments

    lstring blocks = lines[lineNumber].qsplit(";").strip();
    for(unsigned blockNumber = 0; blockNumber < blocks.size(); blockNumber++) {
      string statement = blocks[blockNumber].strip();
      if(statement.empty()) continue;

      if(statement.match("include \"?*\"")) {
        statement.trim<1>("include \"", "\"");
        source({dir(filename), statement});
      } else {
        Instruction instruction;
        instruction.statement = statement;
        instruction.fileNumber = fileNumber;
        instruction.lineNumber = 1 + lineNumber;
        instruction.blockNumber = 1 + blockNumber;
        program.append(instruction);
      }
    }
  }

  return true;
}

void Bass::define(const string& name, const string& value) {
  coreDefines.insert({name, value});
}

bool Bass::assemble() {
  try {
    phase = Phase::Analyze;
    analyze();

    phase = Phase::Query;
    execute();

    phase = Phase::Write;
    execute();

    return true;
  } catch(...) {
    return false;
  }
}

void Bass::printInstruction() {
  auto& i = *activeInstruction;
  print(sourceFilenames[i.fileNumber], ":", i.lineNumber, ":", i.blockNumber, ": ", i.statement, "\n");
}

template<typename... Args> void Bass::notice(Args&&... args) {
  string s = string(std::forward<Args>(args)...);
  print("notice: ", s, "\n");
  printInstruction();
}

template<typename... Args> void Bass::warning(Args&&... args) {
  string s = string(std::forward<Args>(args)...);
  print("warning: ", s, "\n");
  printInstruction();
}

template<typename... Args> void Bass::error(Args&&... args) {
  string s = string(std::forward<Args>(args)...);
  print("error: ", s, "\n");
  auto& i = *activeInstruction;
  print(sourceFilenames[i.fileNumber], ":", i.lineNumber, ":", i.blockNumber, ": ", i.statement, "\n");
  printInstruction();

  struct BassError {};
  throw BassError();
}

unsigned Bass::pc() const {
  return origin + base;
}

void Bass::seek(unsigned offset) {
  if(!targetFile.open()) return;
  if(writePhase()) targetFile.seek(offset);
}

void Bass::write(uint64_t data, unsigned length) {
  if(!targetFile.open()) return;
  if(writePhase()) {
    if(endian == Endian::LSB) targetFile.writel(data, length);
    if(endian == Endian::MSB) targetFile.writem(data, length);
  }
  origin += length;
}

string Bass::text(string s) {
  s.replace("\\n", "\n");
  s.replace("\\q", "\"");
  s.replace("\\\\", "\\");
  return s;
}

string Bass::filepath() const {
  return dir(sourceFilenames[activeInstruction->fileNumber]);
}

void Bass::setMacro(const string& name, const lstring& parameters, unsigned ip) {
  string overloadName = {name, ":", parameters.size()};
  if(auto macro = macros.find({overloadName})) {
    macro().parameters = parameters;
    macro().ip = ip;
  } else {
    macros.insert({overloadName, parameters, ip});
  }
}

void Bass::setDefine(const string& name, const string& value) {
  for(signed n = defines.size() - 1; n >= 0; n--) {
    if(auto define = defines[n].find({name})) {
      define().value = value;
      return;
    }
  }
  defines.last().insert({name, value});
}

optional<string> Bass::findDefine(const string& name) {
  for(signed n = defines.size() - 1; n >= 0; n--) {
    if(auto define = defines[n].find({name})) {
      return define().value;
    }
  }
  return false;
}

void Bass::evaluateDefines(string& s) {
  for(signed x = s.size() - 1, y = -1; x >= 0; x--) {
    if(s[x] == '}') y = x;
    if(s[x] == '{' && y > x) {
      string name = s.slice(x + 1, y - x - 1);
      if(auto define = findDefine(name)) {
        s = {s.slice(0, x), define(), s.slice(y + 1)};
        return evaluateDefines(s);
      }
    }
  }
}

void Bass::setVariable(const string& name, int64_t value, bool constant) {
  string scopedName = name;
  if(scope.size()) scopedName = {scope.merge("."), ".", name};

  if(auto variable = variables.find({scopedName})) {
    if(!writePhase() && variable().constant) error("constant cannot be modified: ", scopedName);
    variable().value = value;
    variable().valid = true;
  } else {
    variables.insert({scopedName, value, constant});
  }
}

optional<int64_t> Bass::findVariable(const string& name) {
  lstring s = scope;
  while(true) {
    string scopedName = {s.merge("."), s.size() ? "." : "", name};
    if(auto variable = variables.find({scopedName})) {
      if(variable().valid) return {true, variable().value};
    }
    if(s.empty()) break;
    s.remove();
  }
  return false;
}

int64_t Bass::getVariable(const string& name) {
  if(auto variable = findVariable(name)) return variable();
  if(queryPhase()) return pc();
  error("variable not found: ", name);
}
