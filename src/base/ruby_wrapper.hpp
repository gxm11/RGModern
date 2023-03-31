// zlib License

// copyright (C) 2023 Guoxiaomi and Krimiston

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

#define RGMBIND(module, method, struct, arity)                         \
  rgm::base::ruby_wrapper<std::remove_reference_t<decltype(worker)>>:: \
      template bind<struct, arity>(module, method)
