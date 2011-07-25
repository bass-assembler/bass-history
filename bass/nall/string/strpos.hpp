#ifndef NALL_STRING_STRPOS_HPP
#define NALL_STRING_STRPOS_HPP

//usage example:
//if(auto position = strpos(str, key)) print(position(), "\n");
//prints position of key within str; but only if it is found

namespace nall {

optional<unsigned> strpos(const char *str, const char *key) {
  const char *base = str;

  while(*str) {
    for(unsigned n = 0;; n++) {
      if(key[n] == 0) return { true, (unsigned)(str - base) };
      if(str[n] != key[n]) break;
    }
    str++;
  }

  return { false, 0 };
}

optional<unsigned> istrpos(const char *str, const char *key) {
  const char *base = str;

  while(*str) {
    for(unsigned n = 0;; n++) {
      if(key[n] == 0) return { true, (unsigned)(str - base) };
      if(chrlower(str[n]) != chrlower(key[n])) break;
    }
    str++;
  }

  return { false, 0 };
}

optional<unsigned> qstrpos(const char *str, const char *key) {
  const char *base = str;

  while(*str) {
    if(*str == '\'' || *str == '\"') {
      char x = *str++;
      while(*str && *str++ != x);
      continue;
    }

    for(unsigned n = 0;; n++) {
      if(key[n] == 0) return { true, (unsigned)(str - base) };
      if(str[n] != key[n]) break;
    }
    str++;
  }

  return { false, 0 };
}

}

#endif
