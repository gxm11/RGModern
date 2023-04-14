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
#include "base/base.hpp"
#include "rmxp/builtin.hpp"

namespace rgm::shader {
using enum config::driver_type;

template <config::driver_type driver>
struct shader_base {
  static void setup(cen::renderer&) {}
};

template <config::driver_type driver,
          template <config::driver_type> class T_shader>
struct shader_static : shader_base<driver> {
  static void setup(cen::renderer&) {}
};

template <config::driver_type driver,
          template <config::driver_type> class T_shader>
struct shader_dynamic : shader_base<driver> {
  static void setup(cen::renderer&) {}
};

template <config::driver_type driver>
struct shader_gray : shader_static<driver, shader_gray> {
  explicit shader_gray() {}
};

template <config::driver_type driver>
struct shader_hue : shader_dynamic<driver, shader_hue> {
  explicit shader_hue(int) {}
};

template <config::driver_type driver>
struct shader_tone : shader_dynamic<driver, shader_tone> {
  explicit shader_tone(rmxp::tone) {}
};

template <config::driver_type driver>
struct shader_transition : shader_dynamic<driver, shader_transition> {
  explicit shader_transition(double, int) {}
};
}  // namespace rgm::shader