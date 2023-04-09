// zlib License

// copyright (C) 2023 Guoxiaomi and Krimiston

// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.

// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:

// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once
#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <string_view>
#include <variant>

#ifndef RGM_BUILDMODE
#define RGM_BUILDMODE 1
#endif

#if RGM_BUILDMODE >= 2
#define RGM_EMBEDED_ZIP
#endif

#ifndef RGM_FULLVERSION
#define RGM_FULLVERSION "RGM_FULLVERSION"
#endif

#ifndef CC_VERSION
#define CC_VERSION "CC_VERSION"
#endif

extern "C" {
void ruby_show_version();
}

// #include "shader/driver.hpp"
namespace rgm::base {
void setup_driver(const std::string_view name);
}

namespace rgm::config {
// constexprs
constexpr std::string_view config_path = "./config.ini";
constexpr int build_mode = RGM_BUILDMODE;
constexpr bool develop = (RGM_BUILDMODE < 2);
constexpr int controller_axis_threshold = 8000;
constexpr int max_threads = 8;
constexpr int tileset_texture_height = 8192;

// configs from command line args
bool btest = false;
bool debug = false;

// configs from config.ini
std::string game_title = "RGModern";
bool asynchronized = false;
std::string resource_prefix = "resource://";
int window_width = 640;
int window_height = 480;
int screen_width = 640;
int screen_height = 480;

bool load_args(int argc, char* argv[]) {
  if (argc == 2 && strncmp(argv[1], "-v", 2) == 0) {
    printf("RGM %s [BuildMode = %d]\n\n", RGM_FULLVERSION, RGM_BUILDMODE);
    printf("Modern Ruby Game Engine (RGM) is licensed under zlib License.\n");
    printf("copyright (C) 2023 Guoxiaomi and Krimiston\n\n");
    printf("Repository: https://github.com/gxm11/RGModern\n\n");
    printf("Compiler: %s\n\n", CC_VERSION);
    printf("Libraries:\n - ");
    ruby_show_version();
#define GETVERSION(x) \
  SDL_##x##_MAJOR_VERSION, SDL_##x##_MINOR_VERSION, SDL_##x##_PATCHLEVEL
    printf(" - SDL %d.%d.%d\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION,
           SDL_PATCHLEVEL);
    printf(" - SDL Image %d.%d.%d\n", GETVERSION(IMAGE));
    printf(" - SDL TTF %d.%d.%d\n", GETVERSION(TTF));
    printf(" - SDL Mixer %d.%d.%d\n", GETVERSION(MIXER));
#undef GETVERSION
    printf(" - centurion, concurrentqueue, incbin, xorstr, libzip, etc.\n");
    return false;
  }

  // load configs from argv
  for (int i = 0; i < argc; ++i) {
    if (strncmp(argv[i], "btest", 6) == 0) {
      rgm::config::btest = true;
    }
    if (strncmp(argv[i], "debug", 6) == 0) {
      rgm::config::debug = true;
    }
  }
  return true;
}

void load_ini() {
  if (!std::filesystem::exists(config_path.data())) return;

  using section_t = std::map<std::string, std::variant<bool, int, std::string>>;

  std::map<std::string, section_t> data;
  section_t* p_section = nullptr;
  std::array<char, 1024> line{};

  std::ifstream ifs(config_path.data(), std::ios::in);
  while (ifs.getline(line.data(), line.size())) {
    if (line[0] == '[') {
      // new section
      std::string section_name(line.data() + 1,
                               strchr(line.data(), ']') - line.data() - 1);
      data[section_name] = {};
      p_section = &data[section_name];
    }

    char* equal = strchr(line.data(), '=');
    if (equal) {
      int pos_equal = equal - line.data();
      // key-value pair
      std::string key(line.data(), pos_equal);
      std::string value(equal + 1, strlen(line.data()) - pos_equal - 1);
      if (p_section) {
        if (value == "ON") {
          p_section->insert_or_assign(key, true);
        } else if (value == "OFF") {
          p_section->insert_or_assign(key, false);
        } else if (std::isdigit(value[0])) {
          p_section->insert_or_assign(key, std::stoi(value));
        } else {
          p_section->insert_or_assign(key, value);
        }
      }
    }

    line.fill(0);
  }

  game_title = std::get<std::string>(data["Game"]["Title"]);
  asynchronized = !std::get<bool>(data["Kernel"]["Synchronization"]);
  resource_prefix = std::get<std::string>(data["Kernel"]["ResourcePrefix"]);
  window_width = std::get<int>(data["System"]["WindowWidth"]);
  window_height = std::get<int>(data["System"]["WindowHeight"]);
  screen_width = std::get<int>(data["System"]["ScreenWidth"]);
  screen_height = std::get<int>(data["System"]["ScreenHeight"]);
  base::setup_driver(std::get<std::string>(data["Kernel"]["RenderDriver"]));
}
}  // namespace rgm::config
