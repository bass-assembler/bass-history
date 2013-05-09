bool Bass::assembleIntrinsic(string& block) {
  if(block == "@pc") {
    block = hex(pc());
    return true;
  }

  if(block == "@origin") {
    block = {"0x", hex(origin)};
    return true;
  }

  if(block.match("@defined ?*")) {
    block.ltrim<1>("@defined ");
    block = qualifyMacro(block, 0);
    block = macros.find(block) ? "1" : "0";
    return true;
  }

  if(block.match("@eval ?*")) {
    block.ltrim<1>("@eval ");
    evalMacros(block);
    block = eval(block);
    return true;
  }

  if(block.match("@hex ?*")) {
    block.ltrim<1>("@hex ");
    evalMacros(block);
    block = {"0x", hex(eval(block))};
    return true;
  }

  //integer {@compare string, string}
  if(block.beginswith("@compare ")) {
    lstring args = block.ltrim<1>("@compare ").qsplit(",").strip();
    if(args.size() != 2) error("incorrect argument count");
    signed result = strcmp(decodeText(args(0)), decodeText(args(1)));
    block = result;
    return true;
  }

  //integer {@find string, needle}
  if(block.beginswith("@find ")) {
    lstring args = block.ltrim<1>("@find ").qsplit(",").strip();
    if(args.size() != 2) error("incorrect argument count");
    string text = decodeText(args(0));
    string needle = decodeText(args(1));
    auto result = text.find(needle);
    block = result ? (signed)result() : -1;
    return true;
  }

  //integer {@get string, offset}
  if(block.beginswith("@get ")) {
    lstring args = block.ltrim<1>("@get ").qsplit(",").strip();
    if(args.size() != 2) error("incorrect argument count");
    string text = decodeText(args(0));
    unsigned offset = eval(args(1));
    if(offset < text.size()) block = text[offset];
    else block = "0";
    return true;
  }

  //integer {@length string}
  if(block.beginswith("@length ")) {
    lstring args = block.ltrim<1>("@length ").strip();
    if(args.size() != 1) error("incorrect argument count");
    string text = decodeText(args(0));
    unsigned length = text.size();
    block = length;
    return true;
  }

  //string {@ltrim string, needle}
  if(block.beginswith("@ltrim ")) {
    lstring args = block.ltrim<1>("@ltrim ").qsplit(",").strip();
    if(args.size() != 2) error("incorrect argument count");
    string text = decodeText(args(0));
    string needle = decodeText(args(1));
    text.ltrim(needle);
    block = encodeText(text);
    return true;
  }

  //integer {@match string, needle}
  if(block.beginswith("@match ")) {
    lstring args = block.ltrim<1>("@match ").qsplit(",").strip();
    if(args.size() != 2) error("incorrect argument count");
    string text = decodeText(args(0));
    string needle = decodeText(args(1));
    bool result = text.match(needle);
    block = result ? "1" : "0";
    return true;
  }

  //string {@replace string, from, to}
  if(block.beginswith("@replace ")) {
    lstring args = block.ltrim<1>("@replace ").qsplit(",").strip();
    if(args.size() != 3) error("incorrect argument count");
    string text = decodeText(args(0));
    string from = decodeText(args(1));
    string to = decodeText(args(2));
    text.replace(from, to);
    block = encodeText(text);
    return true;
  }

  //string {@rtrim string, needle}
  if(block.beginswith("@rtrim ")) {
    lstring args = block.ltrim<1>("@rtrim ").qsplit(",").strip();
    if(args.size() != 2) error("incorrect argument count");
    string text = decodeText(args(0));
    string needle = decodeText(args(1));
    text.rtrim(needle);
    block = encodeText(text);
    return true;
  }

  //string {@set string, offset, integer}
  if(block.beginswith("@set ")) {
    lstring args = block.ltrim<1>("@set ").qsplit(",").strip();
    if(args.size() != 3) error("incorrect argument count");
    string text = decodeText(args(0));
    unsigned offset = eval(args(1));
    char value = eval(args(2));
    if(offset < text.size()) text[offset] = value;
    block = encodeText(text);
    return true;
  }

  //string {@slice string, offset, length}
  if(block.beginswith("@slice ")) {
    lstring args = block.ltrim<1>("@slice ").qsplit(",").strip();
    if(args.size() != 3) error("incorrect argument count");
    string text =decodeText(args(0));
    unsigned offset = eval(args(1));
    unsigned length = eval(args(2));
    text = text.slice(offset, length);
    block = encodeText(text);
    return true;
  }

  //string {@transform string, from, to}
  if(block.beginswith("@transform ")) {
    lstring args = block.ltrim<1>("@transform ").qsplit(",").strip();
    if(args.size() != 3) error("incorrect argument count");
    string text = decodeText(args(0));
    string from = decodeText(args(1));
    string to = decodeText(args(2));
    text.transform(from, to);
    block = encodeText(text);
    return true;
  }

  return false;
}
