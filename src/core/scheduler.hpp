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
#include <thread>

#include "config.hpp"
#include "cooperation.hpp"
#include "type_traits.hpp"

namespace rgm::core {
template <typename T>
struct scheduler_cast {
  using type = T;
};

template <cooperation, typename...>
struct scheduler;

template <cooperation c>
struct scheduler<c> {
  std::stop_source stop_source;
};

template <cooperation c, typename... T_workers>
  requires(sizeof...(T_workers) > 0)
struct scheduler<c, T_workers...> : scheduler<c> {
  static constexpr cooperation co_type = c;

  std::tuple<T_workers...> workers;

  void run() {
    static_assert(((T_workers::co_type == co_type) && ...));

    std::apply([this](auto&... worker) { ((worker.p_scheduler = this), ...); },
               workers);

    if constexpr (co_type == cooperation::asynchronous) {
      run_asynchronous();
    } else if constexpr (co_type == cooperation::exclusive) {
      run_exclusive();
    }
  }

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

  void run_exclusive() {
    std::apply([](auto&... worker) { (worker.before(), ...); }, workers);
    std::apply([](auto&... worker) { (worker.run(), ...); }, workers);
    std::apply([](auto&... worker) { (worker.after(), ...); }, workers);
  }

  template <typename T_task>
  bool broadcast(T_task&& task) {
    auto set_task = [&task](auto&... worker) {
      auto get_task = []<typename T_worker>(T_worker& worker, T_task& task) {
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

    bool ret = std::apply(set_task, workers);
    if constexpr (config::develop) {
      if (!ret) {
        printf("WARN: There's ingored task, check your code.\n");
      }
    }
    return ret;
  }
};
}  // namespace rgm::core