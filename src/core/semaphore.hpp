// zlib License

// Copyright (C) [2023] [Xiaomi Guo]

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
#include <atomic>
#include <condition_variable>
#include <mutex>

#include "cooperation.hpp"

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
  static constexpr cooperation co_type = cooperation::asynchronous;

  semaphore* pause;
  void run(auto&) { pause->release(); }
};
}  // namespace rgm::core