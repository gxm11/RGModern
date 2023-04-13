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

namespace rgm::core {
/// @brief worker 的合作模式：异步多线程 / 排他单线程 / 协程单线程。
enum class cooperation { asynchronous, exclusive, concurrent };

/// @brief 指示当前 worker 的合作模式为异步多线程模式
/// @tparam index 当前 worker 的索引
/// 异步多线程模式下，调用宏 RGMWAIT(index) 会使当前 worker 进入等待，
/// 直到索引为 index 的 worker 清空其任务队列。
template <size_t index>
struct flag_as {
  static constexpr cooperation co_type = cooperation::asynchronous;
  static constexpr size_t co_index = index;
};

/// @brief 指示当前 worker 的合作模式为排他单线程模式
/// @tparam index 当前 worker 的索引
template <size_t index>
struct flag_ex {
  static constexpr cooperation co_type = cooperation::exclusive;
  static constexpr size_t co_index = index;
};

/// @brief 指示当前 worker 的合作模式为协程单线程模式（尚未实现）
/// @tparam index 当前 worker 的索引
template <size_t index>
struct flag_co {
  static constexpr cooperation co_type = cooperation::concurrent;
  static constexpr size_t co_index = index;
};
}  // namespace rgm::core