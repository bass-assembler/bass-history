struct BassSnesCpuCanonical : public Bass {
  void seek(unsigned offset);
  bool assembleBlock(const string &block);

protected:
  enum class Mapper : unsigned { None, LoROM, HiROM } mapper;
};
