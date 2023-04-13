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
#include "base/base.hpp"
#include "ruby_wrapper.hpp"

namespace rgm::ext {
struct ping {
  int hint;

  void run(auto&, std::string& out) { out = "pong: " + std::to_string(hint); }
};

struct init_ping {
  static void before(auto& this_worker) {
    static decltype(auto) worker = this_worker;
    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Ext = rb_define_module_under(rb_mRGM, "Ext");

    RGMBIND2(rb_mRGM_Ext, "async_ping", ping, 1);
  }
};
}  // namespace rgm::ext