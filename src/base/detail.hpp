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

namespace rgm::base {
/// @brief 管理 ruby 的 VALUE 和 C++ 内各数值类型的转换的类
/// @name meta
/// 部分数据会在 ruby 和 C++ 层各存储一份，只在特定时机同步。
/// detail 类负责这些 ruby 和 C++ 中类型匹配的数据的自动转换。
struct detail {
  /* 静态成员变量 id_table，缓存 ruby 中各 Symbol 的 ID 提升查找效率。*/
  inline static std::vector<ID> id_table = {};

  /// @brief from_ruby 的模板声明，将 ruby 中的 VALUE 转换成 T 类型
  /// @tparam T C++ 层的数据类型
  /// @param VALUE 类型的变量表示 ruby 中的 object 或即时值
  /// @return 转换后 T 类型的数据
  template <typename T>
  static T from_ruby(const VALUE);

  /// @brief from_ruby 对一般整型变量的特化处理
  /// @tparam T 目标类型，受到 std::integral 约束
  /// @param value_ 关联 ruby 对象的 VALUE
  /// @return 转换后 T 类型的数据
  /// T_FIXNUM 限制了值的范围必须在 -0x4000_0000 ~ 0x3fff_ffff 之间
  /// 所以这里只包括部分 int32_t 和 short / uint8_t 等短类型。
  template <std::integral T>
  static std::remove_const_t<T> from_ruby(const VALUE value_) {
    Check_Type(value_, T_FIXNUM);
    return FIX2INT(value_);
  }

  /// @brief detail 类被调用时的接口函数，被子类继承并重载不同模板类型的版本
  /// @tparam T 返回值类型
  /// @param object 关联 ruby 对象的 VALUE
  /// @return 转换后 T 类型的数据
  template <typename T>
  static T get(VALUE object) {
    return from_ruby<T>(object);
  }
};

/// @brief from_ruby 对 const uint64_t 类型变量的特化处理。
/// @return uint64_t 返回 0 或者 ruby 对象的 object_id 的值。
/// @see src/rmxp/drawable.hpp
/// 对于 ruby 中的类，其属性如果是 object 而不是数值，在 C++ 层中匹配的成员
/// 变量是 const uint64_t 类型，储存该属性的 object_id。
/// 如果对象不为 nil，此特化形式获取 ruby 对象的 object_id 并返回。
/// 如果对象为 nil，返回 0 而不是 Qnil（=4）。
template <>
uint64_t detail::from_ruby<const uint64_t>(const VALUE value_) {
  /* 多数情况下这个对象不会是 nil */
  if (value_ != Qnil) [[likely]] {
    return NUM2ULL(rb_obj_id(value_));
  } else [[unlikely]] {
    return 0;
  }
}

/**
 * @brief uint64_t 类型变量的特化处理。
 * @note 该类型是单纯的整数转 uint64_t，与上面的 const uint64_t 做一个区分处理。
 * 通常用来获取传入的 object id，或者其他无符号大整数。
 */
template <>
uint64_t detail::from_ruby<uint64_t>(const VALUE value_) {
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

/**
 * @brief std::string 类型变量的特化处理。
 * @param value_ ruby 对象的 VALUE
 */
template <>
std::string detail::from_ruby<std::string>(const VALUE value_) {
  Check_Type(value_, T_STRING);
  return std::string{RSTRING_PTR(value_)};
}

/**
 * @brief std::string_view 类型变量的特化处理。
 * @param value_ ruby 对象的 VALUE
 */
template <>
std::string_view detail::from_ruby<std::string_view>(const VALUE value_) {
  Check_Type(value_, T_STRING);
  return std::string_view{RSTRING_PTR(value_),
                          static_cast<size_t>(RSTRING_LEN(value_))};
}

// detail 类，继承自 base::detail 类，内部的方法和变量都是静态的。
// 其重载的静态方法 get 能根据枚举类型 word 来获取 ruby 对象的实例变量。
// 显然，只能在 ruby 线程中才能使用 detail 类，安全地访问 ruby 对象的实例变量。

/**
 * @brief detail 类，方便对 ruby 中的实例变量进行操作。
 */
template <typename word>
struct detail_ext : detail {
  static_assert(std::is_enum_v<word>,
                "Type argument 1 for rgm::base::detail must be Enum.");

  using detail::from_ruby;
  using detail::id_table;

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
    if constexpr (std::same_as<U, uint64_t>) {
      return from_ruby<const uint64_t>(get<w>(object));
    } else {
      return from_ruby<U>(get<w>(object));
    }
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
};
}  // namespace rgm::base
#define RGMLOAD(name, type) type name = detail::get<type>(name##_)
#define RGMLOAD2(name, type, name2) type name = detail::get<type>(name2)