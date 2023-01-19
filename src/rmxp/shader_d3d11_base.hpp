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
#include "d3d11.h"

namespace rgm::rmxp {
struct shader_base {
  static SDL_Renderer* renderer;
  static ID3D11DeviceContext* context;
  static ID3D11Device* device;

  static void setup(SDL_Renderer* renderer) {
    device = SDL_RenderGetD3D11Device(renderer);
    shader_base::renderer = renderer;
    device->GetImmediateContext(&shader_base::context);
  }
};
SDL_Renderer* shader_base::renderer;
ID3D11DeviceContext* shader_base::context;
ID3D11Device* shader_base::device;

template <typename T>
struct shader_static : shader_base {
  static ID3D11PixelShader* current_shader;

  ID3D11PixelShader* previous_shader;

  explicit shader_static() {
    context->PSGetShader(&previous_shader, NULL, 0);
    context->PSSetShader(current_shader, NULL, 0);
  }

  ~shader_static() { context->PSSetShader(previous_shader, NULL, 0); }

  static void setup() {
    device->CreatePixelShader(T::code, T::code_size, NULL, &current_shader);
  }
};

template <typename T>
ID3D11PixelShader* shader_static<T>::current_shader;

template <typename T>
struct shader_dynamic : shader_base {
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

  static void setup() {
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

template <typename T>
ID3D11PixelShader* shader_dynamic<T>::current_shader;

template <typename T>
D3D11_BUFFER_DESC shader_dynamic<T>::cbDesc;

template <typename T>
D3D11_SUBRESOURCE_DATA shader_dynamic<T>::InitData;

template <typename T>
ID3D11Buffer* shader_dynamic<T>::p_buffer;

template <typename T>
ID3D11PixelShader* shader_dynamic<T>::previous_shader;
}  // namespace rgm::rmxp