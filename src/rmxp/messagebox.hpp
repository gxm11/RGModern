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

namespace rgm::rmxp {
struct message_show {
  const char* text;

  void run(auto&) {
    cen::message_box::show(config::game_title, text,
                           cen::message_box_type::information);
  }
};

struct init_message {
  static void before(auto& this_worker) {
    static const decltype(this_worker) worker(this_worker);

    struct wrapper {
      static VALUE show(VALUE, VALUE text_) {
        RGMLOAD(text, const char*);
        worker << message_show{text};
        RGMWAIT(1);

        return Qnil;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_BASE = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_BASE, "message_show", wrapper::show, 1);
  }
};
}  // namespace rgm::rmxp