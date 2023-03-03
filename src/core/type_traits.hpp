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
#include <concepts>
#include <tuple>
#include <type_traits>
#include <variant>

#include "cooperation.hpp"
/**
 * @brief 使用现代元编程的方式完成类型萃取
 */
namespace rgm::core::traits {
template <typename... Ts>
consteval auto expand_tuples(Ts... tuples) {
  static_assert((requires { std::tuple_size_v<Ts>; } && ...));

  return std::tuple_cat(std::forward<Ts>(tuples)...);
}

template <typename First, typename... Rest>
consteval bool is_repeated() {
  return (std::is_same_v<First, Rest> || ...);
}

template <typename First, typename... Rest>
consteval auto unique_tuple(std::tuple<First, Rest...>) {
  if constexpr (sizeof...(Rest) == 0) {
    return std::tuple<First>{};
  } else if constexpr (is_repeated<First, Rest...>()) {
    return unique_tuple(std::tuple<Rest...>{});
  } else {
    return std::tuple_cat(std::tuple<First>{},
                          unique_tuple(std::tuple<Rest...>{}));
  }
}

template <typename T_worker, typename T_task>
consteval bool is_dummy_task() {
  return !(requires(T_worker w, T_task t) { t.run(w); });
}

template <typename T_worker, typename T_task, typename... Rest>
consteval auto remove_dummy_tuple(T_worker*, std::tuple<T_task, Rest...>) {
  if constexpr (is_dummy_task<T_worker, T_task>()) {
    if constexpr (sizeof...(Rest) == 0) {
      return std::tuple<>();
    } else {
      return remove_dummy_tuple(static_cast<T_worker*>(nullptr),
                                std::tuple<Rest...>());
    }
  } else {
    if constexpr (sizeof...(Rest) == 0) {
      return std::tuple<T_task>();
    } else {
      return std::tuple_cat(std::tuple<T_task>(),
                            remove_dummy_tuple(static_cast<T_worker*>(nullptr),
                                               std::tuple<Rest...>()));
    }
  }
}

template <typename T_task>
consteval bool is_storage_task() {
  return (requires(T_task t) { typename T_task::data; });
}

template <typename T_task, typename... Rest>
consteval auto make_data_tuple(std::tuple<T_task, Rest...>) {
  if constexpr (is_storage_task<T_task>()) {
    // 这里的 data 都是 tuple
    using T_data = typename T_task::data;
    static_assert((requires { std::tuple_size_v<T_data>; }));

    if constexpr (sizeof...(Rest) == 0) {
      return T_data{};
    } else {
      return std::tuple_cat(T_data{}, make_data_tuple(std::tuple<Rest...>{}));
    }
  } else {
    if constexpr (sizeof...(Rest) == 0) {
      return std::tuple<>();
    } else {
      return make_data_tuple(std::tuple<Rest...>{});
    }
  }
}

template <typename... Ts>
consteval auto tuple_to_variant(std::tuple<Ts...>) {
  return std::variant<std::monostate, Ts...>{};
}

template <typename Tuple>
consteval cooperation get_co_type() {
  if constexpr (requires(Tuple t) { std::get<0>(t).co_type; }) {
    using T = decltype(std::get<0>(std::declval<Tuple>()));
    return std::remove_reference_t<T>::co_type;
  } else {
    return cooperation::exclusive;
  }
}

// 以上是 consteval 函数。
// 以下是 struct，我保留了一部分 struct，这样才能知道这里是元编程。
template <typename, typename>
struct is_in_tuple : std::false_type {};

template <typename Item, typename... Args>
struct is_in_tuple<Item, std::tuple<Args...>> {
  static constexpr bool value = is_repeated<Item, Args...>();
};

template <typename>
struct for_each;

template <typename... Args>
struct for_each<std::tuple<Args...>> {
  static void before(auto& worker) {
    auto proc = [&worker]<typename T>(T*) {
      if constexpr (requires { T::before(worker); }) {
        T::before(worker);
      }
      static_assert(
          !(requires { T::before(); }),
          "The static function before() without parameters will be ignored. "
          "Please use `auto&' as the first parameter.");
    };
    (proc(static_cast<Args*>(nullptr)), ...);
  }

  static void after(auto& worker) {
    auto proc = [&worker]<typename T>(T*) {
      if constexpr (requires { T::after(worker); }) {
        T::after(worker);
      }
      static_assert(
          !(requires { T::after(); }),
          "The static function after() without parameters will be ignored. "
          "Please use `auto&' as the first parameter.");
    };
    (proc(static_cast<Args*>(nullptr)), ...);
  }
};

}  // namespace rgm::core::traits