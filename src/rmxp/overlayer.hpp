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

namespace rgm::rmxp {
/// @brief overlayer，用于表示和绘制多层的 drawable
/// overlayer 是否加入 drawables 统一管理，或者如何绘制取决于具体实现。
/// 目前只有 window 和 tilemap 是多层的。window 有一层 z+2 的固定 overlayer，
/// tilemap 则是多层。实际处理中，window 的 overlayer 保存在 drawables 中，
/// 而 tilemap 则是依靠 tilemap_manager 来管理。
template <typename T>
struct overlayer {
  /// @brief 绑定的 drawable 数据
  const T* p_drawable;

  /// @brief 该 ovelayer 层的编号
  const size_t m_index;

  /// @brief 实现 drawable 的接口函数 skip
  bool skip() const { return true; }
};
}  // namespace rgm::rmxp