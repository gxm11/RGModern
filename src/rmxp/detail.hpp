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
// detail 类，继承自 base::detail 类，内部的方法和变量都是静态的。
// 其重载的静态方法 get 能根据枚举类型 word 来获取 ruby 对象的实例变量。
// 显然，只能在 ruby 线程中才能使用 detail 类，安全地访问 ruby 对象的实例变量。

/**
 * @brief detail 类，方便对 ruby 中的实例变量进行操作。
 */
struct detail : base::detail {
  static_assert(std::is_enum_v<word>,
                "Type argument 1 for rgm::base::detail must be Enum.");

  using base::detail::from_ruby;
  using base::detail::id_table;

  explicit detail() : base::detail() {}

  /**
   * @brief 获取 ruby 对象中名称为 w 的实例变量。
   *
   * @tparam w 枚举值，其名称与实例变量的名称相同，如 id
   * @param object 目标 ruby 对象的 VALUE
   * @return VALUE 实例变量对应的 VALUE
   */
  template <word w>
  static VALUE get(VALUE object) {
    Check_Type(object, T_OBJECT);
    return rb_ivar_get(object, id_table[static_cast<size_t>(w)]);
  }

  /**
   * @brief 获取 ruby 对象中名称为 w 的实例变量，并转换成 U 类型
   *
   * @tparam w 枚举值，其名称与实例变量的名称相同，如 id
   * @tparam U 转换后类型，对象必须是此类型在 ruby 中的对应类型
   * @param object 目标 ruby 对象的 VALUE
   * @return U 返回实例变量的值
   */
  template <word w, typename U>
  static U get(VALUE object) {
    return from_ruby<U>(get<w>(object));
  }

  /**
   * @brief 将 ruby 对象转换成 U 类型
   *
   * @tparam U 转换后类型，对象必须是此类型在 ruby 中的对应类型
   * @param object 目标 ruby 对象的 VALUE
   * @return U 返回实例变量的值
   */
  template <typename U>
  static U get(VALUE object) {
    return from_ruby<U>(object);
  }

  /**
   * @brief 初始化 id_table，定义实例变量的名称对应的 symbol
   */
#define DO_DEFINE_IV(key) \
  id_table[static_cast<size_t>(word::key)] = rb_intern("@" #key)

  static void setup() {
    id_table.resize(static_cast<size_t>(word::total) + 1, 0);
    ITERATE_BUILTINS(DO_DEFINE_IV);
    ITERATE_VALUES(DO_DEFINE_IV);
    ITERATE_OBJECTS(DO_DEFINE_IV);
  }

#undef DO_DEFINE_IV
};

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

  static void before(auto&) {
    detail::setup();

    // 定义实例变量的名称对应的常量
    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Word = rb_define_module_under(rb_mRGM, "Word");

    ITERATE_VALUES(DO_DEFINE_CONST);
  }

#undef DO_DEFINE_CONST
};
}  // namespace rgm::rmxp

#define RGMLOAD(name, type) type name = rmxp::detail::get<type>(name##_)
#define RGMLOAD2(name, type, name2) type name = rmxp::detail::get<type>(name2)