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