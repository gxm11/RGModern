// zlib License
//
// copyright (C) 2023 Guoxiaomi and Krimiston
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once
#include "cooperation.hpp"
#include "kernel.hpp"
#include "scheduler.hpp"
#include "semaphore.hpp"
#include "type_traits.hpp"

namespace rgm::core {
/// @brief worker 类用于管理数据，发送和执行任务
/// @tparam T_flag worker 的合作模式：异步多线程 / 排他单线程 / 协程单线程
/// @tparam T_kernel worker 运作任务的核，分为主动模式和被动模式
/// @tparam T_all_tasks worker 的任务列表。
template <typename T_flag, template <typename> class T_kernel,
          typename T_all_tasks>
struct worker {
  /// @brief 如果合作模式是异步多线程，则添加 synchronize_signal 任务到列表中
  using T_all_tasks_with_flag = std::conditional_t<
      T_flag::co_type == cooperation::asynchronous,
      traits::append_tuple_t<synchronize_signal<T_flag::co_index>, T_all_tasks>,
      T_all_tasks>;

  /// @brief 移除上一步获得的任务列表中重复的部分获得 T_tasks
  using T_tasks = traits::unique_tuple_t<T_all_tasks_with_flag>;

  /// @brief 读取所有 task 的 data 类型，去重后拼合成为 worker 的数据类型
  /// data 类型必须是一个 std::tuple<...>，std::tuple 中的每个元素类型都
  /// 必须有默认的无参数构造函数，否则会编译失败。
  using T_data = traits::unique_tuple_t<traits::make_data_t<T_tasks>>;

  /// @brief 移除 T_tasks 中未定义 run 函数的任务获得 T_kernel_tasks
  using T_kernel_tasks = traits::remove_dummy_t<worker, T_tasks>;

  /// @brief worker 的合作模式
  static constexpr cooperation co_type = T_flag::co_type;

  /// @brief worker 的索引，不同的 worker 此值必须设置为不同
  static constexpr size_t co_index = T_flag::co_index;

  /// @brief worker 的核是否是主动模式
  static constexpr bool is_active =
      std::is_base_of_v<kernel_active<T_kernel_tasks>,
                        T_kernel<T_kernel_tasks>>;

  /// @brief worker 的合作模式是否为异步多线程
  static constexpr bool is_asynchronized =
      (co_type == cooperation::asynchronous);

  /// @brief worker 的核，负责任务队列的执行逻辑
  /// @tparam T_kernel 核的类型
  /// @tparam T_kernel_tasks 核的可执行任务的类型
  T_kernel<T_kernel_tasks> m_kernel;

  /// @brief 在 worker 中保存 scheduler<>* 基类的指针地址
  /// 1. 被用于向下转型为 scheduler<...>* 的派生类指针
  /// 2. 用于获取 scheduler<> 的成员变量 std::stop_source
  /// 这是一个静态变量。
  inline static scheduler<>* p_scheduler = nullptr;

  /// @brief worker 独有的数据，供所有的任务读写
  /// @tparam T_data worker 的数据类型，是一个 std::tuple
  /// 通常使用 RGMDATA 宏获得对应数据的引用
  /// 使用智能指针是为了精确控制其生命周期。
  std::unique_ptr<T_data> p_data;

  /// @brief 发送停止信号，在异步多线程模式下还会恢复等待自身的 worker。
  void stop() {
    p_scheduler->stop_source.request_stop();

    if constexpr (is_asynchronized) {
      m_kernel.release_all();
    }
  }

  /// @brief
  static bool is_stopped() { return p_scheduler->stop_source.stop_requested(); }

  void fiber_setup() {
    if constexpr (co_type == cooperation::concurrent && is_active) {
      auto& fiber_main = p_scheduler->fibers[0];
      auto& fiber = p_scheduler->fibers.at(co_index + 1);

      fiber.first = fiber_create(fiber_main.first, 0,
                                 scheduler<>::fiber_run<worker>, this);

      fiber.second = true;
    }
  }

  static void fiber_yield() {
    if constexpr (co_type == cooperation::concurrent && is_active) {
      auto& fiber_main = p_scheduler->fibers[0];

      fiber_switch(fiber_main.first);
    }
  }

  static void fiber_return() {
    if constexpr (co_type == cooperation::concurrent && is_active) {
      auto& fiber_main = p_scheduler->fibers[0];
      auto& fiber = p_scheduler->fibers.at(co_index + 1);

      fiber.second = false;
      fiber_switch(fiber_main.first);
    }
  }

  /// @brief 根据变量类型，获取 worker 保存的对应数据
  /// @tparam T 变量类型
  /// @return T& 返回 T 类型变量的引用
  template <typename T>
  T& get() {
    constexpr size_t index = traits::tuple_index<T_data, T>();
    return std::get<index>(*p_data);
  }

