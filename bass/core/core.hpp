struct Bass {
  bool open(const string &filename);
  bool assemble(const string &filename);
  void close();

protected:
  void warning(const string &s);
  void error(const string &s);
  void assembleFile(const string &filename);
  virtual bool assembleLine(const string &line);
  virtual bool assembleBlock(const string &block);
  void setDefine(const string &name, const string &value);
  virtual void seek(unsigned offset);
  void write(uint64_t data, unsigned length = 1);

  //eval.cpp
  int64_t eval(const string &s);
  void evalMacros(string &line);
  void evalDefines(string &line);

  struct Macro {
    string name;
    lstring args;
    string value;
  };

  struct Define {
    string name;
    string value;
  };

  struct Label {
    string name;
    unsigned offset;
  };

  file output;
  enum class Endian : bool { LSB, MSB } endian;
  unsigned pass;
  unsigned offset;
  unsigned base;
  uint64_t table[256];
  linear_vector<Macro> macros;
  linear_vector<Define> defines;
  linear_vector<Label> labels;
  Macro *activeMacro;
  string activeNamespace;
  string activeLabel;
  unsigned negativeLabelCounter;
  unsigned positiveLabelCounter;
  stack_filo<string> fileName;
  stack_filo<unsigned> lineNumber;
  stack_filo<unsigned> blockNumber;
};

#include "../arch/snes-cpu/snes-cpu.hpp"
