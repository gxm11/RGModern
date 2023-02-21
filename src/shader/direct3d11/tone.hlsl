// Copyright (c) 2022 Xiaomi Guo
// Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.

Texture2D shaderTexture;
SamplerState samplerState;

struct Input
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD;
};

float4 tone;

float4 main(Input input) : SV_TARGET
{
    float4 color = shaderTexture.Sample(samplerState, input.tex);
    float gray = color.r * 0.299 + color.g * 0.587 + color.b * 0.114;
    color.rgb *= 1 - tone.a;
    color.rgb += gray * tone.a;
    color.rgb += tone.rgb;
    // Return the final color
    return color;
}
