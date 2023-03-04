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
#include <thread>

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
        if constexpr (traits::tuple_include<typename T_worker::T_kernel_tasks, T_task>()) {
          worker << std::move(task);
          return true;
        } else {
          return false;
        }
      };

      return (get_task(worker, task) || ...);
    };

    bool ret = std::apply(set_task, workers);
    if constexpr (config::output_level > 0) {
      if (!ret) {
        printf("There's ingored task, check your code.\n");
      }
    }
    return ret;
  }
};
}  // namespace rgm::core