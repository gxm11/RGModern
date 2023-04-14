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

Texture2D shaderTexture;
SamplerState samplerState;

float4 main(float2 tex : TEXCOORD) : SV_TARGET {
  float4 color = shaderTexture.Sample(samplerState, tex);
  float gray = color.r * 0.299 + color.g * 0.587 + color.b * 0.114;
  color.rgb = gray;
  // Return the final color
  return color;
}
