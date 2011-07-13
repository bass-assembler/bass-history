#include <tuple>
using std::tuple;
using std::get;

struct BassX86 : public Bass {
  bool assembleBlock(const string &block);
  BassX86();

protected:
  enum Mode : unsigned {
    Implied,
    ModRM8,
    ModRM32,
    ModRM8Reverse,
    ModRM32Reverse,
    EA8,
    EA32,
    EA8_IMM8,
    EA32_IMM32,
    Immediate8,
    Immediate16,
    Immediate32,
    Immediate8S,
    Immediate16S,
    Immediate32S,
    R32,
    Relative8,
    Relative16,
    Relative32,
  };

  enum Flag : unsigned {
    None     = 0x00000000,
    Prefix   = 0x00000001,
    Priority = 0x00000002,
    Short    = 0x00000004,
    Long     = 0x00000008,
    ModR     = 0x00000010,
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

  unsigned addr;

  struct Info {
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

  unsigned prefixLength(const Opcode&) const;
  void writePrefix(const Opcode&);
  bool isRegister(const string&) const;
  optional<unsigned> reg8(const string&) const;
  optional<unsigned> reg16(const string&) const;
  optional<unsigned> reg32(const string&) const;
  optional<unsigned> reg32m(string, optional<unsigned> &multiplier) const;
  tuple<unsigned, unsigned> size(const string&);

  bool effectiveAddress(const Opcode&, string);
  bool reg(const Opcode&, const string&);
  void writeEffectiveAddress(const Opcode&);
};
