//bass
//author: byuu
//license: GPLv3

#include "bass.hpp"
#include "core/core.cpp"
#include "arch/table/table.cpp"
#include "arch/snes-smp-canonical/snes-smp-canonical.cpp"

#if defined(BASS_BINARY)

int main(int argc, char **argv) {
  string outputFilename;
  lstring inputFilename;

  Bass *arch = new BassTable;
  bool benchmark = false;

  #if 0
  char argv1[] = "bass";
  char argv2[] = "-o";
  char argv3[] = "test/test.bin";
  char argv4[] = "test/test.asm";
  char *argvN[] = {argv1, argv2, argv3, argv4, nullptr};
  argc = 4;
  argv = (char**)argvN;
  #endif

  for(unsigned n = 1; n < argc;) {
    if(0) {
    } else if(cstring{argv[n]} == "-arch=table") {
      delete arch;
      arch = new BassTable;
      n++;
    } else if(cstring{argv[n]} == "-arch=snes-smp-canonical") {
      delete arch;
      arch = new BassSnesSmpCanonical;
      n++;
    } else if(strbegin(argv[n], "-D")) {
      string argument = argv[n];
      argument.ltrim<1>("-D");
      lstring part = argument.split<1>("=");
      arch->defaultMacros.append({
        {"global::", part[0]}, {}, part(1, "1")  //no argument sets value to true (1)
      });
      n++;
    } else if(cstring{argv[n]} == "-case-insensitive") {
      arch->options.caseInsensitive = true;
      n++;
    } else if(cstring{argv[n]} == "-overwrite") {
      arch->options.overwrite = true;
      n++;
    } else if(cstring{argv[n]} == "-benchmark") {
      benchmark = true;
      n++;
    } else if(cstring{argv[n]} == "-o" && n + 1 < argc) {
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

  if(outputFilename.empty() || inputFilename.size() < 1) {
    print("bass v09.02\n");
    print("author: byuu\n");
    print("usage: bass [-arch=name] [-Dname(=value) ...] [options] -o output input [input ...]\n\n");
    print("supported archs:\n");
    print("  table (default)\n");
    print("  snes-smp-canonical\n");
    print("\n");
    print("supported options:\n");
    print("  -case-insensitive\n");
    print("  -overwrite\n");
    print("  -benchmark\n");
    print("\n");
    return EXIT_FAILURE;
  }

  if(arch->open(outputFilename) == false) {
    print("error: unable to open output file ", outputFilename, "\n");
    return EXIT_FAILURE;
  }

  bool success = true;
  clock_t startTime = clock();
  for(auto &filename : inputFilename) success &= arch->assemble(filename);
  clock_t endTime = clock();
  if(benchmark) print("Assembled in ", (endTime - startTime) / (double)CLOCKS_PER_SEC, " seconds.\n");

  arch->close();
  delete arch;

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif
