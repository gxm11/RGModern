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
#include "config.hpp"

/// @brief 使用元编程的方式完成类型萃取
namespace rgm::core::traits {
template <typename T, typename... Args>
consteval auto append_tuple(T, std::tuple<Args...>) {
  return std::tuple<T, Args...>{};
}

/// @brief 将一个类型添加到 std::tuple 中，作为其第一个元素
/// @tparam Item 新添加的类型
/// @tparam Tuple 目标 std::tuple 类型
template <typename Item, typename Tuple>
using append_tuple_t =
    decltype(append_tuple(std::declval<Item>(), std::declval<Tuple>()));

template <typename... Ts>
consteval auto expand_tuples(Ts... tuples) {
  static_assert((requires { std::tuple_size_v<Ts>; } && ...));

  return std::tuple_cat(std::forward<Ts>(tuples)...);
}

/// @brief 将多个 std::tuple 的内容合并到新的 std::tuple 中
/// @tparam Ts... 接受多个 std::tuple 类型
template <typename... Ts>
using expand_tuples_t = decltype(expand_tuples(std::declval<Ts>()...));

template <typename First, typename... Rest>
consteval bool is_unique() {
  return (!std::same_as<First, Rest> && ...);
}

consteval auto unique_tuple(std::tuple<>) { return std::tuple<>{}; }

template <typename First, typename... Rest>
consteval auto unique_tuple(std::tuple<First, Rest...>) {
  if constexpr (sizeof...(Rest) == 0) {
    return std::tuple<First>{};
  } else if constexpr (is_unique<First, Rest...>()) {
    return append_tuple(First{}, unique_tuple(std::tuple<Rest...>{}));
  } else {
    return unique_tuple(std::tuple<Rest...>{});
  }
}

/// @brief 移除 std::tuple 中重复的元素
/// @tparam Tuple 目标 std::tuple
template <typename Tuple>
using unique_tuple_t = decltype(unique_tuple(std::declval<Tuple>()));

template <typename T_worker, typename T_task>
consteval bool is_dummy_task() {
  return !(requires(T_worker w, T_task t) { t.run(w); });
}

template <typename T_worker, typename T_task, typename... Rest>
consteval auto remove_dummy_tuple(T_worker*, std::tuple<T_task, Rest...>) {
  if constexpr (is_dummy_task<T_worker, T_task>()) {
    if constexpr (sizeof...(Rest) == 0) {
      return std::tuple<>{};
    } else {
      return remove_dummy_tuple(static_cast<T_worker*>(nullptr),
                                std::tuple<Rest...>{});
    }
  } else {
    if constexpr (sizeof...(Rest) == 0) {
      return std::tuple<T_task>{};
    } else {
      return append_tuple(T_task{},
                          remove_dummy_tuple(static_cast<T_worker*>(nullptr),
                                             std::tuple<Rest...>{}));
    }
  }
}

/// @brief 用于将任务列表中的 dummy 任务（即未定义 run 函数的任务）移除
/// @tparam T_tasks 任务列表对应的 std::tuple
/// @tparam T_worker 执行此任务的 worker 类型
template <typename T_worker, typename T_tasks>
using remove_dummy_t = decltype(remove_dummy_tuple(
    static_cast<T_worker*>(nullptr), std::declval<T_tasks>()));

template <typename T_task>
consteval bool is_storage_task() {
  return (requires { typename T_task::data; });
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
      return std::tuple<>{};
    } else {
      return make_data_tuple(std::tuple<Rest...>{});
    }
  }
}

/// @brief 用于将任务列表中的任务的 data 类型提取出来，返回新的 std::tuple
/// @tparam T_tasks 待提取 data 的任务列表
template <typename T_tasks>
using make_data_t = decltype(make_data_tuple(std::declval<T_tasks>()));

/// @brief 将 std::tuple 转换成 std::variant，并添加 std::monostate
template <typename... Ts>
consteval auto tuple_to_variant(std::tuple<Ts...>) {
  return std::variant<std::monostate, Ts...>{};
}

