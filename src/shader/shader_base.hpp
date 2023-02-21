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
#include "base/base.hpp"
#include "driver.hpp"
#include "rmxp/builtin.hpp"

namespace rgm::shader {
template <driver_types driver>
struct shader_base {
  static void setup(cen::renderer&) {}
};

template <driver_types driver, template <driver_types> class T_shader>
struct shader_static : shader_base<driver> {
  static void setup(cen::renderer&) {}
};

template <driver_types driver, template <driver_types> class T_shader>
struct shader_dynamic : shader_base<driver> {
  static void setup(cen::renderer&) {}
};

template <driver_types driver>
struct shader_gray : shader_static<driver, shader_gray> {
  explicit shader_gray() {}
};

template <driver_types driver>
struct shader_hue : shader_dynamic<driver, shader_hue> {
  explicit shader_hue(int) {}
};

template <driver_types driver>
struct shader_tone : shader_dynamic<driver, shader_tone> {
  explicit shader_tone(rmxp::tone) {}
};

template <driver_types driver>
struct shader_transition : shader_dynamic<driver, shader_transition> {
  explicit shader_transition(double, int) {}
};
}  // namespace rgm::shader