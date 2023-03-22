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