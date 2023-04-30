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
#include "base/base.hpp"
#include "word.hpp"

namespace rgm::rmxp {
/**
 * @brief 储存所有 Table 的类，其中的元素都是 std::vector<int16_t>
 * @note table 的处理不同于 bitmap，在 ruby 层额外保存了 &vector::front()，
 * 可以直接拿到数据地址进行快速的操作，参见 get_unsafe 和 set_unsafe。
 * 将 vector 的 &front() 记录在实例变量 @data 中，在使用时直接强制转换该变量
 * 成为 int16_t* 类型（即 &front() 的类型），以跳过查询哈希表。注意，vector
 * 在进入哈希表时是移动拷贝，&front() 不会变化。在调用函数 table_create 和
 * table_resize 时才有可能导致内存位置的改变，而这两个函数会返回新的 &front()。
 */

/// @brief 对应于 RGSS 中 Table 的类，其本质是 std::vector<int16_t>
struct table {
  /// @brief Table 第 1 个维度的大小
  int x_size;

  /// @brief Table 第 2 个维度的大小
  int y_size;

  /// @brief Table 第 3 个维度的大小
  int z_size;

  /// @brief 存储 Table 数据的容器
  std::vector<int16_t> m_data;

  /// @brief 构造函数
  explicit table() : x_size(0), y_size(0), z_size(0), m_data() {}

  /// @brief Table 的维度
  /// @return 返回值可能是 1，2 或者 3
  int dimension() const {
    if (z_size > 1) return 3;
    if (y_size > 1) return 2;
    return 1;
  }

  /// @brief Table 总大小
  /// @return 返回三个维度大小之积
  size_t size() const { return x_size * y_size * z_size; }

  /// @brief 重设 Table 的三个维度
  /// @param new_x 改变后的第 1 个维度大小
  /// @param new_y 改变后的第 2 个维度大小
  /// @param new_z 改变后的第 3 个维度大小
  void resize(int new_x, int new_y, int new_z) {
    x_size = new_x;
    y_size = new_y;
    z_size = new_z;

    /* 总数变小截取，不足补 0 */
    m_data.resize(new_x * new_y * new_z, 0);
    m_data.shrink_to_fit();
  }

  /// @brief 获取 table 中特定位置的值
  /// @param x 第 1 个维度的坐标
  /// @param y 第 2 个维度的坐标
  /// @param z 第 3 个维度的坐标
  /// @return 储存的值
  int16_t get(int x, int y, int z) const {
    int index = x + y * x_size + z * x_size * y_size;

    return get(index);
  }

  /// @brief 获取 table 中特定位置的值
  /// @param 将 table 展开成 1 列，对应的位置
  /// @return 储存的值
  int16_t get(int index) const {
    /* 访问 vector 时总是检查索引的范围 */
    return m_data.at(index);
  }

  /// @brief 修改 table 中特定位置的值
  /// @param x 第 1 个维度的坐标
  /// @param y 第 2 个维度的坐标
  /// @param z 第 3 个维度的坐标
  /// @param value 修改后的值
  void set(int x, int y, int z, int16_t value) {
    int index = x + y * x_size + z * x_size * y_size;

    set(index, value);
  }

  /// @brief 修改 table 中特定位置的值
  /// @param 将 table 展开成 1 列，对应的位置
  void set(int index, int16_t value) {
    /* 访问 vector 时总是检查索引的范围 */
    m_data.at(index) = value;
  }

  /// @brief 返回 table 在堆上的数据指针
  /// @return 返回堆上的指针
  int16_t* data_ptr() { return &(m_data.front()); }
};

/// @brief 存储所有 table，即 Table 对象的类
/// @name data
using tables = std::unordered_map<uint64_t, table>;

/**
 * @brief 创建 Table 相关的 ruby 方法
 */
struct init_table {
  using data = std::tuple<tables>;

