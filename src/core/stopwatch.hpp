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
#include <array>
#include <chrono>

#include "config.hpp"

namespace rgm::core {
struct stopwatch_dummy {
  explicit stopwatch_dummy(const char*, int64_t = 0) {}
  void start() {}
  void step(size_t) {}
};

/**
 * @brief 秒表，用于测试一段反复执行代码的性能。
 * @note stopwatch
 * 通常是全局变量，会在程序退出的时候析构，同时打印相关的调试信息。
 */
struct stopwatch_normal {
  struct interval {
    std::chrono::time_point<std::chrono::steady_clock> now;
    double total_time = 0.0;
  };

  static constexpr int max_size = 16;
  static uint64_t total_index;

  std::array<interval, max_size> data;
  const char* name;
  uint64_t index;
  int64_t counts;

  /**
   * @brief stopwatch 的构造函数。
   *
   * @param name 打印结果时显示的名称。
   * @param skip_counts 正式计时前跳过的步数。
   */
  explicit stopwatch_normal(const char* name, int skip_counts = 1) : data() {
    this->name = name;
    this->index = total_index++;
    this->counts = 0 - skip_counts;
  }

  /**
   * @brief stopwatch 的析构函数。会打印出相关的调试信息。
   */
  ~stopwatch_normal() {
    if (counts <= 0) return;

    printf("===       StopWatch [%lld] <%s>      ===\n", total_index - index,
           name);

    double total = 0.0;
    for (size_t i = 1; i < max_size; ++i) {
      total += data[i].total_time;
    }
    printf("[ total count ]: %lld\n", counts);
    printf("[ average time ]: %f ms (%.1f Hz)\n", total * 1000.0 / counts,
           counts / total);

    double rate = 0.0;
    for (int i = 1; i < max_size; ++i) {
      rate += data[i].total_time / total;
      if (data[i].total_time > 0) {
        printf("step %d - %d: %f ms, %f (acc: %f)\n", i - 1, i,
               data[i].total_time * 1000.0 / counts, data[i].total_time / total,
               rate);
      }
    }
    printf("===       StopWatch [%lld] <%s>      ===\n", total_index - index,
           name);
  }

  /** 计时开始 */
  void start() {
    counts++;
    if (counts < 0) return;
    data[0].now = std::chrono::steady_clock::now();
  }

  /** 标记计时区间 */
  void step(size_t index) {
    if (counts < 0) return;
    data[index].now = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = data[index].now - data[index - 1].now;
    data[index].total_time += diff.count();
  }
};
uint64_t stopwatch_normal::total_index = 0;

using stopwatch = std::conditional_t<config::output_level == 0, stopwatch_dummy,
                                     stopwatch_normal>;
}  // namespace rgm::core
