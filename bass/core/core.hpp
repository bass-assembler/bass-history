struct Bass {
  bool target(const string& filename, bool create);
  bool source(const string& filename);
  bool preprocess();
  bool assemble();

  //preprocessor
  void setDefine(const string& name, const string& value);

protected:
  enum class Phase : unsigned { Initialize, Analyze, Execute, Query, Write };
  enum class Endian : unsigned { LSB, MSB };

  struct Instruction {
    string statement;
    unsigned ip;

    unsigned fileNumber;
    unsigned lineNumber;
    unsigned blockNumber;
  };

  struct BlockStack {
    string type;
    unsigned ip;
  };

  struct Macro {
    string name;
    lstring parameters;
    unsigned ip;

    unsigned hash() const { return name.hash(); }
    bool operator==(const Macro& source) const { return name == source.name; }
    Macro() {}
    Macro(const string& name) : name(name) {}
    Macro(const string& name, const lstring& parameters, unsigned ip) : name(name), parameters(parameters), ip(ip) {}
  };

  struct Define {
    string name;
    string value;

    unsigned hash() const { return name.hash(); }
    bool operator==(const Define& source) const { return name == source.name; }
    Define() {}
    Define(const string& name) : name(name) {}
    Define(const string& name, const string& value) : name(name), value(value) {}
  };

  struct Variable {
    string name;
    int64_t value;
    bool constant;

    unsigned hash() const { return name.hash(); }
    bool operator==(const Variable& source) const { return name == source.name; }
    Variable() {}
    Variable(const string& name) : name(name) {}
    Variable(const string& name, int64_t value, bool constant) : name(name), value(value), constant(constant) {}
  };

  file targetFile;
  lstring sourceFilenames;

  vector<Instruction> program;
  vector<Instruction> instructions;
  vector<BlockStack> blockStack;
  hashset<Macro> macros;
  vector<hashset<Define>> contexts;
  hashset<Define> defines;
  hashset<Variable> variables;
  vector<unsigned> callStack;
  vector<bool> ifStack;
  lstring stack;
  lstring scope;
  int64_t stringTable[256];
  Phase phase = Phase::Initialize;
  Endian endian = Endian::LSB;
  unsigned macroInvocationCounter = 0;
  unsigned ip = 0;
  unsigned origin = 0;
  signed base = 0;
  unsigned lastLabelCounter;
  unsigned nextLabelCounter;

  //core
  bool analyzePhase() const { return phase == Phase::Analyze; }
  bool executePhase() const { return phase == Phase::Execute; }
  bool queryPhase() const { return phase == Phase::Query; }
  bool writePhase() const { return phase == Phase::Write; }

  template<typename... Args> void error(Args&&... args);
  template<typename... Args> void warning(Args&&... args);
  unsigned pc() const;
  void seek(unsigned offset);
  void write(uint64_t data, unsigned length = 1);

  //evaluator
  int64_t evaluate(const string& expression);
  int64_t evaluate(Eval::Node* node);
  int64_t evaluateMember(Eval::Node* node);
  int64_t evaluateLiteral(Eval::Node* node);

  //preprocessor
  bool preprocessAnalyze();
  bool preprocessAnalyzeInstruction(Instruction& instruction);
  bool preprocessExecute();
  bool preprocessExecuteInstruction(Instruction& instruction);
  void preprocessDefines(string& statement);
  void setMacro(const string& name, const lstring& parameters, unsigned ip);

  //assembler
  virtual void assembleInitialize();
  bool assemblePhase();
  virtual bool assembleInstruction(Instruction& instruction);
  string text(string s);
  optional<int64_t> findVariable(const string& name);
  int64_t getVariable(const string& name);
  void setVariable(const string& name, int64_t value, bool constant = false);
};
