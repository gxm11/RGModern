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
#ifdef RGM_SHADER_D3D11
#include "shader_d3d11.hpp"
#else
#ifdef RGM_SHADER_OPENGL
#include "shader_opengl.hpp"
#else
#include "shader_empty.hpp"
#endif
#endif

namespace rgm::rmxp {
template <typename T>
struct init_shader {
  static void before(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    shader_base::setup(renderer.get());
    T::setup();
  }

  static void after(auto&) {
    if constexpr (requires { T::p_buffer; }) {
      T::p_buffer->Release();
    }
  }
};
}  // namespace rgm::rmxp