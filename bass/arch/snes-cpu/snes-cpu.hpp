struct BassSnesCpu : public Bass {
  void seek(unsigned offset);
  bool assembleBlock(const string &block);

protected:
  enum class Mapper : unsigned { Manual, LoROM, HiROM } mapper;
};
