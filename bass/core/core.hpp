struct Bass {
  bool target(const string& filename, bool create);
  bool source(const string& filename);
  void define(const string& name, const string& value);
  bool assemble(bool strict = false);

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

  struct Macro {
    string name;
    lstring parameters;
    unsigned ip;
    bool scoped;

    unsigned hash() const { return name.hash(); }
    bool operator==(const Macro& source) const { return name == source.name; }
    bool operator< (const Macro& source) const { return name <  source.name; }
    Macro() {}
    Macro(const string& name) : name(name) {}
    Macro(const string& name, const lstring& parameters, unsigned ip, bool scoped) : name(name), parameters(parameters), ip(ip), scoped(scoped) {}
  };

  struct Variable {
    string name;
    int64_t value;
    bool constant;
    bool valid = false;

    unsigned hash() const { return name.hash(); }
    bool operator==(const Variable& source) const { return name == source.name; }
    bool operator< (const Variable& source) const { return name <  source.name; }
    Variable() {}
    Variable(const string& name) : name(name) {}
    Variable(const string& name, int64_t value, bool constant, bool valid) : name(name), value(value), constant(constant), valid(valid) {}
  };

  struct BlockStack {
    unsigned ip;
    string type;
  };

  struct CallStack {
    unsigned ip;
    bool scoped;
  };

  file targetFile;
  lstring sourceFilenames;

  Instruction* activeInstruction = nullptr;
  vector<Instruction> program;
  vector<BlockStack> blockStack;    //track the start and end of blocks
  hashset<Define> coreDefines;      //defines specified on the terminal
  vector<hashset<Define>> defines;  //stack frame to hold variables for macro recursion
  hashset<Macro> macros;
  hashset<Variable> variables;
  vector<CallStack> callStack;      //track macro recursion
  vector<bool> ifStack;             //track conditional matching
  lstring stack;                    //track push, pull directives
  lstring scope;                    //track scope recursion
  int64_t stringTable[256];         //overrides for d[bwldq] text strings
  Phase phase;
  Endian endian = Endian::LSB;
  unsigned macroInvocationCounter;  //used for {#} support
  unsigned ip = 0;                  //instruction pointer into program
  unsigned origin = 0;              //file offset
  signed base = 0;                  //file offset to memory map displacement
  unsigned lastLabelCounter = 1;    //- instance counter
  unsigned nextLabelCounter = 1;    //+ instance counter
  bool strict = false;              //upgrade warnings to errors when true

  bool analyzePhase() const { return phase == Phase::Analyze; }
  bool queryPhase() const { return phase == Phase::Query; }
  bool writePhase() const { return phase == Phase::Write; }

  //core
  void printInstruction();
  template<typename... Args> void notice(Args&&... args);
  template<typename... Args> void warning(Args&&... args);
  template<typename... Args> void error(Args&&... args);

  unsigned pc() const;
  void seek(unsigned offset);
  void write(uint64_t data, unsigned length = 1);

  string text(string s);
  string filepath() const;

  void evaluateDefines(string& statement);
  void setDefine(const string& name, const string& value);
  optional<Define&> findDefine(const string& name);

  void setMacro(const string& name, const lstring& parameters, unsigned ip, bool scoped);
  optional<Macro&> findMacro(const string& name);

  void setVariable(const string& name, int64_t value, bool constant = false);
  optional<Variable&> findVariable(const string& name);

  //evaluate
  enum class Evaluation : unsigned { Default = 0, Strict = 1 };  //strict mode disallows forward-declaration of constants
  int64_t evaluate(const string& expression, Evaluation mode = Evaluation::Default);
  int64_t evaluate(Eval::Node* node, Evaluation mode);
  lstring evaluateParameters(Eval::Node* node, Evaluation mode);
  int64_t evaluateFunction(Eval::Node* node, Evaluation mode);
  int64_t evaluateMember(Eval::Node* node, Evaluation mode);
  int64_t evaluateLiteral(Eval::Node* node, Evaluation mode);

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
