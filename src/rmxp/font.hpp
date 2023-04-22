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
/// @brief 全局变量，储存了所有字体与其路径的映射关系
/// @name data
/// 只有拥有数据 font_manager<true> 的 worker 能独写此变量，
/// 其他 orker 只能读。
std::vector<std::string> font_paths = {};

/// @brief 管理字体的数据类
/// @tparam 是否对全局变量 font_paths 有读写权限
/// @name data
/// 为了保证线程安全，只能有 1 个 font_manager<true>。
template <bool owner>
struct font_manager {
  /// @brief 存储所有字体的 map 容器，{字体ID, 字号} => font对象
  /// 字体的数量相对会少很多，这里使用 map 来管理，较 hash 更好。
  std::map<std::pair<int, int>, cen::font> m_data;

  /// @brief 向 font_paths 中添加一个字体的路径
  /// @return 返回这个路径在 font_paths 中的位置
  /// 只在 owner 为 true 时，此函数有定义。
  int try_insert(std::string_view path)
    requires(owner)
  {
    auto it = std::find(font_paths.begin(), font_paths.end(), path);
    /* 下面的 push_back 可能导致迭代器失效，先计算搜索结果 */
    int id = static_cast<int>(it - font_paths.begin());

    if (it == font_paths.end()) {
      font_paths.push_back(std::string(path));
    }

    return id;
  }

  /// @brief 使用 font id 和 font size 获取对应的 cen::font 对象
  cen::font& get(int id, int font_size) {
    /* 先在 m_data 中查找，若找到了返回结果 */
    auto it = m_data.find({id, font_size});
    if (it != m_data.end()) return it->second;

    /* 将新的 cen::font 对象添加到 m_data 中 */
    m_data.emplace(std::pair{id, font_size},
                   cen::font(font_paths.at(id), font_size));

    /* 再次查找并返回结果 */
    return m_data.find({id, font_size})->second;
  }
};

/// @brief 数据类 font_manager 相关的初始化类
/// @tparam owner font_manager 的模板参数
/// @name task
template <bool owner>
struct init_font {
  using data = std::tuple<font_manager<owner>>;

  static void before(auto& this_worker)
    requires(owner)
  {
    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /* ruby method: Base#font_create -> font_manager::try_insert */
      static VALUE create(VALUE, VALUE path_) {
        RGMLOAD(path, std::string_view);

        /* 只有 owner = true 的 font_manager 才有 try_insert 函数 */
        font_manager<true>& fonts = RGMDATA(font_manager<true>);

        int id = fonts.try_insert(path);
        return INT2FIX(id);
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "font_create", wrapper::create, 1);

    /* 预留一些空间，减少内存分配次数 */
    font_paths.reserve(32);
  }

  static void after(auto& worker) {
    /* cen::font 要提前释放，否则会导致 Segmentation fault */
    RGMDATA(font_manager<owner>).m_data.clear();
  }
};
}  // namespace rgm::rmxp