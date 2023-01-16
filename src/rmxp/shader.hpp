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
#include "shader/gray.h"
#include "shader/hue.h"
#include "shader/tone.h"
#include "shader/transition.h"
#include "shader_base.hpp"

namespace rgm::rmxp {
struct shader_gray : shader_static<shader_gray> {
  static constexpr const unsigned char* code = rgm_shader_gray_data;
  static constexpr size_t code_size = sizeof(rgm_shader_gray_data);
};

struct shader_tone : shader_dynamic<shader_tone> {
  struct buffer_t {
    float red;
    float green;
    float blue;
    float gray;
  };

  static buffer_t data;

  static constexpr const unsigned char* code = rgm_shader_tone_data;
  static constexpr size_t code_size = sizeof(rgm_shader_tone_data);

  explicit shader_tone(tone t) : shader_dynamic<shader_tone>() {
    data.red = t.red / 255.0f;
    data.green = t.green / 255.0f;
    data.blue = t.blue / 255.0f;
    data.gray = t.gray / 255.0f;

    refresh();
  }
};
shader_tone::buffer_t shader_tone::data;

struct shader_hue : shader_dynamic<shader_hue> {
  // k3 没有任何作用，但是不设置就会导致 shader 失效。
  // 可能原因：在C++创建常量缓冲区时大小必须为16字节的倍数。
  // 参见：https://blog.csdn.net/X_Jun96/article/details/87722194

  struct buffer_t {
    float k0;
    float k1;
    float k2;
    float k3;
  };

  static buffer_t data;

  static constexpr const unsigned char* code = rgm_shader_hue_data;
  static constexpr size_t code_size = sizeof(rgm_shader_hue_data);

  explicit shader_hue(int hue) : shader_dynamic<shader_hue>() {
    constexpr double pi = 3.141592653589793;
    constexpr double r3 = 1.7320508075688772;
    double angle = (pi / 180.0f) * hue;

    data.k1 = (1.0 - cos(angle) - r3 * sin(angle)) / 3.0;
    data.k2 = (1.0 - cos(angle) + r3 * sin(angle)) / 3.0;
    data.k0 = 1.0 - data.k1 - data.k2;

    refresh();
  }
};
shader_hue::buffer_t shader_hue::data;

struct shader_transition : shader_dynamic<shader_transition> {
  struct buffer_t {
    float k0;
    float k1;
    float k2;
    float k3;
  };

  static buffer_t data;

  static constexpr const unsigned char* code = rgm_shader_transition_data;
  static constexpr size_t code_size = sizeof(rgm_shader_transition_data);

  explicit shader_transition(double rate, int vague)
      : shader_dynamic<shader_transition>() {
    if (vague == 0) {
      data.k0 = 0;
      data.k1 = rate;
    } else {
      data.k0 = 1;
      data.k2 = rate - vague / 255.0;
      data.k3 = 255.0 / vague;
    }

    refresh();
  }
};
shader_transition::buffer_t shader_transition::data;
}  // namespace rgm::rmxp