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
#include "base/base.hpp"

extern "C" {
void Init_fiddle();
void Init_zlib();
}

namespace rgm::rmxp {
/**
 * @brief 引入 ruby 的扩展功能
 * @note 目前的扩展有
 * 1. Fiddle 库
 * 2. RGM::BuildMode 常量
 */
struct init_extension {
  static void before(auto&) {
    Init_fiddle();
    Init_zlib();
    VALUE rb_mRGM = rb_define_module("RGM");

    rb_gv_set("$DEBUG", config::debug ? Qtrue : Qfalse);
    rb_gv_set("$BTEST", config::btest ? Qtrue : Qfalse);

    rb_const_set(rb_mRGM, rb_intern("Default_Title"),
                 rb_utf8_str_new_cstr(config::game_title));
    rb_const_set(rb_mRGM, rb_intern("Default_Config"),
                 rb_utf8_str_new_cstr(config::game_config));
    rb_const_set(rb_mRGM, rb_intern("Resource_Prefix"),
                 rb_utf8_str_new_cstr(config::resource_prefix));
    rb_const_set(rb_mRGM, rb_intern("BuildMode"), INT2FIX(config::build_mode));
  }
};
}  // namespace rgm::rmxp