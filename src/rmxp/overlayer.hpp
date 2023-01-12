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