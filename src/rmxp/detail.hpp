// zlib License

// Copyright (C) [2023] [Xiaomi Guo]

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
#include "word.hpp"

namespace rgm::rmxp {
using detail = base::detail_ext<word>;

/**
 * @brief 建立实例变量对应的 symbol，定义相关常量
 */
struct init_detail {
  /**
   * @brief 将字符串中的小写字母转换成大写字母
   *
   * @param str 目标字符串
   * @return std::array<char, 32> 输出std::array
   */

#define DO_DEFINE_CONST(key)                               \
  rb_const_set(rb_mRGM_Word, rb_intern("Attribute_" #key), \
               INT2FIX(static_cast<int>(word::key)))
#define DO_DEFINE_IV(key) \
  id_table[static_cast<size_t>(word::key)] = rb_intern("@" #key)

  /**
   * @brief 初始化 id_table，定义实例变量的名称对应的 symbol
   */
  static void setup() {
    auto& id_table = detail::id_table;
    id_table.resize(static_cast<size_t>(word::total) + 1, 0);
    ITERATE_BUILTINS(DO_DEFINE_IV);
    ITERATE_VALUES(DO_DEFINE_IV);
    ITERATE_OBJECTS(DO_DEFINE_IV);
  }

  static void before(auto&) {
    setup();
    // 定义实例变量的名称对应的常量
    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Word = rb_define_module_under(rb_mRGM, "Word");

    ITERATE_VALUES(DO_DEFINE_CONST);
  }

#undef DO_DEFINE_IV
#undef DO_DEFINE_CONST
};
}  // namespace rgm::rmxp
