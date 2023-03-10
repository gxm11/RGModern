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
#include "d3d11.h"
#include "direct3d11/gray.h"
#include "direct3d11/hue.h"
#include "direct3d11/tone.h"
#include "direct3d11/transition.h"
#include "shader_base.hpp"

namespace rgm::shader {
template <>
struct shader_base<direct3d11> {
  static SDL_Renderer* renderer;
  static ID3D11DeviceContext* context;
  static ID3D11Device* device;

  static void setup(cen::renderer& renderer) {
    device = SDL_RenderGetD3D11Device(renderer.get());
    shader_base::renderer = renderer.get();
    device->GetImmediateContext(&shader_base::context);
  }
};
SDL_Renderer* shader_base<direct3d11>::renderer;
ID3D11DeviceContext* shader_base<direct3d11>::context;
ID3D11Device* shader_base<direct3d11>::device;

template <template <size_t> class T_shader>
struct shader_static<direct3d11, T_shader> : shader_base<direct3d11> {
  using T = T_shader<direct3d11>;

  static ID3D11PixelShader* current_shader;

  ID3D11PixelShader* previous_shader;

  explicit shader_static() {
    context->PSGetShader(&previous_shader, NULL, 0);
    context->PSSetShader(current_shader, NULL, 0);
  }

  ~shader_static() { context->PSSetShader(previous_shader, NULL, 0); }

  static void setup(cen::renderer&) {
    device->CreatePixelShader(T::code, T::code_size, NULL, &current_shader);
  }
};

template <template <size_t> class T_shader>
ID3D11PixelShader* shader_static<direct3d11, T_shader>::current_shader;

template <template <size_t> class T_shader>
struct shader_dynamic<direct3d11, T_shader> : shader_base<direct3d11> {
  using T = T_shader<direct3d11>;

  static ID3D11PixelShader* current_shader;
  // constants buffer description
  static D3D11_BUFFER_DESC cbDesc;
  static D3D11_SUBRESOURCE_DATA InitData;

  static ID3D11Buffer* p_buffer;
  static ID3D11PixelShader* previous_shader;

  explicit shader_dynamic() {
    context->PSGetShader(&previous_shader, NULL, NULL);
  }

  ~shader_dynamic() { context->PSSetShader(previous_shader, NULL, 0); }

  static void refresh() {
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    SDL_zero(mappedResource);

    context->Map(p_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    memcpy(mappedResource.pData, &T::data, sizeof(T::data));
    context->Unmap(p_buffer, 0);

    context->PSSetShader(current_shader, NULL, 0);
    context->PSSetConstantBuffers(0, 1, &p_buffer);
  }

  static void setup(cen::renderer&) {
    device->CreatePixelShader(T::code, T::code_size, NULL, &current_shader);

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

template <template <size_t> class T_shader>
ID3D11PixelShader* shader_dynamic<direct3d11, T_shader>::current_shader;

template <template <size_t> class T_shader>
D3D11_BUFFER_DESC shader_dynamic<direct3d11, T_shader>::cbDesc;

template <template <size_t> class T_shader>
D3D11_SUBRESOURCE_DATA shader_dynamic<direct3d11, T_shader>::InitData;

template <template <size_t> class T_shader>
ID3D11Buffer* shader_dynamic<direct3d11, T_shader>::p_buffer;

template <template <size_t> class T_shader>
ID3D11PixelShader* shader_dynamic<direct3d11, T_shader>::previous_shader;

template <>
struct shader_gray<direct3d11> : shader_static<direct3d11, shader_gray> {
  static constexpr const unsigned char* code = rgm_shader_gray_data;
  static constexpr size_t code_size = sizeof(rgm_shader_gray_data);
};

template <>
struct shader_hue<direct3d11> : shader_dynamic<direct3d11, shader_hue> {
  // k3 ???????????????????????????????????????????????? shader ?????????
  // ??????????????????C++???????????????????????????????????????16??????????????????
  // ?????????https://blog.csdn.net/X_Jun96/article/details/87722194

  struct buffer_t {
    float k0;
    float k1;
    float k2;
    float k3;
  };

  static inline buffer_t data;

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

template <>
struct shader_tone<direct3d11> : shader_dynamic<direct3d11, shader_tone> {
  struct buffer_t {
    float red;
    float green;
    float blue;
    float gray;
  };

  static inline buffer_t data{};

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

template <>
struct shader_transition<direct3d11>
    : shader_dynamic<direct3d11, shader_transition> {
  struct buffer_t {
    float k0;
    float k1;
    float k2;
    float k3;
  };

  static inline buffer_t data;

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