// zlib License
//
// copyright (C) 2023 Guoxiaomi and Krimiston
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once
#include "core/core.hpp"

namespace rgm::base {
/// @brief ruby 相关的初始化类，初始化 ruby 的运行环境
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

/// @brief Config 相关的初始化类，定义了 RGM::Config 下的常量
/// 这些常量对应 rgm::config 中的变量，部分读取自 config.ini 文件
struct init_config {
  static void before(auto&) {
    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Config = rb_define_module_under(rb_mRGM, "Config");
    rb_const_set(rb_mRGM_Config, rb_intern("Config_Path"),
                 rb_utf8_str_new_cstr(config::config_path.data()));
    rb_const_set(rb_mRGM_Config, rb_intern("Build_Mode"),
                 INT2FIX(config::build_mode));
    rb_const_set(rb_mRGM_Config, rb_intern("Controller_Axis_Threshold"),
                 INT2FIX(config::controller_axis_threshold));
    rb_const_set(rb_mRGM_Config, rb_intern("Max_Workers"),
                 INT2FIX(config::max_workers));
    rb_const_set(rb_mRGM_Config, rb_intern("Tileset_Texture_Height"),
                 INT2FIX(config::tileset_texture_height));
    rb_const_set(rb_mRGM_Config, rb_intern("Battle_Test"),
                 config::btest ? Qtrue : Qfalse);
    rb_const_set(rb_mRGM_Config, rb_intern("Debug"),
                 config::debug ? Qtrue : Qfalse);
    rb_const_set(rb_mRGM_Config, rb_intern("Synchronized"),
                 config::synchronized ? Qtrue : Qfalse);
    rb_const_set(rb_mRGM_Config, rb_intern("Game_Console"),
                 config::game_console ? Qtrue : Qfalse);
    rb_const_set(rb_mRGM_Config, rb_intern("Game_Title"),
                 rb_utf8_str_new_cstr(config::game_title.data()));
    rb_const_set(rb_mRGM_Config, rb_intern("Resource_Prefix"),
                 rb_utf8_str_new_cstr(config::resource_prefix.data()));
    rb_const_set(rb_mRGM_Config, rb_intern("Window_Width"),
                 INT2FIX(config::window_width));
    rb_const_set(rb_mRGM_Config, rb_intern("Window_Height"),
                 INT2FIX(config::window_height));
    rb_const_set(rb_mRGM_Config, rb_intern("Screen_Width"),
                 INT2FIX(config::screen_width));
    rb_const_set(rb_mRGM_Config, rb_intern("Screen_Height"),
                 INT2FIX(config::screen_height));
    rb_const_set(rb_mRGM_Config, rb_intern("Render_Driver"),
                 INT2FIX(static_cast<int>(config::driver)));
    rb_const_set(rb_mRGM_Config, rb_intern("Render_Driver_Name"),
                 rb_utf8_str_new_cstr(config::driver_name.data()));
  }
};
}  // namespace rgm::base