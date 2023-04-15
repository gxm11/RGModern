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
#include "core/core.hpp"
#include "detail.hpp"

namespace rgm::base {
/// @brief ruby_wrapper 模板类用于快速绑定 ruby 中的函数和 C++ 的任务类
/// @tparam T_worker 显然只能是解释执行 ruby 的 worker
/// @name meta
template <typename T_worker>
struct ruby_wrapper {
  /// @brief value 模板类，将任何输入类型转换成 VALUE
  template <typename>
  struct value {
    using type = VALUE;
  };

  /// @brief 绑定的 Ruby 函数实际执行的内容
  /// @param 此函数的参数都是 VALUE 类型
  /// @return 返回值是 Qnil
  /// 1. 构造了 T 类型的变量
  /// 2. 尝试将 args... 由 VALUE 转换成 T 中成员变量的类型
  /// 3. 将构造的变量发送到其他 worker 执行
  template <typename T, typename... Args>
  static VALUE send(VALUE, value<Args>::type... args) {
    T_worker::template send<T>(T{detail::get<Args>(args)...});
    return Qnil;
  }

  /// @brief 将 Ruby 函数绑定到 C++ 中任务类的函数，并且自动处理数据类型转换
  /// @tparam T C++ 中的任务类
  /// @tparam arity T 的 field 数量，或者说 Ruby 函数的参数数量，两者必须等同
  /// @param module Ruby 中模块对应的 VALUE
  /// @param name Ruby 中的方法名
  /// 用这个方法定义的 Ruby 中的模块方法，将把 Ruby 的函数参数转换成 T
  /// 的成员变量 对应的类型，并把 T{} 广播给所有的 worker。
  template <typename T, size_t arity>
  static void bind(VALUE module, const char* name) {
    using U = decltype(core::traits::struct_to_tuple<arity>(std::declval<T>()));
    auto helper = []<typename... Args>(std::tuple<Args...>*) {
      /* 这里是静态模板函数的全特化，所以存在内存地址 */
      return &send<T, Args...>;
    };

    auto* fp = helper(static_cast<U*>(nullptr));
    rb_define_module_function(module, name, fp, arity);
  }
};
}  // namespace rgm::base

/// @brief 将 Ruby 函数绑定到 C++ 中任务类的宏
/// 定义了一个异步任务，使用的格式与 rb_define_module_function 相同。
/// 此任务类也必须有某个 worker 承接，否则调用会没有任何效果。
#define RGMBIND(module, method, struct, arity)                         \
  rgm::base::ruby_wrapper<std::remove_reference_t<decltype(worker)>>:: \
      template bind<struct, arity>(module, method)
