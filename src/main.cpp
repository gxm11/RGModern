// zlib License

// Copyright (C) [2023] [Xiaomi Guo]

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

#include "main.hpp"

#include <any>

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
      return 0;
    }
  }

  rgm::config::load(argc, argv);

  cen::log_info(cen::log_category::system, "rgm start running...");
  if (rgm::config::asynchronized) {
    rgm::engine_async_t engine;
    engine.run();
  } else {
    rgm::engine_sync_t engine;
    engine.run();
  }
  cen::log_info(cen::log_category::system, "rgm terminated.");
  return 0;
}