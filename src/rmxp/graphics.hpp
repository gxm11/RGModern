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
#include "bitmap.hpp"
#include "builtin.hpp"
#include "render_base.hpp"
#include "render_tilemap.hpp"
#include "render_transition.hpp"
#include "render_viewport.hpp"
#include "table.hpp"

namespace rgm::rmxp {
core::stopwatch graphics_timer("graphics");

/**
 * @brief 创建 Graphics 相关的 ruby 方法
 */
struct init_graphics {
  static void before(auto& this_worker) {
    static decltype(auto) worker = this_worker;

    struct wrapper {
      static void update_tilemap(z_index zi, size_t depth = 0) {
        tilemap_manager& tm = RGMDATA(tilemap_manager);
        tables* p_tables = &(RGMDATA(tables));

        // size_t depth = v_ptr ? 1 : 0;
        // 如果即将绘制的内容不是tilemap，尽可能的创建
        // render<overlayer<tilemap>>
        while (true) {
          auto [p_info, index] = tm.next_layer(zi, depth);
          if (index == 0) break;
          if (!p_info->p_tilemap->skip()) {
            worker >> render<overlayer<tilemap>>{p_info, p_tables, index};
          }
        }
      }

      static VALUE update(VALUE, VALUE screen_width_, VALUE screen_height_) {
        // 当前绘制任务对应的 viewport 对象的指针
        // viewport* v_ptr = nullptr;
        tables* p_tables = &(RGMDATA(tables));
        tilemap_manager& tm = RGMDATA(tilemap_manager);
        RGMLOAD(screen_width, int);
        RGMLOAD(screen_height, int);
        default_viewport.rect.width = screen_width;
        default_viewport.rect.height = screen_height;
        // 跳过绘制的 lambda
        auto visitor_skip = [](auto& item) -> bool {
          // 此分支目前只对 window 和 viewport 有效
          // Plane 和 Tilemap 是平铺的
          // Tilemap 虽然可以不平铺，但是其宽和高不确定
          // Sprite 判断比较复杂，在 render<sprite> 里实现
          if constexpr (requires { item.visible(rect{}); }) {
            // const rect& r =
            //     v_ptr ? v_ptr->rect : rect{0, 0, screen_width,
            //     screen_height};
            const rect& r =
                item.p_viewport ? item.p_viewport->rect : default_viewport.rect;
            return item.skip() || !item.visible(r);
          } else {
            return item.skip();
          }
        };
        // 发送绘制任务的 lambda
        auto visitor_render = [p_tables, &tm]<typename T>(T& item) {
          // 不处理 viewport
          if constexpr (std::same_as<T, viewport>) return;
          // 以 CRTP 形式继承自 drawable_object 的对象是数据的拥有者，刷新数据
          if constexpr (std::is_base_of_v<drawable_object<T>, T>) {
            item.refresh_object();
          }
          if constexpr (std::same_as<T, tilemap>) {
            // 如果是 tilemap，继续刷新 autotiles 属性
            // 注意这里会修改render_target，需要还原。
            item.autotiles <<
                [](auto id) { worker >> bitmap_make_autotile{id}; };
            // tilemap 在绘制时要传递 p_tables
            worker >> render<T>{&item, /*v_ptr,*/ p_tables};
            // 刷新在 tm 中的存储的 tilemap_info
            z_index zi;
            zi << item.ruby_object;
            // size_t depth = v_ptr ? 1 : 0;
            size_t depth = item.p_viewport ? 1 : 0;
            tm.setup(zi, item, depth);
          } else {
            // 生成通用的绘制任务
            worker >> render<T>{&item /*, v_ptr*/};
          }
        };

        graphics_timer.step(4);
        graphics_timer.start();
        // 处理一次输入事件
        worker >> base::poll_event{};
        graphics_timer.step(1);
        // 绘制开始
        worker >> base::clear_screen{};
        // 遍历 drawables，如果是 Viewport，则再遍历一层
        drawables& data = RGMDATA(drawables);
        for (auto& [zi, item] : data.m_data) {
          if (std::visit(visitor_skip, item)) continue;
          update_tilemap(zi);

          // 非 viewport 的情况
          if (!std::holds_alternative<viewport>(item)) {
            std::visit(visitor_render, item);
            continue;
          }
          // viewport 的情况，继续遍历 m_data
          viewport& v = std::get<viewport>(item);
          // 设置 v_ptr
          // v_ptr = &v;
          v.refresh_object();

          worker >> before_render_viewport{&v};
          for (auto& [sub_zi, sub_item] : v.p_drawables->m_data) {
            if (std::visit(visitor_skip, sub_item)) continue;
            update_tilemap(sub_zi, 1 /*, v_ptr*/);

            std::visit(visitor_render, sub_item);
          }
          update_tilemap(z_index{INT32_MAX, 0}, 1 /*, v_ptr*/);
          worker >> after_render_viewport{&v};

          // 清空 v_ptr
          // v_ptr = nullptr;
        }
        update_tilemap(z_index{INT32_MAX, 0});
        // 绘制结束
        graphics_timer.step(2);
        // 线程进入等待，直到绘制完成
        RGMWAIT(1);
        graphics_timer.step(3);
        return Qnil;
      }

      static VALUE transition(VALUE, VALUE freeze_id_, VALUE current_id_,
                              VALUE rate_, VALUE transition_id_, VALUE vague_) {
        RGMLOAD(freeze_id, uint64_t);
        RGMLOAD(current_id, uint64_t);
        RGMLOAD(rate, double);
        RGMLOAD(transition_id, uint64_t);
        RGMLOAD(vague, int);

        worker >> base::poll_event{};
        // 绘制开始
        worker >> base::clear_screen{};

        if (transition_id == 0) {
          worker >> render_transition<1>{freeze_id, current_id, rate};
        } else {
          worker >> render_transition<2>{freeze_id, current_id, rate,
                                         transition_id, vague};
        }
        // 绘制结束
        RGMWAIT(1);
        return Qnil;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "graphics_update", wrapper::update,
                              2);
    rb_define_module_function(rb_mRGM_Base, "graphics_transition",
                              wrapper::transition, 5);
  }
};
}  // namespace rgm::rmxp