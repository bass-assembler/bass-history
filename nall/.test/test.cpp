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

struct Settings : Configuration::Document {
  double version;

  struct Video : Configuration::Node {
    string driver;
    bool synchronize;
    struct MaskOverscan : Configuration::Node {
      bool enable;
      unsigned horizontal;
      unsigned vertical;
    } maskOverscan;
    bool startFullScreen;
  } video;

  struct Audio : Configuration::Node {
    string driver;
    bool mute;
  } audio;

  Settings() {
    append(version = 3.2, "Version", "version");

    video.append(video.driver = "OpenGL", "Driver");
    video.append(video.synchronize = false, "Synchronize", "synchronize to vertical retrace");
    video.maskOverscan.assign(video.maskOverscan.enable = true);
    video.maskOverscan.append(video.maskOverscan.horizontal = 8, "Horizontal");
    video.maskOverscan.append(video.maskOverscan.vertical = 8, "Vertical");
    video.append(video.maskOverscan, "MaskOverscan");
    video.append(video.startFullScreen = true, "StartFullScreen");
    append(video, "Video");

    audio.append(audio.driver = "ALSA", "Driver");
    audio.append(audio.mute = false, "Mute", "silence audio output");
    append(audio, "Audio", "audio settings");

    load("settings.bml");
    save("settings.bml");

    print(video.driver, "\n");
    print(video.synchronize, "\n");
    print(audio.driver, "\n");
    print(audio.mute, "\n");
  }
} settings;

int main() {
  lstring list = string{" test 1 , test 2 , test 3 "}.split(",").strip();
  list.strip();
  for(auto &i : list) print(i, "!\n");
  return 0;
}

#if 0
int main(int argc, char **argv) {
  auto document = BML::Document(string::read("document.bml"));
  if(document.error) return print(document.error, "\n"), 0;

//auto result = document.find("*/spc7110(revision=2)/rom(=Q,id=data,size=0x400000)");
  auto result = document.find("*/spc7110(revision<3)[1]/rom[1-2]");
  for(auto &node : result) print(node.name, "=", node["name"].data, "\n");
  print("\n");

  for(auto &root : document) {
    print("{", root.name, "}");
    if(!root.data.empty()) print("={", root.data, "}");
    print("\n");
    for(auto &node : root) {
      print("  {", node.name, "}");
      if(!node.data.empty()) print("={", node.data, "}");
      print("\n");
      for(auto &leaf : node) {
        print("    {", leaf.name, "}");
        if(!leaf.data.empty()) print("={", leaf.data, "}");
        print("\n");
        for(auto &last : leaf) {
          print("      {", last.name, "}");
          if(!last.data.empty()) print("={", last.data, "}");
          print("\n");
        }
      }
    }
  }

  return 0;
}
#endif
