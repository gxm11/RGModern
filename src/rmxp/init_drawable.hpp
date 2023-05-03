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
#include "drawable.hpp"
#include "tilemap_manager.hpp"

namespace rgm::rmxp {

/*
 * Drawable，可绘制类在 ruby 中会调用以下接口操作 C++ 层的对应数据。
 * 1. create，创建相应的对象，存储在 drawables 中；
 * 2. dispose，将相应的对象从 drawables 中移除；
 * 3. set_z，修改 z 值，从而改变对象在 drawables 中的位置；
 * 4. refresh_value，刷新对象的成员变量，重新读取 ruby 中对应的实例变量。
 * 其中，dispose 和 set_z 不关心具体的类型，直接操作整个 variant，
 * 但 create 和 refresh_value 的效果会跟随 Drawable 类型而变化。
 * 
 * 裸指针 @data_ptr，作为 create 的返回值，只在 refresh_value 用到。
 * 对于 dispose 和 set_z，都需要通过 z_index 查找对象，但是 z_index 不保存
 * 在 C++ 层中，故必须通过传入 id，通过 id2z 获得 z 值，组合成 z_index，再去
 * drawables 中查找，仅仅通过裸指针 @data_ptr 是无法实现的。
 * 
 * Viewport 虽然也是 Drawable，但是 Viewport 始终在第一层，故单独实现。
 */

/// @brief 创建 drawable 通用的 ruby 方法
/// 方法包括：
/// 1. dispose
/// 2. set_z
struct init_drawable_base {
  /* 引入数据对象 drawables 和 id2z */
  using data = std::tuple<drawables, id2z>;

  static void before(auto& this_worker) {
    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /* ruby method: Base#drawable_dispose -> drawables::erase */
      static VALUE dispose(VALUE, VALUE viewport_, VALUE id_) {
        drawables& data = RGMDATA(drawables);
        id2z& cache_z = RGMDATA(id2z);

        RGMLOAD(id, uint64_t);

        /* id 不在 cache_z 中等价于 id 不在 drawables 中 */
        auto opt = cache_z.find_z(id);
        if (!opt) return Qnil;

        /* 从 cache_z 中移除 id */
        cache_z.erase(id);

        /* 从 tilemap_manager 的 infos 中移除 id */
        RGMDATA(tilemap_manager).infos.erase(id);

        int z = opt.value();

        drawables* p_data = &data;
        /* 找到 viewport，以及对应的 p_drawables */
        if (viewport_ != Qnil) {
          z_index v_zi;
          v_zi << viewport_;
          viewport& v = std::get<viewport>(data.m_data[v_zi]);
          p_data = v.p_drawables.get();
        }

        auto node = p_data->m_data.extract(z_index{z, id});
        if (!node.empty()) {
          /* 处理 fixed delta_z overlayer */
          auto erase_visitor = [=]([[maybe_unused]] auto&& item) {
            if constexpr (requires { item.fixed_overlayer_zs; }) {
              for (uint16_t delta_z : item.fixed_overlayer_zs) {
                p_data->m_data.erase(z_index{z + delta_z, id});
              }
            }
          };
          std::visit(erase_visitor, node.mapped());
        }
        return Qnil;
      }

      /* ruby method: Base#drawable_set_z -> drawables::set_z */
      static VALUE set_z(VALUE, VALUE drawable_, VALUE viewport_, VALUE z_) {
        drawables& data = RGMDATA(drawables);
        id2z& cache_z = RGMDATA(id2z);

        RGMLOAD(z, int);

        z_index zi;
        zi << drawable_;

        auto opt = cache_z.find_z(zi.id);
        if (!opt) return z_;

        int new_z = z;
        cache_z.insert(zi.id, new_z);

        drawables* p_data = &data;
        /* 找到 viewport，以及对应的 p_drawables */
        if (viewport_ != Qnil) {
          z_index v_zi;
          v_zi << viewport_;
          viewport& v = std::get<viewport>(data.m_data[v_zi]);
          p_data = v.p_drawables.get();
        }

        p_data->set_z(zi, new_z);
        return z_;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "drawable_dispose",
                              wrapper::dispose, 2);
    rb_define_module_function(rb_mRGM_Base, "drawable_set_z", wrapper::set_z,
                              3);
  }
};

/// @brief 创建 drawable 特化的 ruby 方法
/// @tparam T_Drawable 目标类型
/// 方法包括：
/// 1. create
/// 2. refresh_value
template <typename T_Drawable>
struct init_drawable {
  static void before(auto& this_worker) {
    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /* ruby method: Base#drawable_create -> drawables::insert */
      static VALUE create(VALUE, VALUE drawable_) {
        drawables& data = RGMDATA(drawables);
        id2z& cache_z = RGMDATA(id2z);

        z_index zi;
        T_Drawable drawable;
        zi << drawable_;
        drawable << drawable_;

        viewport* v_ptr = drawable.p_viewport;

        /* id 在 drawables 中等价于 id 在 cache_z 中 */
        cache_z.insert(zi.id, zi.z);

        /* 向 tilemap_manager 的 infos 中添加 id */
        if constexpr (std::is_same_v<T_Drawable, tilemap>) {
          tilemap_manager& tm = RGMDATA(tilemap_manager);
          tm.infos.emplace(zi.id, tilemap_info());
        }

        drawables* p_data = v_ptr ? v_ptr->p_drawables.get() : &data;
        p_data->m_data.emplace(zi, std::move(drawable));
        /* 处理 fixed delta_z overlayer */
        if constexpr (requires { T_Drawable::fixed_overlayer_zs; }) {
          T_Drawable* p_drawable = &std::get<T_Drawable>(p_data->m_data[zi]);
          size_t index = 0;
          for (uint16_t delta_z : T_Drawable::fixed_overlayer_zs) {
            p_data->m_data.emplace(z_index{zi.z + delta_z, zi.id},
                                   overlayer<T_Drawable>{p_drawable, index});
            ++index;
          }
        }

        /* 返回 drawable 里蕴含的指针 */
        T_Drawable* data_ptr = &std::get<T_Drawable>(p_data->m_data[zi]);
        return ULL2NUM(reinterpret_cast<uint64_t>(data_ptr));
      }

      /* ruby method: Base#drawable_refresh_value -> drawable::refresh_value */
      static VALUE refresh_value(VALUE, VALUE data_ptr_, VALUE type_) {
        RGMLOAD(type, int);
        RGMLOAD(data_ptr, T_Drawable*);

        if (data_ptr) {
          data_ptr->refresh_value(static_cast<word>(type));
        }
        return Qnil;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    {
      std::string name = std::string{T_Drawable::name} + "_create";
      rb_define_module_function(rb_mRGM_Base, name.data(), wrapper::create, 1);
    }
    {
      std::string name = std::string{T_Drawable::name} + "_refresh_value";
      rb_define_module_function(rb_mRGM_Base, name.data(),
                                wrapper::refresh_value, 2);
    }
  }
};
}  // namespace rgm::rmxp