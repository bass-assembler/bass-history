struct BassX86 : public Bass {
  bool assembleBlock(const string &block);
  BassX86();

  enum Mode : unsigned {
    Implied,
    Register,
    AccumulatorImmediate,
    EffectiveRegister,
    RegisterEffective,
    Effective,
    EffectiveImmediate,
    Immediate,
    Relative,
    RegisterEffectiveImmediate,
  };

  enum class Flag : unsigned {
    None   = 0x00000000,
    Prefix = 0x00000001,

    RM = 0x70000000,
    R0 = 0x00000000,
    R1 = 0x10000000,
    R2 = 0x20000000,
    R3 = 0x30000000,
    R4 = 0x40000000,
    R5 = 0x50000000,
    R6 = 0x60000000,
    R7 = 0x70000000,
  };

  enum class Size : unsigned {
    None,       //size unknown
    Byte,       // 8-bit
    Word,       //16-bit
    Pair,       //16-bit or 32-bit
    Long,       //32-bit
    Quad,       //64-bit
    ExactByte,  // 8-bit only
    ExactWord,  //16-bit only
    ExactLong,  //32-bit only
    ExactQuad,  //64-bit only
  };

protected:
  struct Opcode {
    unsigned prefix;
    string mnemonic;
    Mode mode;
    Flag flag;
    Size operand0;
    Size operand1;
    Size operand2;
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
    Size os0;
    Size os1;
    Size os2;

    bool rp;
    bool mp;

    //effective address (ModR/M + SIB + displacement)
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

  void writeR(Size size);
  void writeM(Size size);
  void writePrefix(const Opcode&);
  bool isRegister(const string&) const;
  optional<unsigned> reg8(const string&) const;
  optional<unsigned> reg16(const string&) const;
  optional<unsigned> reg32(const string&) const;
  optional<unsigned> reg32m(string, optional<unsigned> &multiplier) const;
  optional<unsigned> regs(const string&) const;
  unsigned size(const string&);
  int64_t eval(const string&);
  unsigned flagR(const Opcode&);
  bool isCorrectSize(const Opcode&);
  optional<unsigned> verifyRegister(Size, const string&);
  Size detectSize(string &s);

  bool effectiveAddress(Size, const string&);
  bool reg(const Opcode&, const string&);
  void writeEffectiveAddress(const Opcode&);
};
