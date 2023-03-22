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
