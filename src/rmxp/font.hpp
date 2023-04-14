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
#include "detail.hpp"

namespace rgm::rmxp {
// 这是一个全局变量，存储了所有字体的路径。
// 只有标记为 owner 的线程才能读写，其他线程只能读。
std::array<std::string, 128> font_paths = {};

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
          "Number of different fonts must less than 128!");
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
  using data = std::tuple<font_manager<owner>>;

  static void before(auto& this_worker)
    requires(owner)
  {
    static decltype(auto) worker = this_worker;

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