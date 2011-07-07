struct BassSnesSmp : public Bass {
  bool assembleBlock(const string &block);
  BassSnesSmp();

protected:
  enum Mode : unsigned {
    Implied,
    Vector,
    Direct,
    DirectBit,
    DirectBitRelative,
    Address,
    DirectXRelative,
    IndirectX,
  };

  struct Opcode {
    uint8_t byte;
    string name;
    string pattern;
    bool priority;
    Mode mode;
  };

  struct Family {
    string pattern;
    linear_vector<Opcode> opcode;
  };
  linear_vector<Family> family;
};
