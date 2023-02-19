// Copyright (c) 2022 Xiaomi Guo
// Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan
// PSL v2. You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
// Mulan PSL v2 for more details.
#pragma once
#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>

namespace rgm::config {
// configs
bool btest = false;
bool debug = false;

const char* game_title = "RGModern";
const char* config_path = "./config.ini";

// constexprs
constexpr int window_width = 640;
constexpr int window_height = 480;

#ifndef RGM_BUILDMODE
#define RGM_BUILDMODE 1
#endif

#if RGM_BUILDMODE >= 2
constexpr int output_level = 0;
constexpr bool check_renderstack = false;
#define RGM_EMBEDED_ZIP
#else
constexpr int output_level = 1;
constexpr bool check_renderstack = true;
#endif

constexpr int build_mode = RGM_BUILDMODE;
static bool asynchronized = false;
static std::string resource_prefix = "resource://";

enum class drivers {
  software,
  opengl,
  direct3d9,
  direct3d11,
};

const char* to_string(drivers d) {
  switch (d) {
    default:
    case drivers::software:
      return "software";
    case drivers::opengl:
      return "opengl";
    case drivers::direct3d9:
      return "direct3d9";
    case drivers::direct3d11:
      return "direct3d11";
  }
}

static drivers driver_type = drivers::direct3d11;

void load(int argc, char* argv[]) {
  // load configs from argv
  for (int i = 0; i < argc; ++i) {
    if (strncmp(argv[i], "btest", 6) == 0) {
      rgm::config::btest = true;
    }
    if (strncmp(argv[i], "debug", 6) == 0) {
      rgm::config::debug = true;
    }
  }

  // load config from ini
  enum class root { game, system, keymap, font, kernel };

#define CHECK_ROOT(key) \
  strncmp(line.data(), "[" #key "]", sizeof(#key) + 1) == 0

#define GET_ITEM(key)                               \
  strncmp(line.data(), #key "=", sizeof(#key)) == 0 \
      ? line.data() + sizeof(#key)                  \
      : 0
#define CHECK_ITEM(value1, value2) \
  strncmp(value1, value2, sizeof(value2) - 1) == 0

  if (!std::filesystem::exists(config_path)) return;

  root t = root::game;
  std::array<char, 1024> line{};
  char* p_value = 0;

  std::ifstream ifs(config_path, std::ios::in);
  while (ifs.getline(line.data(), line.size())) {
    if (CHECK_ROOT(Game)) t = root::game;
    if (CHECK_ROOT(System)) t = root::system;
    if (CHECK_ROOT(Keymap)) t = root::keymap;
    if (CHECK_ROOT(Font)) t = root::font;
    if (CHECK_ROOT(Kernel)) t = root::kernel;

    if (t == root::kernel) {
      // synchronization
      if (p_value = GET_ITEM(Synchronization), p_value) {
        asynchronized = CHECK_ITEM(p_value, "OFF");
      }
      // render driver
      if (p_value = GET_ITEM(RenderDriver), p_value) {
        if (CHECK_ITEM(p_value, "software")) {
          driver_type = drivers::software;
        }
        if (CHECK_ITEM(p_value, "opengl")) {
          driver_type = drivers::opengl;
        }
        if (CHECK_ITEM(p_value, "direct3d9")) {
          driver_type = drivers::direct3d9;
        }
        if (CHECK_ITEM(p_value, "direct3d11")) {
          driver_type = drivers::direct3d11;
        }
      }
      // resource prefix
      if (p_value = GET_ITEM(ResourcePrefix), p_value) {
        resource_prefix = p_value;
      }
    }
  }
}
}  // namespace rgm::config

#ifndef RGM_FULLVERSION
#define RGM_FULLVERSION "RGM_FULLVERSION"
#endif

#ifndef CC_VERSION
#define CC_VERSION "CC_VERSION"
#endif
