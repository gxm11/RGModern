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
#include "d3d11.h"
#include "direct3d11/gray.h"
#include "direct3d11/hue.h"
#include "direct3d11/tone.h"
#include "direct3d11/transition.h"

namespace rgm::shader {
/// @brief 基本的 shader 类对 direct3d11 渲染器的特化
template <>
struct shader_base<direct3d11> {
  /// @brief 渲染器的数据地址
  inline static SDL_Renderer* renderer = nullptr;

  /// @brief D3D11 Device Context 的数据地址
  inline static ID3D11DeviceContext* context = nullptr;

  /// @brief D3D11 Device 的数据地址
  inline static ID3D11Device* device = nullptr;

  /// @brief 初始化 D3D11 Device
  static void setup(cen::renderer& renderer) noexcept {
    device = SDL_RenderGetD3D11Device(renderer.get());
    shader_base::renderer = renderer.get();
    device->GetImmediateContext(&shader_base::context);
  }
};

/// @brief 静态 shader 类对 direct3d11 渲染器的特化
/// @tparam T_shader shader 的类型
/// 利用 RAII 机制切换 shader，渲染效果执行结束后再切回来
template <template <config::driver_type> class T_shader>
struct shader_static<direct3d11, T_shader> : shader_base<direct3d11> {
  /* T_shader 对不同的渲染器也有特化 */
  using T = T_shader<direct3d11>;

  /// @brief T_shader 对应的 shader 对象地址
  inline static ID3D11PixelShader* current_shader = nullptr;

  /// @brief 存储调用此 shader 之前的 shader 状态，用于后续恢复
  ID3D11PixelShader* previous_shader;

  /// @brief 构造函数
  explicit shader_static() {
    /* 存储旧的 shader */
    context->PSGetShader(&previous_shader, NULL, 0);

    /* 应用 T_shader */
    context->PSSetShader(current_shader, NULL, 0);
  }

  /// @brief 析构函数
  ~shader_static() {
    /* 还原旧的 shader */
    context->PSSetShader(previous_shader, NULL, 0);
  }

  /// @brief 初始化 T_shader
  /// @param 渲染器
  static void setup(cen::renderer&) noexcept {
    /* 使用预编译的 code 创建 Pixel Shader */
    device->CreatePixelShader(T::code, T::code_size, NULL, &current_shader);
  }
};

/// @brief 动态 shader 类对 direct3d9 渲染器的特化
/// @tparam T_shader shader 的类型
/// 利用 RAII 机制切换 shader，渲染效果执行结束后再切回来
template <template <config::driver_type> class T_shader>
struct shader_dynamic<direct3d11, T_shader> : shader_base<direct3d11> {
  /* T_shader 对不同的渲染器也有特化 */
  using T = T_shader<direct3d11>;

  inline static ID3D11PixelShader* current_shader = nullptr;
  /* constants buffer description */
  inline static D3D11_BUFFER_DESC cbDesc{};

  /* constants sub resource data */
  inline static D3D11_SUBRESOURCE_DATA InitData{};

  /* constants buffer pointer */
  inline static ID3D11Buffer* p_buffer = nullptr;

  /// @brief T_shader 对应的 shader 对象地址
  inline static ID3D11PixelShader* previous_shader = nullptr;

  /// @brief 构造函数
  explicit shader_dynamic() {
    /* 存储旧的 shader */
    context->PSGetShader(&previous_shader, NULL, NULL);
  }

  ~shader_dynamic() {
    /* 还原旧的 shader */
    context->PSSetShader(previous_shader, NULL, 0);
  }

  /// @brief 要主动调用 refresh() 函数以设置 Constant Buffer
  static void refresh() {
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    SDL_zero(mappedResource);

    context->Map(p_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    memcpy(mappedResource.pData, &T::data, sizeof(T::data));
    context->Unmap(p_buffer, 0);

    /* 应用 T_shader */
    context->PSSetShader(current_shader, NULL, 0);

    /* 设置 Constant Buffers */
    context->PSSetConstantBuffers(0, 1, &p_buffer);
  }

  /// @brief 初始化 T_shader
  /// @param 渲染器
  static void setup(cen::renderer&) noexcept {
    /* 使用预编译的 code 创建 Pixel Shader */
    device->CreatePixelShader(T::code, T::code_size, NULL, &current_shader);

    /* constants buffer 相关的创建和初始化操作 */
    SDL_zero(cbDesc);
    cbDesc.ByteWidth = sizeof(T::data);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbDesc.MiscFlags = 0;
    cbDesc.StructureByteStride = 0;

    SDL_zero(InitData);
    InitData.pSysMem = &(T::data);
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;

    device->CreateBuffer(&cbDesc, &InitData, &p_buffer);
  }
};

/// @brief 用于实现灰度的 shader 类对 direct3d11 渲染器的特化
template <>
struct shader_gray<direct3d11> : shader_static<direct3d11, shader_gray> {
  static constexpr const unsigned char* code = rgm_shader_gray_data;
  static constexpr size_t code_size = sizeof(rgm_shader_gray_data);
};

/// @brief 用于实现色相的 shader 类对 direct3d11 渲染器的特化
template <>
struct shader_hue<direct3d11> : shader_dynamic<direct3d11, shader_hue> {
  // k3 没有任何作用，但是不设置就会导致 shader 失效。
  // 可能原因：在C++创建常量缓冲区时大小必须为16字节的倍数。
  // 参见：https://blog.csdn.net/X_Jun96/article/details/87722194

  struct buffer_t {
    float k0;
    float k1;
    float k2;
    float k3;
  };

  inline static buffer_t data{};

  static constexpr const unsigned char* code = rgm_shader_hue_data;
  static constexpr size_t code_size = sizeof(rgm_shader_hue_data);

  explicit shader_hue(int hue) {
    constexpr double pi = 3.141592653589793;
    constexpr double r3 = 1.7320508075688772;
    double angle = (pi / 180.0f) * hue;

    data.k1 = (1.0 - cos(angle) - r3 * sin(angle)) / 3.0;
    data.k2 = (1.0 - cos(angle) + r3 * sin(angle)) / 3.0;
    data.k0 = 1.0 - data.k1 - data.k2;

    refresh();
  }
};

/// @brief 用于实现色调的 shader 类对 direct3d11 渲染器的特化
template <>
struct shader_tone<direct3d11> : shader_dynamic<direct3d11, shader_tone> {
  struct buffer_t {
    float red;
    float green;
    float blue;
    float gray;
  };

  inline static buffer_t data{};

  static constexpr const unsigned char* code = rgm_shader_tone_data;
  static constexpr size_t code_size = sizeof(rgm_shader_tone_data);

  explicit shader_tone(rmxp::tone t) {
    data.red = t.red / 255.0f;
    data.green = t.green / 255.0f;
    data.blue = t.blue / 255.0f;
    data.gray = t.gray / 255.0f;

    refresh();
  }
};

/// @brief 用于实现渐变的 shader 类对 direct3d11 渲染器的特化
template <>
struct shader_transition<direct3d11>
    : shader_dynamic<direct3d11, shader_transition> {
  struct buffer_t {
    float k0;
    float k1;
    float k2;
    float k3;
  };

  inline static buffer_t data{};

  static constexpr const unsigned char* code = rgm_shader_transition_data;
  static constexpr size_t code_size = sizeof(rgm_shader_transition_data);

  explicit shader_transition(double rate, int vague) {
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
}  // namespace rgm::shader