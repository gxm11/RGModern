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
// 这是一个全局变量，存储了所有字体的路径。
// 只有标记为 owner 的线程才能读写，其他线程只能读。
std::array<std::string, 32> font_paths = {};

template <bool owner>
struct font_manager {
  // 存储所有字体的 map 容器，{字体ID, 字号} => font对象
  // 字体的数量相对会少很多，这里使用 map 来管理，较 hash 更好。
  std::map<std::pair<int, int>, cen::font> data;

  explicit font_manager() : data() {}

  int insert(const char* path)
    requires(owner)
  {
    int id;
    for (id = 0; id < static_cast<int>(font_paths.size()); ++id) {
      if (font_paths[id] == path) {
        break;
      }
      if (font_paths[id] == "") {
        font_paths[id] = path;
        break;
      }
    }
    if (id == static_cast<int>(font_paths.size())) {
      throw std::invalid_argument(
          "Number of different fonts must less than 32!");
    }

    return id;
  }

  cen::font& get(int id, int font_size) {
    auto it = data.find({id, font_size});
    if (it != data.end()) {
      return it->second;
    }
    data.emplace(std::pair{id, font_size},
                 cen::font(font_paths[id], font_size));
    auto it2 = data.find({id, font_size});
    return it2->second;
  }
};

template <bool owner>
struct init_font {
  using data = rgm::data<font_manager<owner>>;

  static void before(auto& this_worker)
    requires(owner)
  {
    static const decltype(this_worker) worker(this_worker);

    struct wrapper {
      static VALUE create(VALUE, VALUE path_) {
        RGMLOAD(path, const char*);

        font_manager<true>& fonts = RGMDATA(font_manager<true>);

        int id = fonts.insert(path);
        return INT2FIX(id);
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "font_create", wrapper::create, 1);
  }

  static void after(auto& worker) { RGMDATA(font_manager<owner>).data.clear(); }
};
}  // namespace rgm::rmxp