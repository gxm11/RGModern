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
#include "builtin.hpp"

namespace rgm::rmxp {
struct shader_base {
  static void setup(SDL_Renderer*) {}
};
struct shader_gray {
  static void setup() {}
};
struct shader_hue {
  shader_hue(int) {}
  static void setup() {}
};
struct shader_tone {
  shader_tone(tone) {}
  static void setup() {}
};
struct shader_transition {
  shader_transition(double, int) {}
  static void setup() {}
};
}