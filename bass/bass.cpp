//bass v12-beta1
//license: GPLv3
//author: byuu
//project started: 2013-09-27

#include <nall/nall.hpp>
using namespace nall;

#include "core/core.hpp"
#include "core/core.cpp"

#include "arch/table/table.hpp"
#include "arch/table/table.cpp"

int main(int argc, char** argv) {
  if(argc == 1) {
    print("bass v12-beta1\n");
    print("usage: bass [-overwrite] [-o target] source [source ...]\n");
    return 0;
  }

  string targetFilename;
  bool overwrite = false;
  lstring sourceFilenames;

  for(unsigned n = 1; n < argc;) {
    string s = argv[n];

    if(s == "-o") {
      targetFilename = argv[n + 1];
      n += 2;
      continue;
    }

    if(s == "-overwrite") {
      overwrite = true;
      n += 1;
      continue;
    }

    if(!s.beginswith("-")) {
      sourceFilenames.append(s);
      n += 1;
      continue;
    }

    print("error: unrecognized argument: ", s, "\n");
    return -1;
  }

  BassTable bass;
  bass.target(targetFilename, overwrite);
  for(auto& sourceFilename : sourceFilenames) {
    bass.source(sourceFilename);
  }
  if(bass.preprocess() == false) return -1;
  if(bass.assemble() == false) return -1;
  return 0;
}
