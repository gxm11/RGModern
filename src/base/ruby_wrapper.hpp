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
  template <typename>
  struct value {
    using type = VALUE;
  };

  explicit ruby_wrapper(T_worker&) {}

  template <typename T, typename... Args>
  static VALUE send(VALUE, value<Args>::type... args) {
    T_worker::template send<T>(T{detail::get<Args>(args)...});
    return Qnil;
  }

  template <typename T, size_t arity>
  static void bind(VALUE module, const char* name) {
    using U = decltype(core::traits::struct_to_tuple<arity>(std::declval<T>()));
    auto helper = []<typename... Args>(std::tuple<Args...>*) {
      return &send<T, Args...>;
    };

    auto* fp = helper(static_cast<U*>(nullptr));
    rb_define_module_function(module, name, fp, arity);
  }
};
}  // namespace rgm::base

#define RGMBIND(module, method, struct, arity) \
  rgm::base::ruby_wrapper(worker).template bind<struct, arity>(module, method)
