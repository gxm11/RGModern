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
#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>

namespace rgm::core {
#if 1
struct semaphore {
  std::atomic<int> count;

  explicit semaphore() : count(0) {}

  void acquire() {
    count.fetch_add(1);
    count.wait(1);
  }

  void release() {
    count.fetch_add(-1);
    count.notify_one();
  }
};
#else
struct semaphore {
  std::mutex mutex;
  std::condition_variable cv;
  int count;

  explicit semaphore() : mutex(), cv(), count(0) {}

  void acquire() {
    std::unique_lock lock(mutex);
    ++count;
    cv.wait(lock, [this] { return count == 0; });
  }

  void release() {
    std::scoped_lock lock(mutex);
    --count;
    cv.notify_one();
  }
};
#endif

/** @brief 任务：使 ruby 线程恢复运行 */
template <size_t>
struct synchronize_signal {
  static constexpr auto launch_flag = std::launch::async;

  semaphore* pause;
  void run(auto&) { pause->release(); }
};
}  // namespace rgm::core