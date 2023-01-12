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
#include <vector>

#include "core/core.hpp"
#include "ruby.hpp"

namespace rgm::base {
/**
 * @brief 管理 ruby 的 VALUE 和 C++ 内各数值类型的转换
 * @note 内含全局变量 id_table 用于缓存 ruby 中 Symbol 的 ID 以提升查找效率。
 */
struct detail {
  /** 静态成员变量 id_table，缓存 ruby 中 Symbol 的 ID 提升查找效率。*/
  static std::vector<ID> id_table;

  /**
   * @brief 将 ruby 对象（VALUE 类型）转换成 T 类型。
   *
   * @tparam T 目标类型
   * @param value_ ruby 对象的 VALUE
   * @return T 返回 T 类型的变量，T必须是数值或布尔值。
   */
  template <typename T>
  static T from_ruby(const VALUE);

  /**
   * @brief 一般整型变量的特化处理，包括 int / short / uint8_t 等。
   *
   * @tparam T 目标类型
   * @param value_ ruby 对象的 VALUE
   * @return T 返回 T 类型的变量，T必须是整型数值。
   */
  template <std::integral T>
  static std::remove_const_t<T> from_ruby(const VALUE value_) {
    Check_Type(value_, T_FIXNUM);
    return FIX2INT(value_);
  }
};
/** 全局变量 id_table */
std::vector<ID> detail::id_table = {};

/**
 * @brief uint64_t 类型变量的特化处理。该变量代表 ruby 层的对象。
 * @note 如果对象为 nil，返回 0，否则返回对象的 object_id。
 * 目前只有 Bitmap 和 Table 类的对象需要用此方法获取 id。
 * 虽然 drawable 的 z_index::id 的类型也是 uint64_t，
 * 但是 z_index 是“隐藏”的成员，它不需要通过实例变量访问。
 * @param value_ ruby 对象的 VALUE
 * @return uint64_t 返回 0 或者对象 object_id 的值。
 */
template <>
uint64_t detail::from_ruby<uint64_t>(const VALUE value_) {
  if (value_ != Qnil) [[likely]] {
    return NUM2ULL(rb_obj_id(value_));
  } else [[unlikely]] {
    return 0;
  }
}

template <>
uint64_t detail::from_ruby<const uint64_t>(const VALUE value_) {
  return NUM2ULL(value_);
}

/**
 * @brief 浮点数类型变量的特化处理。
 * @param value_ ruby 对象的 VALUE
 * @return double 返回该 VALUE 所包含的浮点数值
 */
template <>
double detail::from_ruby<double>(const VALUE value_) {
  switch (TYPE(value_)) {
      // 注意 FIXNUM 也是合法的浮点数。
    case T_FIXNUM:
      break;
    case T_FLOAT:
      break;
    default:
      rb_unexpected_type(value_, T_FLOAT);
      break;
  }
  return NUM2DBL(value_);
}

/**
 * @brief bool 类型变量的特化处理。
 * @param value_ ruby 对象的 VALUE
 */
template <>
bool detail::from_ruby<bool>(const VALUE value_) {
  return value_ == Qtrue;
}

/**
 * @brief const char* 类型变量的特化处理。
 * @param value_ ruby 对象的 VALUE
 */
template <>
const char* detail::from_ruby<const char*>(const VALUE value_) {
  Check_Type(value_, T_STRING);
  return RSTRING_PTR(value_);
}
}  // namespace rgm::base
