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

  virtual void initialize(unsigned pass);
  void warning(const string &s);
  void error(const string &s);
  unsigned pc() const;
  string qualifyMacro(string name);
  string qualifyLabel(string name);
  void assembleFile(const string &filename);
  void assembleMacro(const string &name, const lstring &args);
  void assembleSource(const string &source);
  virtual bool assembleBlock(string &block);
  void setMacro(const string &name, const lstring &args, const string &value);
  void setLabel(const string &name, unsigned offset);
  virtual void seek(unsigned offset);
  virtual void write(uint64_t data, unsigned length = 1);

  //eval.cpp
  int64_t eval(const string &s);
  bool evalMacros(string &block);
  void evalDefines(string &line);
  string evalDefine(string &name);

  file output;
  enum class Endian : bool { LSB, MSB } endian;
  unsigned pass;
  unsigned origin;
  signed base;
  uint64_t table[256];
  vector<Macro> macros;
  vector<Label> labels;
  string activeNamespace;
  Macro activeMacro;   //buffer used to construct new macros (not to evaluate them)
  string activeLabel;  //active label prefix for sublabels
  unsigned macroDepth;
  unsigned macroExpandCounter;
  unsigned macroRecursionCounter;
  unsigned lastLabelCounter;
  unsigned nextLabelCounter;
  Condition condition;
  vector<Condition> conditionStack;
  vector<string> stack;

  //these vectors preserve state across file incsrc recursion
  vector<string> fileName;
  vector<unsigned> lineNumber;
  vector<unsigned> blockNumber;
  vector<string> activeLine;

public:
  vector<Macro> defaultMacros;
};
