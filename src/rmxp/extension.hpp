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
                 rb_utf8_str_new_cstr(config::game_title.data()));
    rb_const_set(rb_mRGM, rb_intern("Default_Config"),
                 rb_utf8_str_new_cstr(config::config_path));
    rb_const_set(rb_mRGM, rb_intern("Resource_Prefix"),
                 rb_utf8_str_new_cstr(config::resource_prefix.data()));
    rb_const_set(rb_mRGM, rb_intern("BuildMode"), INT2FIX(config::build_mode));
  }
};
}  // namespace rgm::rmxp