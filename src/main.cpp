// Copyright (c) 2022 Xiaomi Guo
// Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan
// PSL v2. You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
// Mulan PSL v2 for more details.

#include "main.hpp"

/**
 * @brief SDL_Main 函数，程序的实际入口。
 *
 * @param argc
 * @param argv
 * @return int = 0，表示程序正常退出
 */
int main(int argc, char* argv[]) {
#ifdef __WIN32
  SetConsoleOutputCP(65001);
#endif
  for (int i = 0; i < argc; ++i) {
    // show version and exit
    if (strncmp(argv[i], "-v", 2) == 0) {
      printf("RGM %s [BuildMode = %d]\n\n", RGM_FULLVERSION, RGM_BUILDMODE);
      printf("Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.\n");
      printf("Copyright (c) 2022 Xiaomi Guo\n\n");
      printf("Compiler: %s\n", CC_VERSION);
      printf("Vendors:\n - ");
      ruby_show_version();
#define GETVERSION(x) \
  SDL_##x##_MAJOR_VERSION, SDL_##x##_MINOR_VERSION, SDL_##x##_PATCHLEVEL
      printf(" - SDL %d.%d.%d\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION,
             SDL_PATCHLEVEL);
      printf(" - SDL Image %d.%d.%d\n", GETVERSION(IMAGE));
      printf(" - SDL TTF %d.%d.%d\n", GETVERSION(TTF));
      printf(" - SDL Mixer %d.%d.%d\n", GETVERSION(MIXER));
#undef GETVERSION
      printf("Asynchronized Mode: %s\n",
             rgm::config::asynchornized ? "ON" : "OFF");
      return 0;
    }
    // load configs from argv
    if (strncmp(argv[i], "btest", 6) == 0) {
      rgm::config::btest = true;
    }
    if (strncmp(argv[i], "debug", 6) == 0) {
      rgm::config::debug = true;
    }
  }
  engine_t engine;
  cen::log_info(cen::log_category::system, "rgm start running...");
  engine.run();
  cen::log_info(cen::log_category::system, "rgm terminated.");
  return 0;
}