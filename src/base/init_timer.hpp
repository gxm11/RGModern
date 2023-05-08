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
#include "detail.hpp"
#include "timer.hpp"

namespace rgm::base {
/// @brief 数据类 timer 相关的初始化类
/// @see src/base/timer.hpp
struct init_timer {
  using data = std::tuple<timer>;

  static void before(auto& this_worker) {
    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /** ruby method: Base#synchronize -> RGMWAIT */
      static VALUE synchronize(VALUE, VALUE worker_id_) {
        RGMLOAD(worker_id, int);

        if (worker_id > config::max_workers) {
          rb_raise(rb_eArgError,
                   "There're too many threads, please change "
                   "config::max_workers.\n");
        }

        switch (worker_id) {
          default:
            break;
          case 1:
            RGMWAIT(1);
            break;
          case 2:
            RGMWAIT(2);
            break;
          case 3:
            RGMWAIT(3);
            break;
          case 4:
            RGMWAIT(4);
            break;
          case 5:
            RGMWAIT(5);
            break;
          case 6:
            RGMWAIT(6);
            break;
          case 7:
            RGMWAIT(7);
            break;
          case 8:
            RGMWAIT(8);
            break;
        }

        RGMDATA(timer).reset();
        return Qnil;
      }

      /* ruby method: Base#check_delay -> timer::tick */
      static VALUE check_delay(VALUE, VALUE frame_rate_) {
        RGMLOAD(frame_rate, double);

        double freq = 1 / frame_rate;
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

    /* 在此处重置 timer */
    RGMDATA(timer).reset();
  }
};
}  // namespace rgm::base