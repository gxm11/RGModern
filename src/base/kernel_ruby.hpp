// zlib License

// copyright (C) 2023 Guoxiaomi and Krimiston

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
#include <iostream>  // cerr

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
  /// @param[in]  data  rb_rescue2 传入参数，未使用。
  /// @return     任意，rb_rescue2 会返回此值。
  static VALUE script_run(VALUE) {
#ifdef RGM_EMBEDED_ZIP
    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_funcall(rb_mRGM_Base, rb_intern("load_script"), 1,
               rb_str_new_literal("script/load.rb"));
#else
    rb_ary_push(rb_gv_get("$LOAD_PATH"), rb_str_new_cstr("./src/script"));
    rb_load(rb_str_new_literal("load.rb"), 0);
#endif
    return Qnil;
  }

  /// @param[in]  data  rb_rescue2 传入参数，未使用。
  /// @param[in]  exc   rb_rescue2 捕获的 ruby 异常。
  /// @return     任意，rb_rescue2 会返回此值。
  static VALUE script_rescue(VALUE, VALUE exc) {
    VALUE backtrace = rb_ary_to_ary(rb_funcall(exc, rb_intern("backtrace"), 0));
    VALUE message = rb_str_to_str(rb_funcall(exc, rb_intern("message"), 0));
    std::cerr << StringValueCStr(message) << std::endl;
    for (long i = 0; i < RARRAY_LEN(backtrace); ++i) {
      VALUE lp = rb_str_to_str(rb_ary_entry(backtrace, i));
      std::cerr << StringValueCStr(lp) << std::endl;
    }
    return Qnil;
  }

  void run(auto&) {
    // rb_rescue2 的说明参见 include/ruby/backward/cxxanyargs.hpp
    rb_rescue2(script_run, Qnil, script_rescue, Qnil, rb_eException,
               static_cast<VALUE>(0));
    ruby_finalize();
  }
};
}  // namespace rgm::base
