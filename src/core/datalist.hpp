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
#include <tuple>

#include "type_traits.hpp"

namespace rgm::core {
/**
 * @brief datalist 类用于储存一系列不同类型的变量。
 * @tparam Args 可变参数类型，代表内部存储的变量，必须要有无参数的构造函数。
 * @note datalist 是使用继承递归定义的类型。每个类型拥有成员变量 data。
 * 泛型函数 get() 用于获取指定类型的变量 data，如果类型不匹配则调用父类方法。
 * datalist 本质上和结构体类似，区别在于：结构体是通过变量名来区分不同的变量，
 * 而 datalist 是通过变量的类型来区分，所以 datalist
 * 存储的变量，其类型各不相同。
 */

template <typename T>
struct datalist;

template <typename... Ts>
struct datalist<std::tuple<Ts...>> {
  std::tuple<Ts...> data;

  template <typename T>
  T& get() {
    static_assert((std::same_as<T, Ts> || ... || false));

    auto get_ptr = [](auto&... args) -> T* {
      T* ptr = nullptr;

      auto set_ptr = [&ptr]<typename U>(U& u) {
        if constexpr (std::same_as<std::decay_t<U>, T>) {
          ptr = &u;
        }
      };

      (set_ptr(args), ...);
      return ptr;
    };

    return *std::apply(get_ptr, data);
  }
};
}  // namespace rgm::core
