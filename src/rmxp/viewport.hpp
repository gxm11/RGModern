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
#include "builtin.hpp"
#include "drawable.hpp"

namespace rgm::rmxp {
/// @brief 设置 default_viewport 大小
/// default_viewport 始终跟 screen 一样大小。因为 resize_screen 在 base 命名
/// 空间中定义，而 viewport 类和 default_viewport 在 rmxp 命名空间中定义，为
/// 了简化代码结构，不在 resize_screen 中设置 default_viewport，而是使用此任务。
struct setup_default_viewport {
  /// @brief 全局变量 default_viewport 数据的地址
  /// 此数据虽然是全局变量，但是在 ruby worker 以外操作仍然要通过指针间接访问。
  viewport* v;

  void run(auto& worker) {
    base::renderstack& stack = RGMDATA(base::renderstack);

    /* 开发模式检查是否有 renderstack 的出入栈错误 */
    if constexpr (config::develop) {
      if (stack.stack.size() != 1) {
        cen::log_error(
            "setup default viewport failed, the stack depth is not equal to "
            "1!");
        throw std::length_error{"renderstack in setup default viewport"};
      }
    }

    /* 设置 default_viewport 的大小位置等属性 */
    v->ox = 0;
    v->oy = 0;
    v->rect.x = 0;
    v->rect.y = 0;
    v->rect.width = stack.current().width();
    v->rect.height = stack.current().height();
  }
};

/// @brief 创建 viewport 通用的 ruby 方法
/// 包括：
/// 1. create，创建 viewport，添加到 drawables 中
/// 2. dispose，将 viewport 从 drawables 中移除，释放相关资源，
///    同时释放所有绑定到该 viewport 的其他 drawable。
/// 3. set_z，修改 viewport 的 z 值
/// 4. refresh_value，同步更新值类型的属性
/// @see ./src/rmxp/init_drawable.hpp
struct init_viewport {
  static void before(auto& this_worker) {
    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /* ruby method: Base#viewport_create -> drawables::insert */
      static VALUE create(VALUE, VALUE viewport_) {
        drawables& data = RGMDATA(drawables);
        id2z& cache_z = RGMDATA(id2z);

        z_index v_zi;
        viewport v;
        v.p_drawables = std::make_unique<drawables>();

        v_zi << viewport_;
        v << viewport_;

        /* id 在 drawables 中等价于 id 在 cache_z 中 */
        cache_z.insert(v_zi.id, v_zi.z);
        data.m_data.emplace(std::move(v_zi), std::move(v));

        /* 返回 viewport 里蕴含的指针 */
        viewport* data_ptr = &std::get<viewport>(data.m_data[v_zi]);
        return ULL2NUM(reinterpret_cast<uint64_t>(data_ptr));
      }

      /* ruby method: Base#viewport_dispose -> drawables::erase */
      static VALUE dispose(VALUE, VALUE id_) {
        drawables& data = RGMDATA(drawables);
        id2z& cache_z = RGMDATA(id2z);

        RGMLOAD(id, uint64_t);

        /* id 不在 cache_z 中等价于 id 不在 drawables 中 */
        auto opt = cache_z.find_z(id);
        if (!opt) return Qnil;

        /* 从 cache_z 中移除 id */
        cache_z.erase(id);

        int z = opt.value();

        /* 从 cache_z 中移除所有属于此 viewport 的元素 */
        drawable& item = data.m_data[z_index{z, id}];
        viewport& v = std::get<viewport>(item);
        for (auto& [sub_zi, sub_item] : v.p_drawables->m_data) {
          cache_z.erase(sub_zi.id);
        }
        v.p_drawables->m_data.clear();

        /* 从 drawables 中移除 viewport */
        data.m_data.erase(z_index{z, id});
        return Qnil;
      }

      /* ruby method: Base#viewport_set_z -> drawables::set_z */
      static VALUE set_z(VALUE, VALUE viewport_, VALUE z_) {
        drawables& data = RGMDATA(drawables);
        id2z& cache_z = RGMDATA(id2z);

        z_index zi;
        zi << viewport_;

        RGMLOAD(z, int);
        int new_z = z;

        auto opt = cache_z.find_z(zi.id);
        if (!opt) return z_;
        cache_z.insert(zi.id, new_z);

        data.set_z(zi, new_z);
        return z_;
      }

      /* ruby method: Base#viewport_refresh_value -> viewport::refresh_value */
      static VALUE refresh_value(VALUE, VALUE data_ptr_, VALUE type_) {
        RGMLOAD(type, int);
        RGMLOAD(data_ptr, viewport*);

        if (data_ptr) {
          data_ptr->refresh_value(static_cast<word>(type));
        }
        return Qnil;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "viewport_create", wrapper::create,
                              1);
    rb_define_module_function(rb_mRGM_Base, "viewport_dispose",
                              wrapper::dispose, 1);
    rb_define_module_function(rb_mRGM_Base, "viewport_set_z", wrapper::set_z,
                              2);
    rb_define_module_function(rb_mRGM_Base, "viewport_refresh_value",
                              wrapper::refresh_value, 2);
  }
};
}  // namespace rgm::rmxp