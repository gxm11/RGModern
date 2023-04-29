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
#include "config.hpp"
#include "cooperation.hpp"
#include "type_traits.hpp"

namespace rgm::core {
/// @brief scheduler_cast 的泛化形式，永远映射为 void
/// @tparam scheduler 的合作模式，针对模式进行特化以映射到不同的派生类型
template <cooperation>
struct scheduler_cast {
  using type = void;
};

/// @brief 可变参数模板类 scheduler 的声明
template <typename...>
struct scheduler;

/// @brief 可变参数模板类的无参数特化形式，作为其他 scheduler 的基类使用
template <>
struct scheduler<> {
  /// @brief scheduler 中所有的 worker 都共享同一个 stop_source
  /// worker 中保存了 scheduler<>* 的基类指针，从而可以访问到此成员变量。
  std::stop_source stop_source;

  /// @brief 储存所有协程对象的状态和数据，第一个元素是主协程。
  /// 容器中存储的 std::pair 的第一个元素是 fiber_t 的指针，第二个元素（bool
  /// 型变量）表示此 fiber 是否还在运行。
  std::array<std::pair<fiber_t*, bool>, config::max_workers + 1> fibers;

  /// @brief 在协程中执行的内容
  /// @tparam T_worker worker 的类型
  /// @param fiber fiber_t 的指针，其 userdata 存储了相应 worker 的指针。
  template <typename T_worker>
  static void fiber_run(fiber_t* fiber) {
    if (!fiber->userdata) return;

    /* 读取传入的 worker 指针 */
    T_worker* p_worker = static_cast<T_worker*>(fiber->userdata);

    auto& worker = *p_worker;
    static_assert(worker.is_concurrent);

    /*
     * 在协程外初始化 ruby 会导致 stack too deep，产生 segment fault。
     * 为了兼容 ruby，只能在这里执行 ruby worker 的 before 和 after，
     * 并将所有以协程方式执行的 worker 都这样处理。
     */
    worker.fiber_yield();
    worker.before();
    worker.fiber_yield();
    worker.run();
    worker.fiber_yield();
    worker.after();

    /* 不再需要切换回来 */
    worker.fiber_return();
  }
};

/// @brief 可变参数模板类 scheduler 的特化形式，继承自 scheduler<>
/// @tparam First 第一个 worker，scheduler 至少要有 1 个 worker
/// @tparam Rest... 其他的 worker(s)
template <typename First, typename... Rest>
struct scheduler<First, Rest...> : scheduler<> {
  /// @brief scheduler 的合作模式，所有 worker 的合作模式必须相同
  static constexpr cooperation co_type = First::co_type;

  /// @brief 储存所有 worker 对象的 tuple
  std::tuple<First, Rest...> workers;

  /// @brief run 函数是整个 RGModern 程序的入口
  /// 在 main.cpp 中，首先创建了 scheduler 的对象创建
  /// 然后立刻调用此函数，执行主流程。此函数退出时，程序结束。
  void run() {
    /* 所有 worker 的合作模式必须相同 */
    static_assert(((Rest::co_type == co_type) && ...));

    /* 将 this 作为基类指针赋值给 worker 的 p_scheduler 对象 */
    std::apply([this](auto&... worker) { ((worker.p_scheduler = this), ...); },
               workers);

    if constexpr (co_type == cooperation::asynchronous) {
      cen::log_warn("[Kernel] the cooperation type is asynchronous.");
      run_asynchronous();
    } else if constexpr (co_type == cooperation::exclusive) {
      cen::log_warn("[Kernel] the cooperation type is exclusive.");
      run_exclusive();
    } else if constexpr (co_type == cooperation::concurrent) {
      cen::log_warn("[Kernel] the cooperation type is concurrent.");
      run_concurrent();
    }
  }

