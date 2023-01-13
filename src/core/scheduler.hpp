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
#include <thread>

#include "type_traits.hpp"

namespace rgm::core {
/**
 * @brief scheduler 内部组合了多个 worker，并在不同线程里执行 worker 的 run
 * 函数。
 * @tparam Args 可变参数类型，代表内部组合的 worker。
 * @note scheduler 是使用继承递归定义的类型。
 */
template <typename...>
struct scheduler;

/**
 * @brief scheduler 的基类，其指针可以使用 magic_cast 向下转型成派生类的指针。
 */
template <>
struct scheduler<> {
  /**
   * @brief 标志当前 scheduler 是否在执行。
   * @note 初始为真。worker 保存了基类的指针，可以访问到该变量。
   * worker 在其 kernel 的 run 函数退出后会将其更新为假。
   * worker 的 run 函数在检测到 scheduler 为假时也会退出。
   */
  std::stop_source stop_source;
};

template <typename T_worker, typename... Rest>
struct scheduler<T_worker, Rest...> : scheduler<Rest...> {
  T_worker m_worker;

  /** 创建不同的线程，执行 worker 的 run 函数 */
  void run() {
    m_worker.p_scheduler = this;
    if constexpr (sizeof...(Rest) > 0) {
      std::jthread t([this] { m_worker.run(); });
      scheduler<Rest...>::run();
    } else {
      m_worker.run();
    }
  }

  /** 广播一个任务，如果该任务在某个 worker
   * 的可执行任务列表中，就放到其任务队列里 */
  template <typename T_task>
  bool broadcast(T_task&& task) {
    if constexpr (traits::is_repeated_v<T_task, typename T_worker::T_tasks>) {
      m_worker.m_kernel.enqueue(std::forward<T_task>(task));
      return true;
    } else if constexpr (sizeof...(Rest) > 0) {
      return scheduler<Rest...>::template broadcast(std::forward<T_task>(task));
    } else if constexpr (config::output_level > 0) {
      printf("There's ingored task, check your code.\n");
    }
    return false;
  }
};
}  // namespace rgm::core