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
#include "shader_base.hpp"
/* include shader_base before other c headers */
#include "d3dx9.h"
#include "direct3d9/gray.h"
#include "direct3d9/hue.h"
#include "direct3d9/tone.h"
#include "direct3d9/transition.h"

namespace rgm::shader {
/// @brief 基本的 shader 类对 direct3d9 渲染器的特化
template <>
struct shader_base<direct3d9> {
  /// @brief 渲染器的数据地址
  inline static SDL_Renderer* renderer = nullptr;

  /// @brief D3D9 Device 的数据地址
  inline static IDirect3DDevice9* device = nullptr;

  /// @brief 初始化 D3D9 Device
  static void setup(cen::renderer& renderer) noexcept {
    shader_base::renderer = renderer.get();
    shader_base::device = SDL_RenderGetD3D9Device(renderer.get());
  }
};

/// @brief 静态 shader 类对 direct3d9 渲染器的特化
/// @tparam T_shader shader 的类型
/// 利用 RAII 机制切换 shader，渲染效果执行结束后再切回来
template <template <config::driver_type> class T_shader>
struct shader_static<direct3d9, T_shader> : shader_base<direct3d9> {
  /* T_shader 对不同的渲染器也有特化 */
  using T = T_shader<direct3d9>;

  /// @brief T_shader 对应的 shader 对象地址
  inline static IDirect3DPixelShader9* current_shader = nullptr;

  /// @brief 存储调用此 shader 之前的 shader 状态，用于后续恢复
  IDirect3DPixelShader9* previous_shader;

  /// @brief 构造函数
  explicit shader_static() {
    /* 存储旧的 shader */
    IDirect3DDevice9_GetPixelShader(device, &previous_shader);

    /* 应用 T_shader */
    IDirect3DDevice9_SetPixelShader(device, current_shader);
  }

  /// @brief 析构函数
  ~shader_static() {
    /* Flush */
    SDL_RenderFlush(renderer);

    /* 还原旧的 shader */
    IDirect3DDevice9_SetPixelShader(device, previous_shader);
  }

  /// @brief 初始化 T_shader
  /// @param 渲染器
  static void setup(cen::renderer&) noexcept {
    /* 使用预编译的 code 创建 Pixel Shader */
    IDirect3DDevice9_CreatePixelShader(shader_base<direct3d9>::device,
                                       reinterpret_cast<const DWORD*>(T::code),
                                       &current_shader);
  }
};

/// @brief 动态 shader 类对 direct3d9 渲染器的特化
/// @tparam T_shader shader 的类型
/// 利用 RAII 机制切换 shader，渲染效果执行结束后再切回来
template <template <config::driver_type> class T_shader>
struct shader_dynamic<direct3d9, T_shader> : shader_base<direct3d9> {
  /* T_shader 对不同的渲染器也有特化 */
  using T = T_shader<direct3d9>;

  /// @brief T_shader 对应的 shader 对象地址
  inline static IDirect3DPixelShader9* current_shader = nullptr;

  /// @brief T_shader 在运行时要读取的 constant table 地址
  inline static ID3DXConstantTable* constant_table = nullptr;

  /// @brief 存储调用此 shader 之前的 shader 状态，用于后续恢复
  IDirect3DPixelShader9* previous_shader;

  /// @brief 构造函数
  explicit shader_dynamic() {
    /* 存储旧的 shader */
    IDirect3DDevice9_GetPixelShader(device, &previous_shader);

    /* 应用 T_shader */
    IDirect3DDevice9_SetPixelShader(device, current_shader);
  }

  /// @brief 析构函数
  ~shader_dynamic() {
    /* Flush */
    SDL_RenderFlush(renderer);

    /* 还原旧的 shader */
    IDirect3DDevice9_SetPixelShader(device, previous_shader);
  }

  /// @brief 初始化 T_shader
  /// @param 渲染器
  static void setup(cen::renderer&) noexcept {
    /* 使用预编译的 code 创建 Pixel Shader */
    IDirect3DDevice9_CreatePixelShader(shader_base::device,
                                       reinterpret_cast<const DWORD*>(T::code),
                                       &current_shader);
    /* 初始化 ConstantTable */
    D3DXGetShaderConstantTable(reinterpret_cast<const DWORD*>(T::code),
                               &constant_table);
  }
};

/// @brief 用于实现灰度的 shader 类对 direct3d9 渲染器的特化
template <>
struct shader_gray<direct3d9> : shader_static<direct3d9, shader_gray> {
  static constexpr const unsigned char* code = rgm_shader_gray_dx9_data;
  static constexpr size_t code_size = sizeof(rgm_shader_gray_dx9_data);
};

/// @brief 用于实现色相的 shader 类对 direct3d9 渲染器的特化
template <>
struct shader_hue<direct3d9> : shader_dynamic<direct3d9, shader_hue> {
  static constexpr const unsigned char* code = rgm_shader_hue_dx9_data;
  static constexpr size_t code_size = sizeof(rgm_shader_hue_dx9_data);

  explicit shader_hue(int hue) {
    constexpr double pi = 3.141592653589793;
    constexpr double r3 = 1.7320508075688772;
    double angle = (pi / 180.0f) * hue;

    float k[4];
    k[1] = (1.0 - cos(angle) - r3 * sin(angle)) / 3.0;
    k[2] = (1.0 - cos(angle) + r3 * sin(angle)) / 3.0;
    k[0] = 1.0 - k[1] - k[2];
    k[3] = 0;

    /* 设置 Constant Table */
    auto handle = constant_table->GetConstantByName(0, "k");
    constant_table->SetFloatArray(device, handle, k, 4);
  }
};

/// @brief 用于实现色调的 shader 类对 direct3d9 渲染器的特化
template <>
struct shader_tone<direct3d9> : shader_dynamic<direct3d9, shader_tone> {
  static constexpr const unsigned char* code = rgm_shader_tone_dx9_data;
  static constexpr size_t code_size = sizeof(rgm_shader_tone_dx9_data);

  explicit shader_tone(rmxp::tone t) {
    float a[4] = {t.red / 255.0f, t.green / 255.0f, t.blue / 255.0f,
                  t.gray / 255.0f};

    /* 设置 Constant Table */
    auto handle = constant_table->GetConstantByName(0, "tone");
    constant_table->SetFloatArray(device, handle, a, 4);
  }
};

/// @brief 用于实现渐变的 shader 类对 direct3d9 渲染器的特化
template <>
struct shader_transition<direct3d9>
    : shader_dynamic<direct3d9, shader_transition> {
  static constexpr const unsigned char* code = rgm_shader_transition_dx9_data;
  static constexpr size_t code_size = sizeof(rgm_shader_transition_dx9_data);

  explicit shader_transition(double rate, int vague) {
    float k[4] = {0, 0, 0, 0};

    if (vague == 0) {
      k[0] = 0;
      k[1] = rate;
    } else {
      k[0] = 1;
      k[2] = rate - vague / 255.0;
      k[3] = 255.0 / vague;
    }

    /* 设置 Constant Table */
    auto handle = constant_table->GetConstantByName(0, "k");
    constant_table->SetFloatArray(device, handle, k, 4);
  }
};
}  // namespace rgm::shader