  /// @brief worker 执行的第 1 个步骤
  /// 1. 输出 worker 的调试信息
  /// 2. 创建数据，用智能指针 p_data 管理
  /// 3. 执行 T_tasks 的 before 函数
  void before() {
    if constexpr (config::develop) {
      int size = sizeof(typename T_kernel<T_kernel_tasks>::T_variants);
      cen::log_info(
          "worker %lld's kernel has a queue with block size = %d, "
          "total %lld tasks and %lld kernel tasks.",
          co_index, size, std::tuple_size_v<T_tasks>,
          std::tuple_size_v<T_kernel_tasks>);
    }
    p_data = std::make_unique<T_data>();
    traits::for_each<T_tasks>::before(*this);
  }

  /// @brief worker 执行的第 2 个步骤
  /// 执行核的 run 函数。
  /// 只有主动线程，或者是异步多线程的合作模式才有效，否则什么也不做。
  void run() {
    cen::log_info("worker %lld starts running...", co_index);
    if constexpr (is_active || is_asynchronized) {
      m_kernel.run(*this);
    }
    cen::log_info("worker %lld terminated.", co_index);
  }

  /// @brief worker 执行的第 3 个步骤
  /// 1. 修改各 worker 共享的 stop_source 为 stop 状态
  /// 2. 执行 T_tasks 的 after 函数
  /// 3. 删除智能指针 p_data 管理数据
  /// (3) 只在异步多线程模式下删除数据，单线程模式下 p_data 跟随 worker
  /// 的生命周期。
  void after() {
    stop();
    traits::for_each<T_tasks>::after(*this);

    // 同步模式下，不需要主动销毁变量
    if constexpr (is_asynchronized) {
      p_data.reset();
    }
  }

  /// @brief 广播一个任务，查询所有的 worker 是否可以接受它
  /// @tparam T 被广播的任务类型。
  /// @tparam c 辅助 scheduler<>* 向下转型，使用默认参数推迟编译时的推导时机
  /// @param task 被广播的任务，右值表示该任务的生命周期已经由目标 worker 接管
  /// @return true 某个 worker 接受了该任务。
  /// @return false 没有任何 worker 接受了该任务。
  /// 这是一个静态函数，调用静态成员变量 p_scheduer 执行。
  template <typename T, cooperation c = co_type>
  static bool send(T&& task) {
    using derived_t = scheduler_cast<c>::type;

    static_assert(std::is_rvalue_reference_v<T&&>,
                  "Task must be passed as R-value!");
    static_assert(!std::is_void_v<derived_t>,
                  "Failed to downcast scheduler<>* !");

    return static_cast<derived_t>(p_scheduler)->broadcast(std::move(task));
  }

  /// @brief 使当前线程的运行和目标线程同步
  /// @tparam id 要等待的线程的 co_index
  /// 1. 当前线程发送 synchronize_signal 信号
  /// 2. 当前线程使用信号量 m_kernel.m_pause 阻塞运行
  /// 3. 目标线程将 synchronize_signal 信号添加到队列
  /// 4. 目标线程执行 synchronize_signal 的 run 函数，解锁当前线程
  template <size_t id>
  void wait() {
    if (is_stopped()) return;

    if constexpr (is_asynchronized) {
      bool ret = send(synchronize_signal<id>{&(m_kernel.m_pause)});
      if (ret) m_kernel.m_pause.acquire();
    }
  }

  /// @brief 依次执行当前核的任务队列中的任务，直到队列清空
  void flush() {
    /* 如果核为被动模式，调用此函数会导致编译错误 */
    static_assert(is_active);

    m_kernel.flush(*this);
  }

  /// @brief send 的别名
  /// @return 返回 *this
  /// 这是一个成员函数，区别于 send 是一个静态函数。
  template <typename T>
  worker& operator>>(T&& task) {
    static_assert(std::is_rvalue_reference_v<T&&>,
                  "Task must be send as R-value!");

    send(std::move(task));
    return *this;
  }

  /// @brief 接受一个合法的任务
  /// @return 返回 *this
  /// 1. 异步多线程模式，或者是核是主动模式，则任务进入队列等待执行；
  /// 2. 其他情况任务会立刻执行。
  template <typename T>
  worker& operator<<(T&& task) {
    static_assert(std::is_rvalue_reference_v<T&&>,
                  "Task must be passed as R-value!");

    static_assert(traits::tuple_include<T_kernel_tasks, T>());

    if constexpr (is_asynchronized || is_active) {
      m_kernel << std::forward<T>(task);
    } else {
      task.run(*this);
    }
    return *this;
  }
};
}  // namespace rgm::core
