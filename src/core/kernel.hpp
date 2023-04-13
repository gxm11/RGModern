// zlib License

// copyright (C) 2023 Guoxiaomi and Krimiston

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
#include "blockingconcurrentqueue.h"
#include "semaphore.hpp"
#include "type_traits.hpp"

namespace rgm::core {
/// @brief worker 运作任务的核，负责处理任务队列
/// @tparam T_tasks 支持的任务类型，除此以外的类型不会在此 kernel 中执行
/// @tparam active 区分核是主动模式还是被动模式。
/// 主动模式拥有一个主动的循环流程，需要调用 flush 以执行队列中的任务；
/// 被动模式只会执行队列中的任务，并在队列为空时阻塞。
/// 主动模式作为基类使用，需要编写相应的派生类，如 base::kernel_ruby 类。
template <typename T_tasks, bool active>
struct kernel {
  /// @brief 用于阻塞或解锁当前线程的信号量，只在异步 worker 中用到
  semaphore m_pause;

  /// @brief 用于保存所有支持的任务类型的 std::variant
  /// 类型，这将减少内存的分配。
  using T_variants =
      decltype(traits::tuple_to_variant(std::declval<T_tasks>()));

  /// @brief 存放所有待执行任务的队列，这是一个多读多写的无锁管道
  moodycamel::BlockingConcurrentQueue<T_variants> m_queue;

  /// @brief 将任务放入队列中
  /// @tparam T 任务的类型
  /// @param t 任务对象，此参数必须是右值引用类型，将所有权交给队列
  /// @return 返回 *this
  template <typename T>
  auto& operator<<(T&& t) {
    m_queue.enqueue(std::forward<T>(t));
    return *this;
  }

  /// @brief 依次执行队列中的任务并清空队列，只有主动线程才会调用此函数。
  /// @param worker 拥有此 kernel 的 worker
  /// worker 将作为入参传递给各个 task 的 run 函数。
  void flush(auto& worker) {
    /** 所有的 worker 都共享同一个 stop_source */
    auto stop_token = worker.get_stop_token();

    auto visitor = [&worker]<typename T>(T& item) {
      if constexpr (!std::same_as<std::monostate, T>) {
        item.run(worker);
      }
    };

    /** 查看 stop_source 的状态，及时退出运行 */
    while (!stop_token.stop_requested()) {
      T_variants item;
      if constexpr (active) {
        /** 主动模式下队列为空就退出循环 */
        if (!m_queue.try_dequeue(item)) break;
      } else {
        /** 被动模式下队列为空则阻塞 1ms，然后继续获取任务 */
        m_queue.wait_dequeue_timed(item, std::chrono::milliseconds{1});
      }

      std::visit(visitor, item);
    }
  }

  /// @brief 默认的 run 函数就是清空队列 
  /// @param worker 拥有此 kernel 的 worker
  /// 被动线程将使用此函数，主动线程需要覆写此函数以执行特定任务。
  void run(auto& worker) { flush(worker); }
};

/// @brief 偏特化为主动模式的核，当作其他核的基类使用
/// @tparam T_tasks 支持的任务类型
template <typename T_tasks>
using kernel_active = kernel<T_tasks, true>;

/// @brief 偏特化为被动模式的核
/// @tparam T_tasks 支持的任务类型
template <typename T_tasks>
using kernel_passive = kernel<T_tasks, false>;
}  // namespace rgm::core
