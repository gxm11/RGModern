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
// 移除在 ruby 源码中的 unused-paramerter 警告
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // __GNUC__
#include "ruby.h"
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif // __GNUC__

// 宏 access 与 centurion 库冲突
#undef access
// 函数名 bind 与 ruby 头文件冲突
#undef bind
// 函数名 send 与 ruby 头文件冲突
#undef send

/** 加密包模式下不检查 ruby 的变量类型 */
#if RGM_BUILDMODE >= 3
#undef Check_Type
#define Check_Type(v, t) static_assert(true)
#endif
