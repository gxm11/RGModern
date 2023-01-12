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
#include "detail.hpp"
#include "ruby.hpp"
#include "timer.hpp"

namespace rgm::base {
// It seems that the declaration must be inside the namespace.
extern "C" VALUE rb_eInterrupt;

/** @brief 任务：使 ruby 线程抛出 Interrupt 异常 */
struct interrupt_signal {
  void run(auto&) {
    rb_raise(rb_eInterrupt, "Interrupted by another thread.\n");
  }
};

/** @brief 创建 synchronize 相关的 ruby 方法 */
struct init_synchronize {
  using data = rgm::data<timer>;

  static void before(auto& this_worker) {
    static const decltype(this_worker) worker(this_worker);

    /** wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /** RGM::Base.synchronize 方法 */
      static VALUE synchronize(VALUE, VALUE thread_id_) {
        Check_Type(thread_id_, T_FIXNUM);
        int thread_id = FIX2INT(thread_id_);

        if (thread_id & 0b0001) RGMWAIT(1);
        if (thread_id & 0b0010) RGMWAIT(2);
        if (thread_id & 0b0100) RGMWAIT(3);
        if (thread_id & 0b1000) RGMWAIT(4);

        RGMDATA(timer).reset();
        return Qnil;
      }

      static VALUE check_delay(VALUE, VALUE frame_rate_) {
        double freq = 1.0 / detail::from_ruby<double>(frame_rate_);
        RGMDATA(timer).tick(freq);
        return Qnil;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "synchronize", wrapper::synchronize,
                              1);
    rb_define_module_function(rb_mRGM_Base, "check_delay", wrapper::check_delay,
                              1);
    RGMDATA(timer).reset();
  }
};
}  // namespace rgm::base