  static void before(auto& this_worker) {
    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /* ruby method: Base#table_create -> tables::emplace */
      static VALUE create(VALUE, VALUE id_, VALUE x_size_, VALUE y_size_,
                          VALUE z_size_) {
        RGMLOAD(id, uint64_t);
        RGMLOAD(x_size, int);
        RGMLOAD(y_size, int);
        RGMLOAD(z_size, int);

        tables& data = RGMDATA(tables);

        table t;

        t.resize(x_size, y_size, z_size);
        int16_t* data_ptr = t.data_ptr();
        VALUE data_ptr_ = ULL2NUM(reinterpret_cast<uint64_t>(data_ptr));

        data.emplace(id, std::move(t));

        /* 返回堆上指针的值，或者 nil */
        return data_ptr ? data_ptr_ : Qnil;
      }

      /* ruby method: Base#table_dispose -> tables::erase */
      static VALUE dispose(VALUE, VALUE id_) {
        RGMLOAD(id, uint64_t);
        tables& data = RGMDATA(tables);

        data.erase(id);
        return Qnil;
      }

      /* ruby method: Base#table_get -> table::opaerator[] */
      static VALUE get(VALUE, VALUE data_ptr_, VALUE index_) {
        RGMLOAD(index, int);
        RGMLOAD(data_ptr, int16_t*);

        return INT2FIX(data_ptr[index]);
      }

      /* ruby method: Base#table_set -> table::opaerator[] */
      static VALUE set(VALUE, VALUE data_ptr_, VALUE index_, VALUE value_) {
        RGMLOAD(index, int);
        RGMLOAD(value, int);
        RGMLOAD(data_ptr, int16_t*);

        data_ptr[index] = value;
        return value_;
      }

      /* ruby method: Base#table_resize -> table::resize */
      static VALUE resize(VALUE, VALUE id_, VALUE x_size_, VALUE y_size_,
                          VALUE z_size_) {
        RGMLOAD(id, uint64_t);
        RGMLOAD(x_size, int);
        RGMLOAD(y_size, int);
        RGMLOAD(z_size, int);

        tables& data = RGMDATA(tables);

        table& t = data.at(id);
        t.resize(x_size, y_size, z_size);

        /* 返回堆上指针的值，或者 nil */
        int16_t* data_ptr = t.data_ptr();
        if (!data_ptr) return Qnil;

        return ULL2NUM(reinterpret_cast<uint64_t>(t.data_ptr()));
      }

      /*
       * table 在 RGSS 中存储的格式如下：
       * 首先存 5 个 int，代表：dimension, xsize, ysize, zsize, size
       * 然后存 int16_t 的堆上数据。
       * 这里的 load 和 dump 都必须按照此格式来读写以保证兼容性。
       */

      /* ruby method: Base#table_load -> table::table() */
      static VALUE load(VALUE, VALUE id_, VALUE string_) {
        RGMLOAD(id, uint64_t);

        tables& data = RGMDATA(tables);
        table& t = data.at(id);

        int16_t* ptr = reinterpret_cast<int16_t*>(RSTRING_PTR(string_));
        int* ptr_int = reinterpret_cast<int*>(RSTRING_PTR(string_));
        t.resize(ptr_int[1], ptr_int[2], ptr_int[3]);
        memcpy(t.data_ptr(), ptr + 10, t.size() * sizeof(int16_t));
        return Qnil;
      }

      /* ruby method: Base#table_dump -> String */
      static VALUE dump(VALUE, VALUE id_) {
        RGMLOAD(id, uint64_t);

        tables& data = RGMDATA(tables);
        table& t = data.at(id);
        VALUE str = rb_str_new(reinterpret_cast<const char*>(t.data_ptr()),
                               t.size() * sizeof(int16_t));
        return str;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "table_create", wrapper::create, 4);
    rb_define_module_function(rb_mRGM_Base, "table_dispose", wrapper::dispose,
                              1);
    rb_define_module_function(rb_mRGM_Base, "table_load", wrapper::load, 2);
    rb_define_module_function(rb_mRGM_Base, "table_dump", wrapper::dump, 1);
    rb_define_module_function(rb_mRGM_Base, "table_get", wrapper::get, 2);
    rb_define_module_function(rb_mRGM_Base, "table_set", wrapper::set, 3);
    rb_define_module_function(rb_mRGM_Base, "table_resize", wrapper::resize, 4);
  }

  static void after(auto& worker) { RGMDATA(tables).clear(); }
};
}  // namespace rgm::rmxp
