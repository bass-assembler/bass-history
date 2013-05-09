#include <nall/platform.hpp>
#include <nall/any.hpp>
#include <nall/base64.hpp>
#include <nall/bmp.hpp>
#include <nall/compositor.hpp>
#include <nall/config.hpp>
#include <nall/directory.hpp>
#include <nall/dl.hpp>
#include <nall/dsp.hpp>
#include <nall/file.hpp>
#include <nall/filemap.hpp>
#include <nall/function.hpp>
#include <nall/group.hpp>
#include <nall/gzip.hpp>
#include <nall/http.hpp>
#include <nall/image.hpp>
#include <nall/inflate.hpp>
#include <nall/input.hpp>
#include <nall/intrinsics.hpp>
#include <nall/invoke.hpp>
#include <nall/ips.hpp>
#include <nall/map.hpp>
#include <nall/matrix.hpp>
#include <nall/mosaic.hpp>
#include <nall/png.hpp>
#include <nall/priority-queue.hpp>
#include <nall/property.hpp>
#include <nall/public-cast.hpp>
#include <nall/random.hpp>
#include <nall/serializer.hpp>
#include <nall/set.hpp>
#include <nall/stdint.hpp>
#include <nall/stream.hpp>
#include <nall/string.hpp>
#include <nall/thread.hpp>
#include <nall/traits.hpp>
#include <nall/udl.hpp>
#include <nall/unzip.hpp>
#include <nall/utility.hpp>
#include <nall/varint.hpp>
#include <nall/vector.hpp>
#include <nall/zip.hpp>
#include <nall/beat/delta.hpp>
#include <nall/beat/linear.hpp>
#include <nall/beat/metadata.hpp>
#include <nall/beat/patch.hpp>
#include <nall/beat/multi.hpp>
#include <nall/beat/archive.hpp>
#include <nall/stream/stream.hpp>
#include <nall/stream/memory.hpp>
#include <nall/stream/vector.hpp>
#ifdef _WIN32
  #include <nall/windows/guid.hpp>
  #include <nall/windows/registry.hpp>
  #include <conio.h>
#endif
using namespace nall;

#if 1
int main() {
  clock_t s = clock();
  auto document = Markup::Document(string::read("/usr/share/higan/cheats.bml"));
  print(document["database/revision"].text(), "\n");
//for(auto& cartridge : document.find("cartridge")) print(cartridge["name"].text(), "\n");
  clock_t f = clock();
  print(f - s, "\n");
  return 0;
}
#endif

#if 0
#include <set>
#include <unordered_set>

int main() {
  random_lfsr r;
  set<unsigned> tree1;
  std::set<unsigned> tree2;
  clock_t start, finish;

  start = clock();
  r.seed(0x5aa5f00f);
  for(unsigned n = 0; n < 4 * 1024 * 1024; n++) tree1.insert(r());
  finish = clock();
  print(finish - start, "\n");

  start = clock();
  r.seed(0x5aa5f00f);
  for(unsigned n = 0; n < 4 * 1024 * 1024; n++) tree2.insert(r());
  finish = clock();
  print(finish - start, "\n");

  return 0;
}
#endif

#if 0
int main(int argc, char** argv) {
clock_t s = clock();
for(unsigned n = 0; n < 1; n++) {
  auto document = BML::Document(string::read("document.bml"));
  if(document.error) return print(document.error, "\n"), 0;

//auto result = document.find("*/spc7110(revision=2)/rom(=Q,id=data,size=0x400000)");
  auto result = document.find("*/spc7110(revision<3)[1]/rom[1-2]");
  if(1) { for(auto& node : result) print(node.name, "=", node["name"].data, "\n"); print("\n"); }

  if(1) for(auto& root : document) {
    print("{", root.name, "}");
    if(!root.data.empty()) print("={", root.data, "}");
    print("\n");
    for(auto& node : root) {
      print("  {", node.name, "}");
      if(!node.data.empty()) print("={", node.data, "}");
      print("\n");
      for(auto& leaf : node) {
        print("    {", leaf.name, "}");
        if(!leaf.data.empty()) print("={", leaf.data, "}");
        print("\n");
        for(auto& last : leaf) {
          print("      {", last.name, "}");
          if(!last.data.empty()) print("={", last.data, "}");
          print("\n");
        }
      }
    }
  }
}
clock_t f = clock();
print(f-s, "\n");

  return 0;
}
#endif
