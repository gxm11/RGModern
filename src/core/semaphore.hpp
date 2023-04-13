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
#include <atomic>
#include <condition_variable>
#include <mutex>

#include "cooperation.hpp"

namespace rgm::core {
#if 0
/// @brief 以原子变量实现的信号量
/// 这个实现似乎仍然有些问题，可能导致无法唤醒，测试代码：
/* ruby
i = 0
loop do
  puts i / 60 if i % 60 == 0
  i += 1
  exit if i == 6000

  Graphics.update
  Input.update
  100.times do
    RGM::Base.synchronize(1)
    RGM::Base.synchronize(2)
    RGM::Base.synchronize(3)
  end
end
*/
struct semaphore {
  std::atomic<int> count;

  explicit semaphore() : count(0) {}

  void acquire() {
    count.fetch_add(1);
    count.wait(1);
  }

  void release() {
    count.fetch_add(-1);
    count.notify_all();
  }
};
#else
/// @brief 以条件变量实现的信号量
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
    cv.notify_all();
  }
};
#endif

/// @brief 此任务用于线程间的同步，只在异步多线程模式下被用到
/// @name 任务类
/// @tparam index 目标 worker 的索引
template <size_t index>
struct synchronize_signal {
  static constexpr cooperation co_type = cooperation::asynchronous;
  static constexpr size_t co_index = index;

  /// @brief 发送此任务的 worker 的信号量指针
  semaphore* pause;

  /// @brief 解除 pause 所属的 worker 的阻塞，使其恢复运行
  /// @param 执行此任务的 worker 对象，被忽略
  void run(auto&) { pause->release(); }
};
}  // namespace rgm::core