/// @brief 利用结构化绑定，将 struct 映射为相应的 std::tuple
/// 在宏 RGMBIND 和 RGMBIND2 中使用，至多支持 8 个成员变量。
template <size_t n>
consteval auto struct_to_tuple(auto&& s) {
  static_assert(n >= 0 && n <= 8,
                "number of fields of this struct must be between 0 and 8");
  if constexpr (n == 0) {
    return std::tuple{};
  } else if constexpr (n == 1) {
    auto [a] = s;
    return std::tuple{a};
  } else if constexpr (n == 2) {
    auto [a, b] = s;
    return std::tuple{a, b};
  } else if constexpr (n == 3) {
    auto [a, b, c] = s;
    return std::tuple{a, b, c};
  } else if constexpr (n == 4) {
    auto [a, b, c, d] = s;
    return std::tuple{a, b, c, d};
  } else if constexpr (n == 5) {
    auto [a, b, c, d, e] = s;
    return std::tuple{a, b, c, d, e};
  } else if constexpr (n == 6) {
    auto [a, b, c, d, e, f] = s;
    return std::tuple{a, b, c, d, e, f};
  } else if constexpr (n == 7) {
    auto [a, b, c, d, e, f, g] = s;
    return std::tuple{a, b, c, d, e, f, g};
  } else if constexpr (n == 8) {
    auto [a, b, c, d, e, f, g, h] = s;
    return std::tuple{a, b, c, d, e, f, g, h};
  }
}

/*
 * 前面定义的函数都是在非求值上下文中，比如 using T = decltype(func());
 * 从这里开始的函数会用在求值上下文中，从而参数使用了 tuple 的指针避免实例化。
 */

/// @brief 获取一个类型在 std::tuple 的类型参数列表中的位置
/// @tparam Tuple 目标 std::tuple
/// @tparam Item 要查找的类型
/// @return 若存在则返回 Item 在 Tuple 的类型参数列表里的位置，否则返回 Tuple
/// 的大小
template <typename Tuple, typename Item>
consteval size_t tuple_index() {
  auto tuple_find = []<typename... Args>(std::tuple<Args...>*) -> size_t {
    size_t i = 0;
    ((++i, std::same_as<Args, Item>) || ... || ++i);
    return i - 1;
  };
  return tuple_find(static_cast<Tuple*>(nullptr));
}

/// @brief 查找一个类型是否在 std::tuple 的类型参数列表中
/// @tparam Tuple 目标 std::tuple
/// @tparam Item 要查找的类型
/// @return 若存在则返回 true 否则返回 false
template <typename Tuple, typename Item>
consteval bool tuple_include() {
  return tuple_index<Tuple, Item>() != std::tuple_size_v<Tuple>;
}

/*
 * 以上是 consteval 函数。以下是模板偏特化的元编程
 */

template <typename>
struct for_each;

/// @brief for_each 模板类，用于依次执行任务的 before 和 after 函数
/// @tparam std::tuple<Args...> 传入的任务列表
template <typename... Args>
struct for_each<std::tuple<Args...>> {
  static void before(auto& worker) {
    auto proc = [&worker]<typename T>(T*) {
      /*
       * 这里不把 requires 语句写到下面的 constexpr if 条件中，
       * 是为了让 MSVC 能顺利运行。否则 MSVC 会错误地忽略 before 函数。
       * 下面 after 函数的处理也是一样。
       */
      constexpr bool condition = requires { T::before(worker); };
      if constexpr (condition) {
        T::before(worker);
      }
      /* 检查开发者是否遗忘了写 before 的 worker 参数，给出编译期提示 */
      static_assert(!(requires {
        T::before();
      }), "The static function before() without parameters will be ignored. "
          "Please use `auto&' as the first parameter.");
    };
    (proc(static_cast<Args*>(nullptr)), ...);
  }

  static void after(auto& worker) {
    auto proc = [&worker]<typename T>(T*) {
      constexpr bool condition = requires { T::after(worker); };
      if constexpr (condition) {
        T::after(worker);
      }
      /* 检查开发者是否遗忘了写 after 的 worker 参数，给出编译期提示 */
      static_assert(!(requires {
        T::after();
      }), "The static function after() without parameters will be ignored. "
          "Please use `auto&' as the first parameter.");
    };
    (proc(static_cast<Args*>(nullptr)), ...);
  }
};
}  // namespace rgm::core::traits