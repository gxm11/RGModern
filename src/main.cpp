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

#include "main.hpp"

/// @brief SDL_Main 函数，程序的实际入口。
int main(int argc, char* argv[]) {
#ifdef __WIN32
  SetConsoleOutputCP(65001);
#endif

  if (!rgm::config::load_args(argc, argv)) return 0;

  rgm::config::load_ini();

  cen::log_info("RGModern starts running...");
  try {
    if (rgm::config::synchronized) {
      // rgm::engine_sync_t engine;
      rgm::engine_fiber_t engine;
      engine.run();
    } else {
      rgm::engine_async_t engine;
      engine.run();
    }
  } catch (std::exception& e) {
    std::string msg = "RGModern Internal Error:\n";
    cen::message_box::show(rgm::config::game_title, msg + e.what(),
                           cen::message_box_type::error);
  }
  cen::log_info("RGModern ends with applause.");
  return 0;
}