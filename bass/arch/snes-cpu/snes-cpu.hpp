struct BassSnesCpu : public Bass {
  int64_t eval(const string &s);
  void seek(unsigned offset);
  bool assembleBlock(string &block);
  BassSnesCpu();

protected:
  enum Mapper : unsigned { None, LoROM, HiROM } mapper;

  enum Mode : unsigned {
    Implied,
    ImpliedRepeat,
    Immediate,
    ImmediateM,
    ImmediateX,
    Direct,
    DirectX,
    DirectY,
    DirectS,
    Indirect,
    IndirectX,
    IndirectY,
    IndirectSY,
    IndirectLong,
    IndirectLongY,
    Absolute,
    AbsoluteX,
    AbsoluteY,
    IndirectAbsolute,
    IndirectAbsoluteX,
    IndirectLongAbsolute,
    Long,
    LongX,
    Relative,
    RelativeLong,
    BlockMove,
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
    vector<Opcode> opcode;
  };

  vector<Family> family;
};
