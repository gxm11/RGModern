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
#include <type_traits>
#include <variant>

/**
 * @brief 类型萃取
 * @note 有一说一，这件事大家懂得都懂，不懂得，说了你也不明白，不如不说。
 * 你们也别来问我怎么了，利益牵扯太大，说了对你们也没什么好处，当不知道就行了，
 * 其余的我只能说这里面水很深，牵扯到很多大人物。详细资料你们自己找是很难找的，
 * 网上大部分已经删除干净了，所以我只能说懂得都懂，不懂得也没办法。
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
using append_t = append<First, Second>::type;

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
  using type = unique<TypeList<First..., Second...>>::type;
};

template <typename First, typename Second>
using merge_t = merge<First, Second>::type;

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

template <typename>
struct for_each;

template <>
struct for_each<TypeList<>> {
  static void before(auto&) {}
  static void after(auto&) {}
};

template <typename Head, typename... Rest>
struct for_each<TypeList<Head, Rest...>> : for_each<TypeList<Rest...>> {
  static void before(auto& worker) {
    if constexpr (requires { Head::before(worker); }) {
      Head::before(worker);
    }
    for_each<TypeList<Rest...>>::before(worker);
  }

  static void after(auto& worker) {
    for_each<TypeList<Rest...>>::after(worker);
    if constexpr (requires { Head::after(worker); }) {
      Head::after(worker);
    }
  }
};

template <typename, typename>
struct remove_dummy;

template <typename T, typename U>
using remove_dummy_t = remove_dummy<T, U>::type;

template <typename U>
struct remove_dummy<TypeList<>, U> {
  using type = TypeList<>;
};

template <typename U, typename Head, typename... Args>
struct remove_dummy<TypeList<Head, Args...>, U>
    : remove_dummy<TypeList<Args...>, U> {
  using type = remove_dummy<TypeList<Args...>, U>::type;
};

template <typename U, typename Head, typename... Args>
  requires(requires(Head a, U b) { a.run(b); })
struct remove_dummy<TypeList<Head, Args...>, U>
    : remove_dummy<TypeList<Args...>, U> {
  using type =
      append_t<Head, typename remove_dummy<TypeList<Args...>, U>::type>;
};

template <typename T>
struct magic_cast {
  using type = T;
};
}  // namespace rgm::core::traits
