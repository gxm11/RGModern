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
#include <future>
#include <type_traits>
#include <variant>

/**
 * @brief 类型萃取
 * @note 有一说一，这件事大家懂得都懂，不懂得，说了你也不明白，不如不说。
 * 你们也别来问我怎么了，利益牵扯太大，说了对你们也没什么好处，当不知道就行了，
 * 其余的我只能说这里面水很深，牵扯到很多大人物。详细资料你们自己找是很难找的，
 * 网上大部分已经删除干净了，所以我只能说懂得都懂，不懂得也没办法。XD
 */
namespace rgm::core::traits {
template <typename... Ts>
struct TypeList {
  template <template <typename...> typename T>
  using to = T<Ts...>;
};

template <typename, typename>
struct is_repeated : std::false_type {};

template <typename T, typename... Args>
  requires(std::same_as<T, Args> || ... || false)
struct is_repeated<T, TypeList<Args...>> : std::true_type {};

template <typename T, typename U>
constexpr bool is_repeated_v = is_repeated<T, U>::value;

template <typename, typename>
struct append;

template <typename First, typename... Second>
struct append<First, TypeList<Second...>> {
  using type = TypeList<First, Second...>;
};

template <typename First, typename... Second>
  requires(is_repeated_v<First, TypeList<Second...>>)
struct append<First, TypeList<Second...>> {
  using type = TypeList<Second...>;
};

template <typename First, typename Second>
using append_t = typename append<First, Second>::type;

template <typename>
struct unique;

template <>
struct unique<TypeList<>> {
  using type = TypeList<>;
};

template <typename Head, typename... Args>
struct unique<TypeList<Head, Args...>> : unique<TypeList<Args...>> {
  using type = append_t<Head, typename unique<TypeList<Args...>>::type>;
};

template <typename Head, typename... Args>
  requires(is_repeated_v<Head, TypeList<Args...>>)
struct unique<TypeList<Head, Args...>> : unique<TypeList<Args...>> {
  using type = unique<TypeList<Args...>>::type;
};

template <typename, typename>
struct merge;

template <typename... First, typename... Second>
struct merge<TypeList<First...>, TypeList<Second...>> {
  using type = typename unique<TypeList<First..., Second...>>::type;
};

template <typename First, typename Second>
using merge_t = typename merge<First, Second>::type;

template <typename>
struct merge_data;

template <>
struct merge_data<TypeList<>> {
  using type = TypeList<>;
};

template <typename T_task, typename... Rest>
struct merge_data<TypeList<T_task, Rest...>> : merge_data<TypeList<Rest...>> {
  using type = merge_t<typename T_task::data,
                       typename merge_data<TypeList<Rest...>>::type>;
};

template <typename T_task, typename... Rest>
  requires(!requires { typename T_task::data; })
struct merge_data<TypeList<T_task, Rest...>> : merge_data<TypeList<Rest...>> {
  using type = merge_data<TypeList<Rest...>>::type;
};

template <typename...>
struct for_each;

template <typename... Args>
struct for_each<TypeList<Args...>> {
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

template <typename, typename>
struct remove_dummy;

template <typename T, typename U>
using remove_dummy_t = typename remove_dummy<T, U>::type;

template <typename U>
struct remove_dummy<TypeList<>, U> {
  using type = TypeList<>;
};

template <typename U, typename Head, typename... Args>
struct remove_dummy<TypeList<Head, Args...>, U>
    : remove_dummy<TypeList<Args...>, U> {
  using type = typename remove_dummy<TypeList<Args...>, U>::type;
};

template <typename U, typename Head, typename... Args>
  requires(requires(Head a, U b) { a.run(b); })
struct remove_dummy<TypeList<Head, Args...>, U>
    : remove_dummy<TypeList<Args...>, U> {
  using type =
      append_t<Head, typename remove_dummy<TypeList<Args...>, U>::type>;
};

template <typename...>
struct is_asynchronized;

template <>
struct is_asynchronized<> : std::false_type {};

template <typename Head, typename... Args>
struct is_asynchronized<Head, Args...> : is_asynchronized<Args...> {
  static constexpr bool value = is_asynchronized<Args...>::value;
};

template <typename Head, typename... Args>
  requires(requires { Head::launch_flag; })
struct is_asynchronized<TypeList<Head, Args...>>
    : is_asynchronized<TypeList<Args...>> {
  static constexpr bool value = (Head::launch_flag == std::launch::async) ||
                                is_asynchronized<TypeList<Args...>>::value;
};

template <typename T>
struct magic_cast {
  using type = T;
};
}  // namespace rgm::core::traits
