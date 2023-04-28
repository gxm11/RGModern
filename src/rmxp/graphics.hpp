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
/// @brief 秒表 graphics，统计每次 graphics.update 花费的时间
/// 起点和终点都在 graphics.update 中
core::stopwatch graphics_timer("graphics");

/// @brief 画面渲染相关操作的初始化类
/// @name task
struct init_graphics {
  static void before(auto& this_worker) {
    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /* 尝试尽可能多地发送 render<overlayer<tilemap>> */
      static void render_tilemap_overlayer(z_index zi, size_t depth = 0) {
        tilemap_manager& tm = RGMDATA(tilemap_manager);
        tables* p_tables = &(RGMDATA(tables));
        /*
         * 如果即将绘制的内容不是tilemap，尽可能地发送 overlayer<tilemap>
         * 的绘制任务。
         */
        while (true) {
          /* 从 tilemap_manager 中获取下一层 overlayer 的信息 */
          auto opt = tm.next_layer(zi, depth);
          if (!opt) break;

          auto [p_info, index] = opt.value();

          if (!p_info->p_tilemap->skip()) {
            worker >> render<overlayer<tilemap>>{p_info, p_tables, index};
          }
        }
      }

      /* ruby method: Base#graphics_update -> render<T> */
      static VALUE update(VALUE, VALUE screen_width_, VALUE screen_height_) {
        RGMLOAD(screen_width, int);
        RGMLOAD(screen_height, int);

        tables* p_tables = &(RGMDATA(tables));
        tilemap_manager& tm = RGMDATA(tilemap_manager);

        /* 设置 default_viewport 的大小 */
        default_viewport.rect.width = screen_width;
        default_viewport.rect.height = screen_height;

        /* 跳过绘制的 lambda */
        auto visitor_skip = [](auto& item) -> bool { return item.skip(); };

        /* 发送绘制任务的 lambda */
        auto visitor_render = [p_tables, &tm]<typename T>(T& item) {
          /* 不在这里处理 viewport */
          if constexpr (std::same_as<T, viewport>) return;

          /*
           * 刷新对象类型的成员变量对应的数据。只有以 CRTP 形式继承自
           * drawable_object 的对象是数据的拥有者，才会触发刷新。
           */
          if constexpr (std::is_base_of_v<drawable_object<T>, T>) {
            item.refresh_object();
          }

          if constexpr (std::same_as<T, tilemap>) {
            /*
             * 如果是 tilemap，则还需要判断 autotiles 是否发生了变化，
             * 并根据 autotiles 中储存的 Bitmap，绘制新的 autotile 图块。
             */
            item.autotiles <<
                [](auto id) { worker >> bitmap_make_autotile{id}; };

            /* 向 tilemap_info 中添加当前的 tilemap */
            tm.insert(item);

            /* 发送 render<tilemap>，需要额外传递 p_tables */
            worker >> render<tilemap>{&item, p_tables};
          } else {
            /* 发送 render<T> */
            worker >> render<T>{&item};
          }
        };

        /* 计时器的起点和终点 */
        graphics_timer.step(4);
        graphics_timer.start();

        /* 处理当前积压的事件 */
        worker >> base::poll_event{};

        /* 绘制开始，计时阶段 1 */
        graphics_timer.step(1);

        /* 清空屏幕 */
        worker >> base::clear_screen{};

        /* 遍历 drawables，如果是 Viewport，则再遍历一层 */
        drawables& data = RGMDATA(drawables);
        for (auto& [zi, item] : data.m_data) {
          /* 跳过绘制的场合就进入下一个 item */
          if (std::visit(visitor_skip, item)) continue;

          /* 尝试插入 tilemap 的 overlayer */
          render_tilemap_overlayer(zi);

          /* 非 viewport 的情况，发送 render 任务 */
          if (!std::holds_alternative<viewport>(item)) {
            std::visit(visitor_render, item);
            continue;
          }

          /* 刷新 viewport 的对象类型的成员变量对应的数据 */
          viewport& v = std::get<viewport>(item);
          v.refresh_object();

          /* viewport 的前处理 */
          worker >> before_render_viewport{&v};

          /* 遍历 viewport 中的 drawables */
          for (auto& [sub_zi, sub_item] : v.p_drawables->m_data) {
            if (std::visit(visitor_skip, sub_item)) continue;
            render_tilemap_overlayer(sub_zi, 1);

            std::visit(visitor_render, sub_item);
          }

          /* 尝试插入 tilemap 的 overlayer */
          render_tilemap_overlayer(z_index{INT32_MAX, 0}, 1);

          /* viewport 的后处理 */
          worker >> after_render_viewport{&v};
        }

        /* 尝试插入 tilemap 的 overlayer */
        render_tilemap_overlayer(z_index{INT32_MAX, 0});

        /* 绘制任务发送完毕，计时阶段 2 */
        graphics_timer.step(2);

        /* 线程进入等待，直到绘制完成 */
        RGMWAIT(1);

        /* 绘制结束，计时阶段 3 */
        graphics_timer.step(3);

        return Qnil;
      }

      /* ruby method: Base#graphics_transition -> render_transition */
      static VALUE transition(VALUE, VALUE freeze_id_, VALUE current_id_,
                              VALUE rate_, VALUE transition_id_, VALUE vague_) {
        RGMLOAD(freeze_id, uint64_t);
        RGMLOAD(current_id, uint64_t);
        RGMLOAD(rate, double);
        RGMLOAD(transition_id, uint64_t);
        RGMLOAD(vague, int);

        /* 计时器的起点和终点 */
        graphics_timer.step(4);
        graphics_timer.start();

        /* 处理当前积压的事件 */
        worker >> base::poll_event{};

        /* 绘制开始，计时阶段 1 */
        graphics_timer.step(1);

        /* 清空屏幕 */
        worker >> base::clear_screen{};

        /* 发送 render_transition */
        if (transition_id == 0) {
          worker >> render_transition<1>{freeze_id, current_id, rate};
        } else {
          worker >> render_transition<2>{freeze_id, current_id, rate,
                                         transition_id, vague};
        }

        /* 绘制任务发送完毕，计时阶段 2 */
        graphics_timer.step(2);

        /* 线程进入等待，直到绘制完成 */
        RGMWAIT(1);

        /* 绘制结束，计时阶段 3 */
        graphics_timer.step(3);
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