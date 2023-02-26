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
#include "type_traits.hpp"
#if 0
namespace rgm::core {
/**
 * @brief tasklist 类用于存储一系列的不同类型的任务
 * @tparam Args 可变参数类型，代表内部存储的任务的类型。
 * @note tasklist 是使用继承递归定义的类型，并且可以互相嵌套。
 * 重复出现的任务会被删除。任务的 data 成员可能是 TypeList，
 * 所有任务的 data 成员将组合成总的 datalist 并在任务之间共享。
 */
template <typename...>
struct tasklist;

template <>
struct tasklist<> {
  using tasks = traits::TypeList<>;
  using data = traits::TypeList<>;
};

template <typename T_task, typename... Rest>
struct tasklist<T_task, Rest...> : tasklist<Rest...> {
  using tasks = traits::append_t<T_task, typename tasklist<Rest...>::tasks>;
  using data =
      traits::merge_t<typename T_task::data, typename tasklist<Rest...>::data>;
};

template <typename T_task, typename... Rest>
  requires(!requires { typename T_task::data; })
struct tasklist<T_task, Rest...> : tasklist<Rest...> {
  using tasks = traits::append_t<T_task, typename tasklist<Rest...>::tasks>;
  using data = tasklist<Rest...>::data;
};

template <typename T_task, typename... Rest>
  requires(traits::is_repeated_v<T_task, traits::TypeList<Rest...>>)
struct tasklist<T_task, Rest...> : tasklist<Rest...> {
  using tasks = tasklist<Rest...>::tasks;
  using data = tasklist<Rest...>::data;
};

template <typename T_list, typename... Rest>
  requires(std::is_base_of<tasklist<>, T_list>::value)
struct tasklist<T_list, Rest...> : tasklist<Rest...> {
  using tasks = traits::merge_t<typename T_list::tasks,
                                typename tasklist<Rest...>::tasks>;
  using data =
      traits::merge_t<typename traits::merge_data<typename T_list::tasks>::type,
                      typename tasklist<Rest...>::data>;
};
}  // namespace rgm::core
#endif