struct BassSnesSmp : public Bass {
  bool assembleBlock(const string &block);
  BassSnesSmp();

protected:
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
};
