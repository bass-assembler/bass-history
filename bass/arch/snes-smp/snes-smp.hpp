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
    DirectBit,
    DirectBitRelative,
    IndirectX,
    IndirectY,
    DirectImmediate,
    DirectDirect,
    DirectRelative,
    Absolute,
    AbsoluteX,
    AbsoluteY,
    AbsoluteBit,
    AbsoluteBitNot,
    IndirectAbsoluteX,
    Relative,
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
