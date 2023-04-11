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
#include "cooperation.hpp"
#include "kernel.hpp"
#include "scheduler.hpp"
#include "semaphore.hpp"
#include "type_traits.hpp"

namespace rgm::core {
/**
 * @brief worker 类用于执行一系列的任务。
 *
 * @tparam T_kernel 任务执行的逻辑，分为主动模式和被动模式。
 * @tparam Args 可以执行的任务列表。
 */
template <template <typename> class T_kernel, typename T_flag, typename... Args>
struct worker {
  template <typename T>
  using kernel_type = T_kernel<T>;

  using T_all_tasks = decltype(traits::expand_tuples(std::declval<Args>()...));
  using T_all_tasks2 = decltype(traits::append_tuple(
      std::declval<T_flag>(), std::declval<T_all_tasks>()));
  using T_tasks = decltype(traits::unique_tuple(std::declval<T_all_tasks2>()));
  using T_kernel_tasks = decltype(traits::remove_dummy_tuple(
      static_cast<worker*>(nullptr), std::declval<T_tasks>()));
  using T_all_data = decltype(traits::make_data_tuple(std::declval<T_tasks>()));
  using T_data = decltype(traits::unique_tuple(std::declval<T_all_data>()));

  static constexpr cooperation co_type = T_flag::co_type;
  static constexpr cooperation co_index = T_flag::co_index;
  static constexpr bool is_active =
      std::is_base_of_v<kernel_active<T_kernel_tasks>,
                        T_kernel<T_kernel_tasks>>;
  static constexpr bool is_asynchronized =
      (co_type == cooperation::asynchronous);

  /**
   * @brief 任务执行的逻辑
   * @tparam T_kernel_tasks 包含了所有可以执行的任务的 TypeList
   */
  T_kernel<T_kernel_tasks> m_kernel;
  /** 保存父类的指针地址用于向下转型为 scheduler<> 的派生类指针 */
  inline static scheduler<>* p_scheduler = nullptr;
  /** T_data 类，存储的变量可供所有的任务读写 */
  std::unique_ptr<T_data> p_data;

  /**
   * @brief 根据不同的变量类型，获取相应的共享变量。
   *
   * @tparam T 变量类型
   * @return auto& 返回 T 类型变量的引用
   */
  template <typename T>
  T& get() {
    constexpr size_t index = traits::tuple_index<T_data, T>();
    return std::get<index>(*p_data);
  }

  static std::stop_token get_stop_token() {
    return p_scheduler->stop_source.get_token();
  }

  /**
   * @brief worker 主要执行的内容，会在单独的线程里执行。
   * @note 按照以下顺序执行：
   * 1. 创建 datalist (before)
   * 2. 按顺序执行任务列表的静态 before 函数 (before)
   * 3. 执行 kernel 的 run 函数 (kernel_run)
   * 4. 设置 running = false (after)
   * 5. 按倒序执行任务列表的静态 after 函数 (after)
   * 6. 析构 datalist (after)
   */
  void before() {
    if constexpr (config::build_mode < 2) {
      int size = sizeof(typename T_kernel<T_kernel_tasks>::T_variants);
      printf("INFO: worker<%lld> starts running...\n", co_index);
      printf("INFO: cooperation type = %d\n", static_cast<int>(co_type));
      printf("INFO: queue block size = %d\n", size);
      printf("INFO: kernel task size = %lld\n",
             std::tuple_size_v<T_kernel_tasks>);
    }
    p_data = std::make_unique<T_data>();
    traits::for_each<T_tasks>::before(*this);
  }

  void run() {
    if constexpr (is_active || is_asynchronized) {
      m_kernel.run(*this);
    }
  }

  void after() {
    p_scheduler->stop_source.request_stop();
    traits::for_each<T_tasks>::after(*this);

    // 同步模式下，不需要主动销毁变量
    if constexpr (is_asynchronized) {
      p_data.reset();
    }
  }

  /**
   * @brief 广播任务，可能会被某个 worker 接受。
   *
   * @tparam T 被广播的任务类型。
   * @tparam U 辅助 scheduler<co_type>* 向下转型，延迟到 magic_cast 的特化之后。
   * @param task 被广播的任务，必须是右值，表示该任务已经离开作用域。
   * @return true 某个 worker 接受了该任务。
   * @return false 没有任何 worker 接受了该任务。
   */
  template <typename T, cooperation c = co_type>
  static bool send(T&& task) {
    using derived_t = scheduler_cast<c>::type;

    static_assert(std::is_rvalue_reference_v<T&&>,
                  "Task must be passed as R-value!");
    static_assert(!std::is_void_v<derived_t>,
                  "Failed to downcast scheduler<>* !");

    return static_cast<derived_t>(p_scheduler)->broadcast(std::move(task));
  }

  // 向其他线程发送 T 指令，阻塞线程直到 T 指令异步执行完毕。
  template <size_t id>
  void wait() {
    if constexpr (is_asynchronized) {
      bool ret = send(synchronize_signal<id>{&(m_kernel.m_pause)});
      if (ret) m_kernel.m_pause.acquire();
    }
  }

  /** 只有 kernel 为主动模式才生效，清空当前管道内积压的全部任务。 */
  void flush() {
    static_assert(is_active);

    m_kernel.flush(*this);
  }

  /** send 的别名 */
  template <typename T>
  void operator>>(T&& task) {
    static_assert(std::is_rvalue_reference_v<T&&>,
                  "Task must be send as R-value!");

    send(std::move(task));
  }

  template <typename T>
  void operator<<(T&& task) {
    static_assert(std::is_rvalue_reference_v<T&&>,
                  "Task must be passed as R-value!");

    static_assert(traits::tuple_include<T_kernel_tasks, T>());
    if constexpr (is_asynchronized || is_active) {
      m_kernel << std::forward<T>(task);
    } else {
      task.run(*this);
    }
  }
};
}  // namespace rgm::core
