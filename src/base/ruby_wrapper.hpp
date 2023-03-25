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
#include "core/core.hpp"
#include "detail.hpp"

namespace rgm::base {
template <typename T_worker>
struct ruby_wrapper {
  // type to convert all input args into VALUE
  template <typename T>
  struct value {
    using type = VALUE;
  };

  inline static T_worker* p_worker = nullptr;

  explicit ruby_wrapper(T_worker& w) {
    if (!p_worker) p_worker = &w;
  }

  template <typename T, typename... Args>
  static VALUE function(VALUE, value<Args>::type... args) {
    *p_worker >> T{detail::get<Args>(args)...};
    return Qnil;
  }
};
}  // namespace rgm::base