struct BassSnesSmp : public Bass {
  bool assembleBlock(string &block);
  BassSnesSmp();

protected:
  int64_t eval(const string &s);

  enum Mode : unsigned {
    Implied,
    Vector,
    Immediate,
    Direct,
    DirectX,
    DirectY,
    DirectBit,
    DirectBitRelative,
    IndirectX,
    IndirectY,
    DirectImmediate,
    DirectDirect,
    DirectRelative,
    DirectXRelative,
    Absolute,
    AbsoluteX,
    AbsoluteY,
    AbsoluteBit,
    AbsoluteBitNot,
    IndirectAbsoluteX,
    Relative,
    YRelative,
  };

  struct Opcode {
    uint8_t prefix;
    string mnemonic;
    bool priority;
    Mode mode;
    string name;
    string pattern;
  };

  struct Family {
    string pattern;
    linear_vector<Opcode> opcode;
  };

  linear_vector<Family> family;

  //assembleBlock()
  string block, name, args;
  signed relative, address;
  bool priority;
};
