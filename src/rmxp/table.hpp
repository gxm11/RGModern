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
#include "detail.hpp"

namespace rgm::rmxp {
/**
 * @brief 储存所有 Table 的类，其中的元素都是 std::vector<short>
 * @note table 的处理不同于 bitmap，在 ruby 层额外保存了 &vector::front()，
 * 可以直接拿到数据地址进行快速的操作，参见 get_unsafe 和 set_unsafe。
 * 将 vector 的 &front() 记录在实例变量 @data 中，在使用时直接强制转换该变量
 * 成为 int16_t* 类型（即 &front() 的类型），以跳过查询哈希表。注意，vector
 * 在进入哈希表时是移动拷贝，&front() 不会变化。在调用函数 table_create 和
 * table_resize 时才有可能导致内存位置的改变，而这两个函数会返回新的 &front()。
 */
struct table {
  int x_size;
  int y_size;
  int z_size;
  std::vector<int16_t> data;

  explicit table() : x_size(0), y_size(0), z_size(0), data() {}

  explicit table(int x, int y, int z)
      : x_size(x), y_size(y), z_size(z), data() {
    data.resize(x * y * z, 0);
  }

  int dimension() const {
    if (z_size > 1) return 3;
    if (y_size > 1) return 2;
    return 1;
  }

  int size() const { return x_size * y_size * z_size; }

  void resize(int new_x, int new_y, int new_z) {
    x_size = new_x;
    y_size = new_y;
    z_size = new_z;

    data.resize(new_x * new_y * new_z, 0);
  }

  int16_t get(int x, int y, int z) const {
    int index = x + y * x_size + z * x_size * y_size;
    return data[index];
  }

  int16_t get(int index) const { return data[index]; }

  void set(int x, int y, int z, int16_t value) {
    int index = x + y * x_size + z * x_size * y_size;
    data[index] = value;
  }

  void set(int index, int16_t value) { data[index] = value; }

  int16_t* data_ptr() { return &(data.front()); }
};

struct tables : std::unordered_map<uint64_t, table> {
  explicit tables() : std::unordered_map<uint64_t, table>() {}
};

/**
 * @brief 创建 Table 相关的 ruby 方法
 */
struct init_table {
  using data = rgm::data<tables>;

  static void before(auto& this_worker) {
    static decltype(auto) worker = this_worker;

    struct wrapper {
      static VALUE create(VALUE, VALUE id_, VALUE x_size_, VALUE y_size_,
                          VALUE z_size_) {
        RGMLOAD(id, const uint64_t);
        RGMLOAD(x_size, int);
        RGMLOAD(y_size, int);
        RGMLOAD(z_size, int);

        tables& data = RGMDATA(tables);

        table t(x_size, y_size, z_size);
        VALUE data_ = ULL2NUM(reinterpret_cast<uint64_t>(t.data_ptr()));
        data.emplace(id, std::move(t));
        return data_;
      }

      static VALUE dispose(VALUE, VALUE id_) {
        RGMLOAD(id, const uint64_t);
        tables& data = RGMDATA(tables);

        data.erase(id);
        return Qnil;
      }

      static VALUE get(VALUE, VALUE data_, VALUE index_) {
        RGMLOAD(index, int);

        int16_t* data_ptr = reinterpret_cast<int16_t*>(NUM2ULL(data_));
        return INT2FIX(data_ptr[index]);
      }

      static VALUE set(VALUE, VALUE data_, VALUE index_, VALUE value_) {
        RGMLOAD(index, int);
        RGMLOAD(value, int);

        int16_t* data_ptr = reinterpret_cast<int16_t*>(NUM2ULL(data_));
        data_ptr[index] = value;
        return value_;
      }

      static VALUE resize(VALUE, VALUE id_, VALUE x_size_, VALUE y_size_,
                          VALUE z_size_) {
        RGMLOAD(id, const uint64_t);
        RGMLOAD(x_size, int);
        RGMLOAD(y_size, int);
        RGMLOAD(z_size, int);

        tables& data = RGMDATA(tables);

        table& t = data[id];
        t.resize(x_size, y_size, z_size);
        return ULL2NUM(reinterpret_cast<uint64_t>(t.data_ptr()));
      }

      static VALUE load(VALUE, VALUE id_, VALUE string_) {
        RGMLOAD(id, const uint64_t);

        tables& data = RGMDATA(tables);
        table& t = data[id];

        int16_t* ptr = reinterpret_cast<int16_t*>(RSTRING_PTR(string_));
        int* ptr_int = reinterpret_cast<int*>(RSTRING_PTR(string_));
        // int dimension, xsize, ysize, zsize, size;
        t.resize(ptr_int[1], ptr_int[2], ptr_int[3]);
        memcpy(t.data_ptr(), ptr + 10, t.size() * sizeof(uint16_t));
        return Qnil;
      }

      static VALUE dump(VALUE, VALUE id_) {
        RGMLOAD(id, const uint64_t);

        tables& data = RGMDATA(tables);
        table& t = data[id];
        VALUE str = rb_str_new(reinterpret_cast<const char*>(t.data_ptr()),
                               t.size() * sizeof(uint16_t));
        return str;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_BASE = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_BASE, "table_create", wrapper::create, 4);
    rb_define_module_function(rb_mRGM_BASE, "table_dispose", wrapper::dispose,
                              1);
    rb_define_module_function(rb_mRGM_BASE, "table_load", wrapper::load, 2);
    rb_define_module_function(rb_mRGM_BASE, "table_dump", wrapper::dump, 1);
    rb_define_module_function(rb_mRGM_BASE, "table_get", wrapper::get, 2);
    rb_define_module_function(rb_mRGM_BASE, "table_set", wrapper::set, 3);
    rb_define_module_function(rb_mRGM_BASE, "table_resize", wrapper::resize, 4);
  }

  static void after(auto& worker) { RGMDATA(tables).clear(); }
};
}  // namespace rgm::rmxp
