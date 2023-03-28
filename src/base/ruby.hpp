// zlib License

// copyright (C) 2023 Xiaomi Guo and Krimiston

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
// 移除在 ruby 源码中的 unused-paramerter 警告
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif  // __GNUC__
#include "ruby.h"
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif  // __GNUC__

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
