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

#include "cameron314/blockingconcurrentqueue.h"
#include "semaphore.hpp"
#include "type_traits.hpp"

namespace rgm::core {
/**
 * @brief 被动模式的 kernel
 *
 * @tparam T_tasks 可以执行的任务列表
 *
 * @note 被动模式的 kernel 会不断地尝试从管道中读取任务并执行，
 * 每次执行任务后会判断 worker 的执行状况，如果不再执行则退出。
 * 如果管道为空，则有最多 1 ms的阻塞后返回空任务。
 */
template <typename T_tasks>
struct kernel_passive {
  using T_variants =
      typename traits::append_t<std::monostate, T_tasks>::to<std::variant>;
  /** 用于阻塞或解锁当前线程的信号量 */
  semaphore m_pause;
  /** 存放所有待执行任务的管道，这是一个多读多写的无锁管道 */
  moodycamel::BlockingConcurrentQueue<T_variants> m_queue;

  /** 添加任务到管道中 */
  template <typename T>
    requires(traits::is_repeated_v<T, T_tasks>)
  void enqueue(T&& t) {
    m_queue.enqueue(std::forward<T>(t));
  }

  /** 持续读取并执行管道里的任务，直到 worker 不再处于 running 状态 */
  void run(auto& worker) {
    auto visitor = [&worker](auto&& item) {
      if constexpr (!std::same_as<std::monostate,
                                  std::decay_t<decltype(item)>>) {
        item.run(worker);
      }
    };

    while (worker.running()) {
      T_variants item;
      m_queue.wait_dequeue_timed(item, std::chrono::milliseconds{1});
      std::visit(visitor, std::move(item));
    }
  }
};
}  // namespace rgm::core