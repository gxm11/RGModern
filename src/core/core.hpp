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
#include "cooperation.hpp"
#include "kernel.hpp"
#include "scheduler.hpp"
#include "semaphore.hpp"
#include "stopwatch.hpp"
#include "type_traits.hpp"
#include "worker.hpp"

/// @brief 宏 RGMDATA 调用 worker 的 get 方法，以取出特定类型的数据
#define RGMDATA(type) worker.template get<type>()

/// @brief 宏 RGMWAIT 调用 worker 的 wait 方法，等待指定线程的同步信号
#define RGMWAIT(id) worker.template wait<id>()

/// @brief 宏 RGMENGINE 是 scheduler_cast 的特化处理，
/// 将基类指针 scheduler<>* 转型成派生类 T*，对应 scheduler<T_workers...>
#define RGMENGINE(T)                             \
  template <>                                    \
  struct rgm::core::scheduler_cast<T::co_type> { \
    using type = T*;                             \
  }
