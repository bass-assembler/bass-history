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

bool Bass::assemble(bool strict) {
  this->strict = strict;

  try {
    phase = Phase::Analyze;
    analyze();

    phase = Phase::Query;
    execute();

    phase = Phase::Write;
    execute();
  } catch(...) {
    return false;
  }

  return true;
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

  if(strict == false) return;
  struct BassWarning {};
  throw BassWarning();
}

template<typename... Args> void Bass::error(Args&&... args) {
  string s = string(std::forward<Args>(args)...);
  print("error: ", s, "\n");
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

void Bass::evaluateDefines(string& s) {
  for(signed x = s.size() - 1, y = -1; x >= 0; x--) {
    if(s[x] == '}') y = x;
    if(s[x] == '{' && y > x) {
      string name = s.slice(x + 1, y - x - 1);
      if(auto define = findDefine(name)) {
        s = {s.slice(0, x), define().value, s.slice(y + 1)};
        return evaluateDefines(s);
      }
    }
  }
}

void Bass::setDefine(const string& name, const string& value) {
  string frameName = name;
  unsigned n = frameName.beginswith(":") ? 0 : defines.size() - 1;
  frameName.ltrim<1>(":");

  string scopedName = frameName;
  if(scope.size()) scopedName = {scope.merge("."), ".", scopedName};

  if(auto define = defines[n].find({scopedName})) {
    define().value = value;
  } else {
    defines[n].insert({scopedName, value});
  }
}

optional<Bass::Define&> Bass::findDefine(const string& name) {
  string frameName = name;
  unsigned n = frameName.beginswith(":") ? 0 : defines.size() - 1;
  frameName.ltrim<1>(":");

  lstring s = scope;
  while(true) {
    string scopedName = {s.merge("."), s.size() ? "." : "", frameName};
    if(auto define = defines[n].find({scopedName})) {
      return {true, define()};
    }
    if(s.empty()) break;
    s.remove();
  }
  return false;
}

void Bass::setMacro(const string& name, const lstring& parameters, unsigned ip, bool scoped) {
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

optional<Bass::Macro&> Bass::findMacro(const string& name) {
  lstring s = scope;
  while(true) {
    string scopedName = {s.merge("."), s.size() ? "." : "", name};
    if(auto macro = macros.find({scopedName})) {
      return {true, macro()};
    }
    if(s.empty()) break;
    s.remove();
  }
  return false;
}

void Bass::setVariable(const string& name, int64_t value, bool constant) {
  string scopedName = name;
  if(scope.size()) scopedName = {scope.merge("."), ".", name};

  if(auto variable = variables.find({scopedName})) {
    if(queryPhase() && variable().constant) error("constant cannot be modified: ", scopedName);
    variable().value = value;
    variable().valid = constant || writePhase();
  } else {
    variables.insert({scopedName, value, constant, constant || writePhase()});
  }
}

optional<Bass::Variable&> Bass::findVariable(const string& name) {
  lstring s = scope;
  while(true) {
    string scopedName = {s.merge("."), s.size() ? "." : "", name};
    if(auto variable = variables.find({scopedName})) {
      if(queryPhase() || variable().valid) return {true, variable()};
    }
    if(s.empty()) break;
    s.remove();
  }
  return false;
}
