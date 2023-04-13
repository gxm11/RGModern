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
#include "config.hpp"

namespace rgm::core {
/// @brief 秒表的基类，提供了接口函数，但不执行任何操作
/// 在非开发模式下使用此类代替 stopwatch 以提升性能
struct stopwatch_base {
  /// @brief 秒表的构造函数
  /// @param name 秒表的名称，打印结果时显示
  /// @param skip_counts 正式计时前跳过的步数
  explicit stopwatch_base(const char*, int64_t = 0) {}

  /// @brief 秒表计时的起点
  /// 秒表每次调用 start 都会重新开始计时
  void start() {}

  /// @brief 秒表计时的阶段记录点
  /// @param index 表示此处为第 index 个阶段记录点
  void step(size_t) {}
};

/// @brief 秒表，用于测试若干段反复执行代码的性能。
/// @name stopwatch
/// 继承自 stopwatch_base 作为全局变量使用。
/// 借助 RAII 机制，在程序退出时析构并打印相关的调试信息。
struct stopwatch_normal {
  /// @brief 代表一个阶段性记录点的内部类
  struct interval {
    /// @brief 当前记录的时间，与下一个阶段记录的时间做减法来计时
    /// 最后一个阶段记录的时间，要与下一个 start 的时间做减法来计时
    std::chrono::time_point<std::chrono::steady_clock> now;

    /// @brief 此阶段总共耗费的时间
    double total_time = 0.0;
  };

  /// @brief 阶段记录点的最多数量为 16
  static constexpr int max_size = 16;

  /// @brief 秒表的索引，每次创建新的秒表都会 +1
  inline static uint64_t total_index = 0;

  /// @brief interval 的数组，内部是计时的数据
  std::array<interval, max_size> data;

  /// @brief 秒表的名称
  std::string name;

  /// @brief 秒表的索引
  uint64_t index;

  /// @brief 秒表总共记录的轮数，每次执行 start 函数此值都会 +1
  int64_t counts;

  /// @brief stopwatch_normal 的构造函数
  /// @param name 打印结果时显示的名称
  /// @param skip_counts 正式计时前跳过的步数，默认为 1
  explicit stopwatch_normal(const char* name, int skip_counts = 1) : data() {
    this->name = name;
    this->index = total_index++;
    this->counts = 0 - skip_counts;
  }

  /// @brief stopwatch_normal 的析构函数。析构时会打印出相关的调试信息。
  ~stopwatch_normal() {
    if (counts <= 0) return;

    printf("===       StopWatch [%lld] <%s>      ===\n", total_index - index,
           name.data());

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
           name.data());
  }

  /// @brief 标记计时的起点
  void start() {
    counts++;
    if (counts < 0) return;
    data[0].now = std::chrono::steady_clock::now();
  }

  /// @brief 标记计时的阶段记录点
  void step(size_t index) {
    if (counts < 0) return;
    data[index].now = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = data[index].now - data[index - 1].now;
    data[index].total_time += diff.count();
  }
};

/// @brief 根据 config::develop 选择不同的 stopwatch 实现
using stopwatch =
    std::conditional_t<config::develop, stopwatch_normal, stopwatch_base>;
}  // namespace rgm::core
