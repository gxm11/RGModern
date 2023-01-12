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
/** @brief 利用 RAII 机制管理 ruby 初始化和退出的类 */
struct ruby_library {
  /** @brief ruby 的状态，0 表示无错误 */
  int ruby_state;

  /** @brief 初始化 ruby 运行环境 */
  explicit ruby_library() : ruby_state(0) {
    int argc = 0;
    char* argv = nullptr;
    char** pArgv = &argv;

    ruby_sysinit(&argc, &pArgv);
    RUBY_INIT_STACK;
    ruby_init();
    ruby_init_loadpath();
    rb_call_builtin_inits();
  }

  /** @brief 清理 ruby 运行环境 */
  ~ruby_library() {
    if (ruby_state) {
      VALUE rbError = rb_funcall(rb_gv_get("$!"), rb_intern("message"), 0);

      std::ofstream log;
      log.open("./error.log");
      log << rb_string_value_ptr(&rbError);
      log.close();
    };
    ruby_cleanup(ruby_state);
  }
};

/** @brief 将 ruby_library 类型的变量添加到 worker 的 datalist 中 */
struct init_ruby {
  using data = rgm::data<ruby_library>;
};
}  // namespace rgm::base