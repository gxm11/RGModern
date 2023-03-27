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
  static VALUE sender(VALUE, value<Args>::type... args) {
    *p_worker >> T{detail::get<Args>(args)...};
    return Qnil;
  }

  template <typename T, typename... Args>
  static void create_sender(VALUE module, const char* name) {
    auto* fp = &sender<T, Args...>;
    rb_define_module_function(module, name, fp, sizeof...(Args));
  }

  template <typename T, size_t arity>
  static void bind(VALUE module, const char* name) {
    using U = decltype(core::traits::struct_to_tuple<arity>(T{}));
    auto* fp = bind_helper((T*)0, (U*)0);
    rb_define_module_function(module, name, fp, arity);
  }

  template <typename T, typename... Args>
  static auto* bind_helper(T*, std::tuple<Args...>*) {
    return &sender<T, Args...>;
  }
};
}  // namespace rgm::base

// clang-format off
// clang-format 会在 __VA_OPT__ 中的逗号后添加一个空格
#define RGMBIND(module, function, ...)              \
  rgm::base::ruby_wrapper(this_worker)              \
      .template create_sender<function              \
      __VA_OPT__(,) __VA_ARGS__>(module, #function)
#define RGMBIND2(module, method, function, ...)     \
  rgm::base::ruby_wrapper(this_worker)              \
      .template create_sender<function              \
      __VA_OPT__(,) __VA_ARGS__>(module, method)

#define RGMBIND3(module, function, arity) \
  rgm::base::ruby_wrapper(worker) \
      .template bind<function, arity>(module, #function)
// clang-format on

#define RGMBIND4(module, method, function, arity) \
  rgm::base::ruby_wrapper(worker) \
    .template bind<function, arity>(module, method)
