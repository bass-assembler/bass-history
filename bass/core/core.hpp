struct Bass {
  bool target(const string& filename, bool create);
  bool source(const string& filename);
  void define(const string& name, const string& value);
  bool assemble();

protected:
  enum class Phase : unsigned { Analyze, Query, Write };
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
    bool operator< (const Macro& source) const { return name <  source.name; }
    Macro() {}
    Macro(const string& name) : name(name) {}
    Macro(const string& name, const lstring& parameters, unsigned ip) : name(name), parameters(parameters), ip(ip) {}
  };

  struct Define {
    string name;
    string value;

    unsigned hash() const { return name.hash(); }
    bool operator==(const Define& source) const { return name == source.name; }
    bool operator< (const Define& source) const { return name <  source.name; }
    Define() {}
    Define(const string& name) : name(name) {}
    Define(const string& name, const string& value) : name(name), value(value) {}
  };

  struct Variable {
    string name;
    int64_t value;
    bool constant;
    bool valid = true;

    unsigned hash() const { return name.hash(); }
    bool operator==(const Variable& source) const { return name == source.name; }
    bool operator< (const Variable& source) const { return name <  source.name; }
    Variable() {}
    Variable(const string& name) : name(name) {}
    Variable(const string& name, int64_t value, bool constant) : name(name), value(value), constant(constant) {}
  };

  file targetFile;
  lstring sourceFilenames;

  Instruction* activeInstruction = nullptr;
  vector<Instruction> program;
  vector<BlockStack> blockStack;
  set<Macro> macros;
  set<Define> coreDefines;
  vector<set<Define>> defines;
  set<Variable> variables;
  vector<unsigned> callStack;
  vector<bool> ifStack;
  lstring stack;
  lstring scope;
  int64_t stringTable[256];
  Phase phase;
  Endian endian = Endian::LSB;
  unsigned macroInvocationCounter = 0;
  unsigned ip = 0;
  unsigned origin = 0;
  signed base = 0;
  unsigned lastLabelCounter = 1;
  unsigned nextLabelCounter = 1;

  //core
  bool analyzePhase() const { return phase == Phase::Analyze; }
  bool queryPhase() const { return phase == Phase::Query; }
  bool writePhase() const { return phase == Phase::Write; }

  void printInstruction();
  template<typename... Args> void notice(Args&&... args);
  template<typename... Args> void warning(Args&&... args);
  template<typename... Args> void error(Args&&... args);

  unsigned pc() const;
  void seek(unsigned offset);
  void write(uint64_t data, unsigned length = 1);

  string text(string s);
  string filepath() const;

  void setMacro(const string& name, const lstring& parameters, unsigned ip);

  void setDefine(const string& name, const string& value);
  optional<string> findDefine(const string& name);
  void evaluateDefines(string& statement);

  void setVariable(const string& name, int64_t value, bool constant = false);
  optional<int64_t> findVariable(const string& name);
  int64_t getVariable(const string& name);

  //evaluate
  int64_t evaluate(const string& expression);
  int64_t evaluate(Eval::Node* node);
  lstring evaluateParameters(Eval::Node* node);
  int64_t evaluateFunction(Eval::Node* node);
  int64_t evaluateMember(Eval::Node* node);
  int64_t evaluateLiteral(Eval::Node* node);

  //analyze
  bool analyze();
  bool analyzeInstruction(Instruction& instruction);

  //execute
  bool execute();
  bool executeInstruction(Instruction& instruction);

  //assemble
  virtual void initialize();
  virtual bool assemble(const string& statement);
};
