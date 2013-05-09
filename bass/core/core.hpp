struct Bass {
  enum class FileMode : unsigned {
    Auto,
    Create,
    Modify,
  };

  bool open(const string& filename, FileMode mode = FileMode::Auto);
  bool assemble(const string& filename);
  void close();
  Bass();

  struct Options {
    bool overwrite;
  } options;

protected:
  enum class Condition : unsigned {
    NotYetMatched,
    Matching,
    AlreadyMatched,
  };

  struct Context {
    vector<lstring> source;
    string name;
    unsigned line;
    unsigned block;
  };

  struct Macro {
    string name;
    lstring args;
    string value;
    bool operator< (const Macro& source) const { return name <  source.name; }
    bool operator==(const Macro& source) const { return name == source.name; }
    Macro() = default;
    Macro(const string& name) : name(name) {}
    Macro(const string& name, const lstring& args, const string& value) : name(name), args(args), value(value) {}
  };

  struct Label {
    string name;
    unsigned offset = 0;
    bool operator< (const Label& source) const { return name <  source.name; }
    bool operator==(const Label& source) const { return name == source.name; }
    Label() = default;
    Label(const string& name) : name(name) {}
    Label(const string& name, unsigned offset) : name(name), offset(offset) {}
  };

  virtual void initialize(unsigned pass);
  template<typename... Args> void warning(Args&&... args);
  template<typename... Args> void error(Args&&... args);
  unsigned pc() const;
  virtual void seek(unsigned offset);
  virtual void write(uint64_t data, unsigned length = 1);
  string qualifyMacro(string name, unsigned args);
  void setMacro(string name, const lstring& args, const string& value);
  string qualifyLabel(string name);
  void setLabel(string name, unsigned offset);
  void assembleFile(const string& filename);
  unsigned macroRecursion() const;
  optional<string> assembleMacro(const string& name, const lstring& args);
  void assembleSource(const string& name, const string& source);

  //directives.cpp
  virtual bool assembleDirective(string& block);

  //intrinsics.cpp
  virtual bool assembleIntrinsic(string& block);

  //eval.cpp
  int64_t eval(const string& s);
  string evalMacros(string& line);
  optional<string> evalMacro(string name);

  //text.cpp
  string encodeText(string text);
  string decodeText(string text);

  file output;
  enum class Endian : bool { LSB, MSB } endian;
  unsigned pass;
  unsigned origin;
  signed base;
  uint64_t table[256];
  vector<string> sourceFiles;
  vector<Context> contexts;
  set<Macro> macros;
  set<Label> labels;
  string activeNamespace;
  Macro activeMacro;   //buffer used to construct new macros (not to evaluate them)
  string activeLabel;  //active label prefix for sublabels
  unsigned macroDepth;
  unsigned macroExpandCounter;
  vector<string> macroReturnStack;
  unsigned lastLabelCounter;
  unsigned nextLabelCounter;
  vector<Condition> conditionStack;
  vector<string> stack;
  bool sync;

public:
  static const bool Define;
  static const bool Function;

  vector<Macro> defaultMacros;
};
