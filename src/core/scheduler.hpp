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
};

/// @brief 可变参数模板类 scheduler 的特化形式，继承自 scheduler<>
/// @tparam First 第一个 worker，scheduler 至少要有 1 个 worker
/// @tparam Rest... 其他的 worker(s)
template <typename First, typename... Rest>
struct scheduler<First, Rest...> : scheduler<> {
  /// @brief scheduler 的合作模式，所有 worker 的合作模式必须相同
  static constexpr cooperation co_type = First::co_type;

  /// @brief 包含了所有 worker 对象的 tuple
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
      cen::log_warn("[Kernel] the cooperation type is asynchronous");
      run_asynchronous();
    } else if constexpr (co_type == cooperation::exclusive) {
      cen::log_warn("[Kernel] the cooperation type is exclusive");
      run_exclusive();
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