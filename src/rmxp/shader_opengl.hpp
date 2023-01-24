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
INCTXT(shader_tone_fs, "./src/shader/opengl/tone.fs");
INCTXT(shader_transition_fs, "./src/shader/opengl/transition.fs");

namespace rgm::rmxp {
struct shader_gray : shader_static<shader_gray> {
  static constexpr const char* fragment = rgm_shader_gray_fs_data;
};

struct shader_hue : shader_static<shader_hue> {
  static constexpr const char* fragment = rgm_shader_hue_fs_data;

  explicit shader_hue(int hue) {
    constexpr double pi = 3.141592653589793;
    constexpr double r3 = 1.7320508075688772;
    double angle = (pi / 180.0f) * hue;

    float k1 = (1.0 - cos(angle) - r3 * sin(angle)) / 3.0;
    float k2 = (1.0 - cos(angle) + r3 * sin(angle)) / 3.0;
    float k0 = 1.0 - k1 - k2;

    static const auto location = glGetUniformLocation(program_id, "k");
    if (location > 0) {
      glUniform4f(location, k0, k1, k2, 0);
    }
  }
};

struct shader_tone : shader_static<shader_tone> {
  static constexpr const char* fragment = rgm_shader_tone_fs_data;

  explicit shader_tone(tone t) {
    float red = t.red / 255.0f;
    float green = t.green / 255.0f;
    float blue = t.blue / 255.0f;
    float gray = t.gray / 255.0f;

    static const auto location = glGetUniformLocation(program_id, "tone");
    if (location > 0) {
      glUniform4f(location, red, green, blue, gray);
    }
  }
};

struct shader_transition : shader_static<shader_transition> {
  static constexpr const char* fragment = rgm_shader_transition_fs_data;

  explicit shader_transition(double rate, int vague) {
    float k0, k1, k2, k3;

    if (vague == 0) {
      k0 = 0;
      k1 = rate;
      k2 = 0;
      k3 = 0;
    } else {
      k0 = 1;
      k1 = 0;
      k2 = rate - vague / 255.0;
      k3 = 255.0 / vague;
    }

    static const auto location = glGetUniformLocation(program_id, "k");
    if (location > 0) {
      glUniform4f(location, k0, k1, k2, k3);
    }
  }
};
}  // namespace rgm::rmxp