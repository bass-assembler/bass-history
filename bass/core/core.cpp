#include "evaluator.cpp"
#include "preprocessor.cpp"
#include "assembler.cpp"

bool Bass::target(const string& filename, bool overwrite) {
  if(targetFile.open()) targetFile.close();
  if(filename.empty()) return true;

  //cannot modify a file unless it exists
  if(!file::exists(filename)) overwrite = true;

  if(!targetFile.open(filename, overwrite ? file::mode::write : file::mode::modify)) {
    print("warning: unable to open target file ", filename, "\n");
    return false;
  }

  return true;
}

bool Bass::source(const string& filename) {
  if(!file::exists(filename)) {
    print("warning: source file ", filename, " not found\n");
    return false;
  }

  sourceFilename.append(filename);

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
        source({dir(sourceFilename.last()), statement});
      } else {
        Instruction instruction;
        instruction.statement = statement;
        instruction.fileNumber = sourceFilename.size() - 1;
        instruction.lineNumber = 1 + lineNumber;
        instruction.blockNumber = 1 + blockNumber;
        program.append(instruction);
      }
    }
  }

  return true;
}

bool Bass::preprocess() {
  try {
    phase = Phase::Analyze; preprocessAnalyze();
    phase = Phase::Execute; preprocessExecute();
    return true;
  } catch(...) {
    return false;
  }
}

bool Bass::assemble() {
  try {
    phase = Phase::Query; assembleInitialize(); assemblePhase();
    phase = Phase::Write; assembleInitialize(); assemblePhase();
  } catch(...) {
    return false;
  }
}

template<typename... Args> void Bass::error(Args&&... args) {
  string s = string(std::forward<Args>(args)...);
  print("error: ", s, "\n");

  struct BassError {};
  throw BassError();
}

template<typename... Args> void Bass::warning(Args&&... args) {
  string s = string(std::forward<Args>(args)...);
  print("warning: ", s, "\n");

  struct BassWarning {};
  throw BassWarning();
}

bool Bass::writePhase() const {
  return phase == Phase::Write;
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
