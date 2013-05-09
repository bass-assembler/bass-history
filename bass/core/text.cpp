string Bass::encodeText(string text) {
  vector<char> buffer;
  buffer.append('\"');
  unsigned x = 0, y = 0;
  for(auto& byte : text) {
    if(byte == '\\') buffer.append('\\', '\\');
    else if(byte == '\"') buffer.append('\\', 'q');
    else if(byte == '\t') buffer.append('\\', 't');
    else if(byte == '\r') buffer.append('\\', 'r');
    else if(byte == '\n') buffer.append('\\', 'n');
    else buffer.append(byte);
  }
  buffer.append('\"', 0);
  return buffer.data();
}

string Bass::decodeText(string text) {
  text.trim<1>("\"");
  unsigned x = 0, y = 0;
  while(x < text.size()) {
    if(text[x] == '\\') {
      if(text[x + 1] == '\\') text[y++] = '\\';
      else if(text[x + 1] == 'q') text[y++] = '\"';
      else if(text[x + 1] == 't') text[y++] = '\t';
      else if(text[x + 1] == 'r') text[y++] = '\r';
      else if(text[x + 1] == 'n') text[y++] = '\n';
      else warning("unrecognized escape sequence");
      x += 2;
    } else {
      text[y++] = text[x++];
    }
  }
  text.resize(y);
  return text;
}
