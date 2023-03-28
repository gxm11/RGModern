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
#include <cstdint>

#include "core/core.hpp"
#include "ruby.hpp"

namespace rgm::base {
/** @brief 计数器，用于生成递增的 ID。*/
struct counter {
  uint64_t fetch_and_add() {
    static std::atomic<uint64_t> id = 1000;
    return id.fetch_add(10);
  }
};

/** @brief 将 counter 类型的变量添加到 worker 的 datalist 中 */
struct init_counter {
  using data = std::tuple<counter>;

  static void before(auto& this_worker) {
    static decltype(auto) worker = this_worker;

    /** wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /** RGM::Base.new_id 方法 */
      static VALUE new_id(VALUE) {
        return ULL2NUM(RGMDATA(counter).fetch_and_add());
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_BASE = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_BASE, "new_id", wrapper::new_id, 0);
  }
};
}  // namespace rgm::base