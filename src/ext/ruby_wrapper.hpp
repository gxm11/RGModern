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

namespace rgm::ext {
/// @brief 回调 ruby 中的 RGM::Ext.async_callback 函数
/// @name task
struct ruby_callback {
  /// @brief 回调的 Proc ID
  int id;

  /// @brief 回调的字符串值
  std::string buf;

  void run(auto&) {
    VALUE object = rb_str_new(buf.data(), buf.size());
    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Ext = rb_define_module_under(rb_mRGM, "Ext");

    rb_funcall(rb_mRGM_Ext, rb_intern("async_callback"), 2, INT2FIX(id),
               object);
  }
};

/// @brief 封装异步任务的模板类
/// @tparam T 带回调的异步任务类
/// @name meta
/// 此类需配合宏 RGMBIND2 使用。
/// 此类接受一个定义了 run(worker&, std::string&) 函数的任务类，将其封装为
/// 一个正常的任务类。执行 T::run 后，会发送回调到解释执行 ruby 的 worker 。
template <typename T>
struct ruby_async {
  /// @brief 回调的 Proc ID
  int id;

  /// @brief 以组合的形式封装作为参数的类
  T t;

  void run(auto& worker) {
    std::string out = "";
    t.run(worker, out);

    /* 发送回调到解释执行 ruby 的 worker */
    worker >> ruby_callback{id, std::move(out)};
  }
};

/// @brief ruby_wrapper 模板类用于快速绑定 ruby 中的函数和 C++ 的任务类
/// @tparam T_worker 显然只能是解释执行 ruby 的 worker
/// @name meta
/// @see ./src/base/ruby_wrapper.hpp
/// 区别在于 RGMBIND2 定义的函数多了一个尾部的参数为 Proc ID
template <typename T_worker>
struct ruby_wrapper {
  /* 需要使用 base::detail 完成 ruby 到 C++ 类型的转换 */
  using detail = base::detail;

  /// @brief value 模板类，将任何输入类型转换成 VALUE
  template <typename>
  struct value {
    using type = VALUE;
  };

  template <typename T, typename... Args>
  static VALUE send(VALUE, value<Args>::type... args, VALUE id_) {
    using U = ruby_async<T>;
    T_worker::template send<U>(
        U{detail::get<int>(id_), T{detail::get<Args>(args)...}});
    return Qnil;
  }

  template <typename T, size_t arity>
  static void bind(VALUE module, const char* name) {
    using U = decltype(core::traits::struct_to_tuple<arity>(std::declval<T>()));
    auto helper = []<typename... Args>(std::tuple<Args...>*) {
      return &send<T, Args...>;
    };

    auto* fp = helper(static_cast<U*>(nullptr));
    rb_define_module_function(module, name, fp, arity + 1);
  }
};
}  // namespace rgm::ext

/// @brief 将 Ruby 函数绑定到 C++ 中任务类的宏
/// 定义了一个异步任务，使用的格式与 rb_define_module_function 相同。
/// 此任务类必须用 ext::ruby_async 封装，否则会没有任何效果。
#define RGMBIND2(module, method, struct, arity)                       \
  rgm::ext::ruby_wrapper<std::remove_reference_t<decltype(worker)>>:: \
      template bind<struct, arity>(module, method)
