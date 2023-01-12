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
#include "core/core.hpp"
#include "init_ruby.hpp"

namespace rgm::base {
/**
 * @brief 主动模型的 ruby kernel，其 run 函数执行 main.rb。
 * @note main.rb 在编译时已经添加到程序里，不可更改。
 * @tparam T_tasks 可以执行的任务列表
 */
template <typename T_tasks>
struct kernel_ruby : rgm::core::kernel_active<T_tasks> {
  void run([[maybe_unused]] auto& worker) {
#ifdef RGM_EMBEDED_ZIP
    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_funcall(rb_mRGM_Base, rb_intern("load_script"), 1,
               rb_str_new_cstr("script/load.rb"));
#else
    int* p_state = &RGMDATA(ruby_library).ruby_state;
    rb_ary_push(rb_gv_get("$LOAD_PATH"), rb_str_new_cstr("./src/script"));
    rb_load_protect(rb_str_new_cstr("load.rb"), 0, p_state);
#endif
  }
};
}  // namespace rgm::base