// zlib License

// Copyright (C) [2023] [Xiaomi Guo]

// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.

// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:

// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once
#include "base/base.hpp"

namespace rgm::rmxp {
// overlayer，用于多层的 drawable，其实只有 window 用上了
// 指针 T* 指向绑定的 drawable 对象
// index 表示该层的编号，在绘制的时候可能有用。
template <typename T>
struct overlayer {
  const T* p_drawable;
  const size_t m_index;

  bool skip() const { return true; }
};
}  // namespace rgm::rmxp