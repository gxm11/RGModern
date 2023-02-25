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
#include "datalist.hpp"
#include "kernel.hpp"
#include "scheduler.hpp"
#include "semaphore.hpp"
#include "stopwatch.hpp"
#include "tasklist.hpp"
#include "type_traits.hpp"
#include "worker.hpp"

namespace rgm {
/**
 * @brief traits::TypeList 的简写
 *
 * @tparam Args... 可变参数列表，通常是不同的数据类型
 */
template <typename... Args>
using data = core::traits::TypeList<Args...>;
}  // namespace rgm

/** 宏 RGMDATA 调用 worker 的 get 方法，以取出特定类型的数据 */
#define RGMDATA(type) worker.template get<type>()

/** 宏 RGMWAIT 调用 worker 的 wait 方法，等待指定线程的同步信号 */
#define RGMWAIT(id) worker.template wait<id>()

/** 宏 RGMENGINE 特化 traits::magic_cast，以将基类指针 scheduler<>* 转型成派生类
 * T* */
#define RGMENGINE(T)                                           \
  template <>                                                     \
  struct core::traits::magic_cast<core::scheduler<T::co_type>*> { \
    using type = T*;                                              \
  }
