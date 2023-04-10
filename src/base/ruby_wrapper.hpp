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
struct async_callback {
  int id;
  std::string buf;

  void run(auto&) {
    VALUE object = rb_str_new(buf.data(), buf.size() - 1);
    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");

    rb_funcall(rb_mRGM_Base, rb_intern("async_callback"), 2, INT2FIX(id),
               object);
  }
};

template <typename T>
struct ruby_async {
  int callback_id;
  T t;

  void run(auto& worker) {
    std::string out;
    t.run(worker, out);
    worker >> async_callback{callback_id, out};
  }
};

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

  template <typename T, typename... Args>
  static VALUE send2(VALUE, VALUE id_, value<Args>::type... args) {
    using U = ruby_async<T>;
    T_worker::template send<U>(U{detail::get<int>(id_), T{detail::get<Args>(args)...}});
    return Qnil;
  }

  template <typename T, size_t arity>
  static void bind2(VALUE module, const char* name) {
    using U = decltype(core::traits::struct_to_tuple<arity>(std::declval<T>()));
    auto helper = []<typename... Args>(std::tuple<Args...>*) {
      return &send2<T, Args...>;
    };

    auto* fp = helper(static_cast<U*>(nullptr));
    rb_define_module_function(module, name, fp, arity + 1);
  }
};
}  // namespace rgm::base

#define RGMBIND(module, method, struct, arity)                         \
  rgm::base::ruby_wrapper<std::remove_reference_t<decltype(worker)>>:: \
      template bind<struct, arity>(module, method)

#define RGMBIND2(module, method, struct, arity)                        \
  rgm::base::ruby_wrapper<std::remove_reference_t<decltype(worker)>>:: \
      template bind2<struct, arity>(module, method)
