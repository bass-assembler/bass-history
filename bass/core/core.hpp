struct Bass {
  bool open(const string &filename);
  bool assemble(const string &filename);
  void close();

protected:
  void warning(const string &s);
  void error(const string &s);
  unsigned pc() const;
  void assembleFile(const string &filename);
  virtual bool assembleLine(const string &line);
  virtual bool assembleBlock(const string &block);
  void setDefine(const string &name, const lstring &args, const string &value);
  void setLabel(const string &name, unsigned offset);
  virtual void seek(unsigned offset);
  virtual void write(uint64_t data, unsigned length = 1);

  struct Define {
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
  void evalDefines(string &line);
  void evalParams(string &line, Bass::Define &define, lstring &args);

  file output;
  enum class Endian : bool { LSB, MSB } endian;
  unsigned pass;
  unsigned origin;
  signed base;
  uint64_t table[256];
  linear_vector<Define> defines;
  linear_vector<Label> labels;
  Define *activeDefine;
  string activeNamespace;
  string activeLabel;
  unsigned defineExpandCounter;
  unsigned negativeLabelCounter;
  unsigned positiveLabelCounter;
  stack<unsigned> stackPC;
  stack<string> fileName;
  stack<unsigned> lineNumber;
  stack<unsigned> blockNumber;
};
