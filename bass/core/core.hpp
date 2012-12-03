struct Bass {
  bool open(const string &filename);
  bool assemble(const string &filename);
  void close();
  Bass();

  struct Options {
    bool caseInsensitive;
    bool overwrite;
  } options;

protected:
  virtual void initialize(unsigned pass);
  void warning(const string &s);
  void error(const string &s);
  unsigned pc() const;
  void assembleFile(const string &filename);
  virtual bool assembleBlock(const string &block);
  void setMacro(const string &name, const lstring &args, const string &value);
  void setLabel(const string &name, unsigned offset);
  virtual void seek(unsigned offset);
  virtual void write(uint64_t data, unsigned length = 1);

  enum class Condition : unsigned { NotYetMatched, Matching, AlreadyMatched };

  struct Macro {
    string name;
    lstring args;
    string value;
  };

  struct Label {
    string name;
    unsigned offset;
  };

  //eval.cpp
  int64_t eval(const string &s);
  void evalMacros(string &line);
  void evalParams(string &line, Macro &macro, lstring &args);

  file output;
  enum class Endian : bool { LSB, MSB } endian;
  unsigned pass;
  unsigned origin;
  signed base;
  uint64_t table[256];
  vector<Macro> macros;
  vector<Label> labels;
  Macro activeMacro;
  string activeNamespace;
  string activeLabel;
  unsigned macroDepth;
  unsigned macroExpandCounter;
  unsigned lastLabelCounter;
  unsigned nextLabelCounter;
  Condition condition;
  vector<Condition> conditionStack;
  vector<unsigned> originStack;
  vector<signed> baseStack;
  vector<string> fileName;
  vector<unsigned> lineNumber;
  vector<unsigned> blockNumber;

public:
  vector<Macro> defaultMacros;
};
