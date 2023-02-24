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

#include "type_traits.hpp"

namespace rgm::core {
template <typename...>
struct scheduler;

template <>
struct scheduler<> {
  std::stop_source stop_source;
};

template <typename T_async, typename... T_workers>
struct scheduler<T_async, T_workers...> : scheduler<> {
  static constexpr bool is_asynchronized = T_async::value;

  std::tuple<T_workers...> workers;
  void run() {
    std::apply([this](auto&... worker) { ((worker.p_scheduler = this), ...); },
               workers);

    if constexpr (T_async::value) {
      static_assert((T_workers::is_asynchronized && ...));
      static_assert(std::same_as<T_async, std::true_type>);

      run_async();
    } else {
      static_assert((!T_workers::is_asynchronized && ...));
      static_assert(std::same_as<T_async, std::false_type>);

      run_sync();
    }
  }

  void run_async() {
    auto jthread_tuple = std::apply(
        [](auto&... worker) {
          return std::make_tuple(std::jthread([&worker] { worker.run(); })...);
        },
        workers);
  }

  void run_sync() {
    std::apply([](auto&... worker) { (worker.before(), ...); }, workers);
    std::apply([](auto&... worker) { (worker.kernel_run(), ...); }, workers);
    std::apply([](auto&... worker) { (worker.after(), ...); }, workers);
  }

  template <typename T_task>
  bool broadcast(T_task&& task) {
    auto set_task = [&task](auto&... worker) {
      auto get_task = []<typename T_worker>(T_worker& worker, T_task& task) {
        if constexpr (traits::is_repeated_v<
                          T_task, typename T_worker::T_kernel_tasks>) {
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