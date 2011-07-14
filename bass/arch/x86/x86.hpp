struct BassX86 : public Bass {
  bool assembleBlock(const string &block);
  BassX86();

protected:
  enum Mode : unsigned {
    Implied,
    AccumulatorImmediateWord,
    AccumulatorImmediateByte,
    EffectiveWordRegister,
    EffectiveByteRegister,
    RegisterEffectiveWord,
    RegisterEffectiveByte,
    EffectiveWord,
    EffectiveByte,
    EffectiveWordImmediateByte,
    EffectiveWordImmediate,
    EffectiveByteImmediate,
    ImmediateWord,
    ImmediateByte,
    RegisterWord,
    RelativeWord,
    RelativeByte,
  };

  enum Flag : unsigned {
    None     = 0x00000000,
    Prefix   = 0x00000001,
    Priority = 0x00000002,
    Byte     = 0x00000004,
    Word     = 0x00000008,
  };

  struct Opcode {
    unsigned prefix;
    string mnemonic;
    Mode mode;
    Flag flag;
    unsigned param0;
    string name;
    string pattern;
  };

  struct Family {
    string pattern;
    linear_vector<Opcode> opcode;
  };

  linear_vector<Family> family;

  struct CPU {
    unsigned bits;  //16, 32
  } cpu;

  struct Info {
    unsigned ps;  //prefix size
    bool rp;
    bool mp;
    unsigned seg;
    unsigned mod;
    unsigned r;
    unsigned m;
    bool sib;
    unsigned multiplier;
    unsigned r0;
    unsigned r1;
    unsigned ds;
    unsigned dd;
  } info;

  void writePrefix(const Opcode&);
  bool isRegister(const string&) const;
  optional<unsigned> reg8(const string&) const;
  optional<unsigned> reg16(const string&) const;
  optional<unsigned> reg32(const string&) const;
  optional<unsigned> reg32m(string, optional<unsigned> &multiplier) const;
  unsigned size(const string&);
  int64_t eval(const string&);

  bool effectiveAddress(const Opcode&, string);
  bool reg(const Opcode&, const string&);
  void writeEffectiveAddress(const Opcode&);
};
