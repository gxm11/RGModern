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
#include <condition_variable>
#include <functional>

namespace rgm::core {
struct semaphore {
  static std::mutex m;

  std::condition_variable cv;
  bool pause;

  explicit semaphore() : cv(), pause(false) {}

  void acquire(std::function<void()> async_call) {
    pause = true;
    async_call();

    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, [this] { return !pause; });
  }

  void release() {
    std::lock_guard<std::mutex> lock(m);
    pause = false;
    cv.notify_one();
  }
};
std::mutex semaphore::m;

/** @brief 任务：使 ruby 线程恢复运行 */
template <size_t>
struct synchronize_signal {
  semaphore* pause;
  void run(auto&) { pause->release(); }
};
}  // namespace rgm::core