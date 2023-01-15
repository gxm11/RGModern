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
#include "scheduler.hpp"

namespace rgm::core {
template <typename...>
struct scheduler_synchronized;

template <>
struct scheduler_synchronized<> : scheduler<> {};

template <typename T_worker, typename... Rest>
struct scheduler_synchronized<T_worker, Rest...>
    : scheduler_synchronized<Rest...> {
  T_worker m_worker;

  /** 最后一个 worker 必须有 active kernel */
  void run() {
    m_worker.p_scheduler = this;
    m_worker.before();
    if constexpr (sizeof...(Rest) > 0) {
      static_assert(
          !m_worker.is_active,
          "Workers other than the last one should have the passive kernel!");
      scheduler_synchronized<Rest...>::run();
    } else {
      static_assert(m_worker.is_active,
                    "The last worker must have the active kernel!");
      m_worker.kernel_run();
    }
    m_worker.after();
  }

  /** 广播一个任务，如果该任务在某个 worker
   * 的可执行任务列表中，就放到其任务队列里 */
  template <typename T_task>
  bool broadcast(T_task&& task) {
    if constexpr (traits::is_repeated_v<T_task, typename T_worker::T_tasks>) {
      task.run(m_worker);
      return true;
    } else if constexpr (sizeof...(Rest) > 0) {
      return scheduler_synchronized<Rest...>::template broadcast(
          std::forward<T_task>(task));
    } else if constexpr (config::output_level > 0) {
      printf("There's ingored task, check your code.\n");
      return false;
    }
  }
};
}  // namespace rgm::core