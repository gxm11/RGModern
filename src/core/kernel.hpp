// zlib License

// copyright (C) 2023 Xiaomi Guo and Krimiston

// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.

// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:

// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

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
    auto stop_token = worker.get_stop_token();

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