  /// @brief 异步多线程模式的执行内容
  /// 在此模式下，对于每一个 worker 都会创建一个线程，
  /// 在线程内，依次执行 worker 的 before / run / after 函数。
  /// 由于使用了 jthread，函数结束时线程会自动 join，全部 join 后函数退出。
  void run_asynchronous() {
    std::apply(
        [](auto&... worker) {
          return std::make_tuple(std::jthread([&worker] {
            worker.before();
            worker.run();
            worker.after();
          })...);
        },
        workers);
  }

  /// @brief 排他单线程模式的执行内容
  /// 在此模式下，首先依次执行每个 worker 的 before 函数，然后依次执行
  /// 每个 worker 的 run 函数，最后依次执行每个 worker 的 after 函数。
  /// 在此模式下，如果 worker 的 kernel 是被动模式，则 run 函数为空。
  void run_exclusive() {
    std::apply([](auto&... worker) { (worker.before(), ...); }, workers);
    std::apply([](auto&... worker) { (worker.run(), ...); }, workers);
    std::apply([](auto&... worker) { (worker.after(), ...); }, workers);
  }

  /// @brief 协程单线程的执行内容
  /// 在此模式下，被动 worker 没有自己的协程，相关的任务都是立即执行；主动
  /// 的 worker 会在 flush 的前后让出执行权，切换到此函数中，并调度下一个
  /// 主动的 worker。
  void run_concurrent() {
    /* 初始化 fibers */
    fibers.fill({nullptr, false});
    fibers[0].first = fiber_create(nullptr, 0, nullptr, nullptr);

    /* 创建协程 */
    std::apply([this](auto&... worker) { (worker.fiber_setup(), ...); },
               workers);

    /* 执行被动 worker 的 before 函数 */
    std::apply(
        [](auto&... worker) {
          auto before = [](auto&& item) {
            if constexpr (!item.is_active) {
              item.before();
            }
          };

          (before(worker), ...);
        },
        workers);

    while (true) {
      /* 不停地在所有协程之间进行切换 */
      for (size_t i = 1; i < fibers.size(); ++i) {
        auto& fiber = fibers[i];

        /* 如果未创建协程，或者协程不处于运行状态，则检查下一个协程 */
        if (!fiber.first || !fiber.second) continue;

        fiber_switch(fiber.first);
      }
      /* 当收到退出信号时，除非所有的协程都不再运行才退出 */
      if (this->stop_source.stop_requested()) {
        auto it =
            std::find_if(fibers.begin(), fibers.end(),
                         [](auto& fiber) -> bool { return fiber.second; });
        if (it == fibers.end()) break;
      }
    }

    /* 执行被动 worker 的 after 函数 */
    std::apply(
        [](auto&... worker) {
          auto after = [](auto&& item) {
            if constexpr (!item.is_active) {
              item.after();
            }
          };

          (after(worker), ...);
        },
        workers);
  }

  /// @brief 将某个 task 分配给能接受此 task 的 worker
  /// @tparam T_task 被分配的 task 的类型
  /// @return 如果此 task 不被任何 worker 接受，则返回 false，否则返回 true
  template <typename T_task>
  bool broadcast(T_task&& task) {
    auto set_task = [&task](auto&... worker) {
      auto get_task = []<typename T_worker>(T_worker& worker, T_task& task) {
        /* 查询 worker 的 kernel 是否支持此任务类型 */
        if constexpr (traits::tuple_include<typename T_worker::T_kernel_tasks,
                                            T_task>()) {
          worker << std::move(task);
          return true;
        } else {
          return false;
        }
      };

      return (get_task(worker, task) || ...);
    };

    /* 遍历所有的 workers 并执行 set_task 操作 */
    bool ret = std::apply(set_task, workers);

    /* 只在 main.exe 和 debug.exe 下会提醒开发者某个 task 分配失败 */
    if constexpr (config::develop) {
      if (!ret) {
        /* 因为使用了 typeid，编译时不能关掉 rtti */
        cen::log_warn("There's ingored task <%s>, check your code.\n",
                      typeid(T_task).name());
      }
    }
    return ret;
  }
};
}  // namespace rgm::core