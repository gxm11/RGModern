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
#include "shader_opengl_base.hpp"

INCTXT(shader_gray_fs, "./src/shader/opengl/gray.fs");
INCTXT(shader_hue_fs, "./src/shader/opengl/hue.fs");

namespace rgm::rmxp {
struct shader_gray : shader_static<shader_gray> {
  static constexpr const char* fragment = rgm_shader_gray_fs_data;
};

struct shader_hue : shader_static<shader_hue> {
  static constexpr const char* fragment = rgm_shader_hue_fs_data;

  shader_hue(int hue) {
    constexpr double pi = 3.141592653589793;
    constexpr double r3 = 1.7320508075688772;
    double angle = (pi / 180.0f) * hue;

    float k1 = (1.0 - cos(angle) - r3 * sin(angle)) / 3.0;
    float k2 = (1.0 - cos(angle) + r3 * sin(angle)) / 3.0;
    float k0 = 1.0 - k1 - k2;

    auto location = glGetUniformLocation(program_id, "k");
    if (location > 0) {
      glUniform4f(location, k0, k1, k2, 0);
    }
  }
};

struct shader_tone {
  shader_tone(tone) {}
};

struct shader_transition {
  shader_transition(double, int) {}
};
}  // namespace rgm::rmxp