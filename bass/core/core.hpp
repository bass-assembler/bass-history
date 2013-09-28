struct Bass {
  bool target(const string& filename, bool overwrite);
  bool source(const string& filename);
  bool preprocess();
  bool assemble();

protected:
  enum class Phase : unsigned { Analyze, Execute, Query, Write };
  enum class Endian : unsigned { LSB, MSB };

  struct Instruction {
    string statement;
    Eval::Node* expression = nullptr;
    unsigned ip[2];

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
  };

  struct Define {
    string name;
    string value;

    unsigned hash() const { return name.hash(); }
    bool operator==(const Define& source) const { return name == source.name; }
    Define() {}
    Define(const string& name) : name(name) {}
  };

  struct Variable {
    string name;
    int64_t value;

    unsigned hash() const { return name.hash(); }
    bool operator==(const Variable& source) const { return name == source.name; }
    Variable() {}
    Variable(const string& name) : name(name) {}
  };

  Phase phase;
  Endian endian = Endian::LSB;

  file targetFile;
  lstring sourceFilename;
  vector<Instruction> program;
  vector<Instruction> instructions;
  vector<BlockStack> blockStack;
  hashset<Macro> macros;
  vector<hashset<Define>> contexts;
  hashset<Define> defines;
  hashset<Variable> variables;
  vector<unsigned> callStack;
  vector<bool> ifStack;
  unsigned macroInvocationCounter = 0;
  unsigned ip = 0;
  unsigned origin = 0;
  signed base = 0;

  //core
  template<typename... Args> void error(Args&&... args);
  template<typename... Args> void warning(Args&&... args);
  bool writePhase() const;
  unsigned pc() const;
  void seek(unsigned offset);
  void write(uint64_t data, unsigned length = 1);

  //evaluator
  int64_t evaluate(const string& expression);
  int64_t evaluate(Eval::Node* node);
  int64_t evaluateLiteral(Eval::Node* node);

  //preprocessor
  bool preprocessAnalyze();
  bool preprocessExecute();
  void preprocessDefines(string& statement);

  //assembler
  virtual void assembleInitialize();
  bool assemblePhase();
  virtual bool assembleInstruction(Instruction& instruction);
  string text(string s);
};
