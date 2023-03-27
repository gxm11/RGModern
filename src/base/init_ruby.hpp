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
#include <fstream>
#include <iostream>

#include "core/core.hpp"
#include "ruby.hpp"

extern "C" {
void rb_call_builtin_inits();
}

namespace rgm::base {
/** @brief 将 ruby_library 类型的变量添加到 worker 的 datalist 中 */
struct init_ruby {
  static void before(auto&) {
    int argc = 0;
    char* argv = nullptr;
    char** pArgv = &argv;

    ruby_sysinit(&argc, &pArgv);
    RUBY_INIT_STACK;
    ruby_init();
    ruby_init_loadpath();
    rb_call_builtin_inits();
  }
};
}  // namespace rgm::base