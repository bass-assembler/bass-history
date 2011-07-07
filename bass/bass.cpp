#include "bass.hpp"
#include "core/core.cpp"
#include "arch/snes-cpu/snes-cpu.cpp"
#include "arch/snes-smp/snes-smp.cpp"

#if defined(BASS_BINARY)

int main(int argc, char **argv) {
  string outputFilename;
  lstring inputFilename;

  Bass *arch = 0;

  for(unsigned n = 1; n < argc;) {
    if(0);
    else if(!arch && !strcmp(argv[n], "-arch=snes-cpu")) {
      arch = new BassSnesCpu;
      n++;
    } else if(!arch && !strcmp(argv[n], "-arch=snes-smp")) {
      arch = new BassSnesSmp;
      n++;
    } else if(!strcmp(argv[n], "-o") && n + 1 < argc) {
      outputFilename = argv[n + 1];
      n += 2;
    } else if(argv[n][0] != '-') {
      inputFilename.append(argv[n]);
      n++;
    } else {
      print("unknown argument: ", argv[n], "\n");
      n++;
    }
  }

  if(!arch || outputFilename == "" || inputFilename.size() < 1) {
    print("bass v00.03\n");
    print("author: byuu\n");
    print("usage: bass -arch=(arch) -o output input [input ...]\n\n");
    print("supported archs:\n");
    print("  snes-cpu\n");
    print("  snes-smp\n");
    return 0;
  }

  arch->open(outputFilename);
  foreach(filename, inputFilename) arch->assemble(filename);
  arch->close();
  delete arch;

  return 0;
}

#endif

