// zlib License
//
// copyright (C) 2023 Guoxiaomi and Krimiston
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once
#include "lib/lib.hpp"

#ifndef RGM_BUILDMODE
#define RGM_BUILDMODE 1
#endif

#if RGM_BUILDMODE >= 2
#define RGM_EMBEDED_ZIP "embeded.zip"
#endif

#ifndef RGM_FULLVERSION
#define RGM_FULLVERSION "RGM_FULLVERSION"
#endif

#ifndef CC_VERSION
#define CC_VERSION "CC_VERSION"
#endif

namespace rgm::config {
using section_t =
    std::map<std::string, std::variant<std::monostate, bool, int, std::string>>;

/* constexprs */
constexpr std::string_view config_path = "./config.ini";
constexpr int build_mode = RGM_BUILDMODE;
constexpr bool develop = (RGM_BUILDMODE < 2);
constexpr bool concurrent = false;
constexpr int controller_axis_threshold = 8000;
constexpr int max_workers = 8;
constexpr int tileset_texture_height = 8192;
constexpr cen::color screen_background_color = cen::colors::black;
constexpr cen::pixel_format texture_format = cen::pixel_format::bgra32;
constexpr cen::pixel_format surface_format = cen::pixel_format::rgba32;

/* 从命令行参数中读取的设置 */
bool btest = false;
bool debug = false;

/* 从 config.ini 中读取的设置 */
bool synchronized = true;
bool controller_left_arrow = true;
bool controller_right_arrow = true;
std::string game_title = "RGModern";
std::string resource_prefix = "resource://";
int window_width = 640;
int window_height = 480;
int screen_width = 640;
int screen_height = 480;

/* 支持的 driver 的类型 */
enum class driver_type { software, opengl, direct3d9, direct3d11 };

#ifdef __WIN32
std::string driver_name = "direct3d9";
driver_type driver = driver_type::direct3d9;
#else
std::string driver_name = "opengl";
driver_type driver = driver_type::opengl;
#endif

/// @brief 读取数据，设置 config 中各变量
/// @param data config.ini 转换后的 map 数据
void load_data(std::map<std::string, section_t>& data) {
#define Set(item, section, key)                                \
  if (data[section][key].index() != 0) {                       \
    try {                                                      \
      item = std::get<decltype(item)>(data[section][key]);     \
    } catch (std::bad_variant_access const&) {                 \
      cen::message_box::show(                                  \
          "RGModern",                                          \
          "The invalid value of " #key " in section " #section \
          " is ignored. \nRemember to check the config.ini "   \
          "next time!",                                        \
          cen::message_box_type::error);                       \
    }                                                          \
  }

  Set(game_title, "Game", "Title");
  Set(synchronized, "Kernel", "Synchronization");
  Set(controller_left_arrow, "Kernel", "LeftAxisArrow");
  Set(controller_right_arrow, "Kernel", "RightAxisArrow");
  Set(resource_prefix, "Kernel", "ResourcePrefix");
  Set(window_width, "System", "WindowWidth");
  Set(window_height, "System", "WindowHeight");
  Set(screen_width, "System", "ScreenWidth");
  Set(screen_height, "System", "ScreenHeight");
#ifdef __WIN32
  Set(driver_name, "Kernel", "RenderDriver");
#endif
#undef Set

  /* 将 driver_name 转换成小写 */
  std::transform(driver_name.begin(), driver_name.end(), driver_name.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  if (driver_name == "software") driver = driver_type::software;
  if (driver_name == "opengl") driver = driver_type::opengl;
  if (driver_name == "direct3d9") driver = driver_type::direct3d9;
  if (driver_name == "direct3d11") driver = driver_type::direct3d11;
}

/// @brief 读取命令行参数
/// @param argc 命令行参数 ARGC
/// @param argv 命令行参数 ARGV
/// @return false 表示不启动游戏引擎，直接退出，true 表示正常运行
bool load_args(int argc, char* argv[]) {
  if (argc == 2 && strncmp(argv[1], "-v", 2) == 0) {
    /* 显示版本信息 */
    printf("RGM %s [BuildMode = %d]\n\n", RGM_FULLVERSION, RGM_BUILDMODE);
    printf(
        "Modern Ruby Game Engine (RGM) is licensed under the zlib "
        "License.\n");
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
    printf(
        " - centurion, concurrentqueue, incbin, xorstr, libzip, "
        "etc.\n");
    return false;
  }

  /* 读取命令行参数并设置相应的全局变量 */
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

/// @brief 将 ini 中的数据读取到 map 中
void load_ini() {
  if (!std::filesystem::exists(config_path.data())) return;

  std::map<std::string, section_t> data;
  section_t* p_section = nullptr;
  std::array<char, 1024> line{};

  std::ifstream ifs(config_path.data(), std::ios::in);
  while (ifs.getline(line.data(), line.size())) {
    /* [XXX] 模式表示新的 section */
    if (line[0] == '[') {
      std::string section_name(line.data() + 1,
                               strchr(line.data(), ']') - line.data() - 1);
      data[section_name] = {};
      p_section = &data[section_name];
    }

    char* equal = strchr(line.data(), '=');
    if (equal) {
      /* A=B 模式表示一个键值对 */
      int pos_equal = equal - line.data();

      std::string key(line.data(), pos_equal);
      std::string value(equal + 1, strlen(line.data()) - pos_equal - 1);

      /* 根据不同的值设置为不同类型的 value，存储到 map 中 */
      if (p_section) {
        if (value == "ON") {
          p_section->insert_or_assign(key, true);
        } else if (value == "OFF") {
          p_section->insert_or_assign(key, false);
        } else if (std::isdigit(value[0])) {
          p_section->insert_or_assign(key, std::stoi(value));
        } else if (value[0] == '-' && std::isdigit(value[1])) {
          p_section->insert_or_assign(key, std::stoi(value));
        } else {
          p_section->insert_or_assign(key, value);
        }
      }
    }

    /* 重置 line，准备读取下一行 */
    line.fill(0);
  }

  load_data(data);
}
}  // namespace rgm::config
