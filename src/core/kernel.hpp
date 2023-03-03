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
#include <optional>
#include <thread>
#include <variant>

#include "blockingconcurrentqueue.h"
#include "semaphore.hpp"
#include "type_traits.hpp"

namespace rgm::core {
template <typename T_tasks, bool active>
struct kernel {
  /** 用于阻塞或解锁当前线程的信号量 */
  semaphore m_pause;

  /** 存放所有待执行任务的管道，这是一个多读多写的无锁管道 */
  using T_variants =
      decltype(traits::tuple_to_variant(std::declval<T_tasks>()));
  moodycamel::BlockingConcurrentQueue<T_variants> m_queue;

  template <typename T>
  auto& operator<<(T&& t) {
    m_queue.enqueue(std::forward<T>(t));
    return *this;
  }

  void flush(auto& worker) {
    auto stop_token = worker.template get_stop_token();

    auto visitor = [&worker]<typename T>(T& item) {
      if constexpr (!std::same_as<std::monostate, T>) {
        item.run(worker);
      }
    };

    while (!stop_token.stop_requested()) {
      T_variants item;

      if constexpr (active) {
        if (!m_queue.try_dequeue(item)) break;
      } else {
        m_queue.wait_dequeue_timed(item, std::chrono::milliseconds{1});
      }

      std::visit(visitor, item);
    }
  }

  void run(auto& worker) { flush(worker); }
};

template <typename T_tasks>
using kernel_active = kernel<T_tasks, true>;

template <typename T_tasks>
using kernel_passive = kernel<T_tasks, false>;
}  // namespace rgm::core
