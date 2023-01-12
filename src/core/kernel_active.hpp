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
#include <variant>

#include "cameron314/concurrentqueue.h"
#include "semaphore.hpp"
#include "type_traits.hpp"

namespace rgm::core {
/**
 * @brief 所有主动模式 kernel 的基类
 *
 * @tparam T_tasks 可以执行的任务列表
 *
 * @note 主动模式的 kernel 必须是 kernel_active 的派生类，并且实现 run 函数。
 * 主动模式下，kernel 收到广播的任务并不会执行，而是主动调用 step 函数执行。
 */
template <typename T_tasks>
struct kernel_active {
  using T_variants =
      typename traits::append_t<std::monostate, T_tasks>::to<std::variant>;
  /** 用于阻塞或解锁当前线程的信号量 */
  semaphore m_pause;
  /** 存放所有待执行任务的管道，这是一个多读多写的无锁管道 */
  moodycamel::ConcurrentQueue<T_variants> m_queue;

  /** 添加任务到管道中 */
  template <typename T>
    requires(traits::is_repeated_v<T, T_tasks>)
  void enqueue(T&& t) {
    m_queue.enqueue(std::forward<T>(t));
  }

  /** 清空当前管道内积压的全部任务 */
  void step(auto& worker) {
    auto visitor = [&worker](auto&& item) {
      if constexpr (!std::same_as<std::monostate,
                                  std::decay_t<decltype(item)>>) {
        item.run(worker);
      }
    };

    while (true) {
      T_variants item;
      if (!m_queue.try_dequeue(item)) {
        return;
      }
      std::visit(visitor, std::move(item));
    }
  }
};
}  // namespace rgm::core