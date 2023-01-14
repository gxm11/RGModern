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
    static const decltype(this_worker) worker(this_worker);

    struct wrapper {
      static void update_tilemap(z_index zi, viewport* v_ptr = nullptr) {
        tilemap_manager& tm = RGMDATA(tilemap_manager);
        tables* p_tables = &(RGMDATA(tables));

        size_t depth = v_ptr ? 1 : 0;
        // 如果即将绘制的内容不是tilemap，尽可能的创建
        // render<overlayer<tilemap>>
        while (true) {
          auto [p_info, index] = tm.next_layer(zi, depth);
          if (index == 0) break;
          if (!p_info->p_tilemap->skip()) {
            worker << render<overlayer<tilemap>>{p_info, v_ptr, p_tables,
                                                 index};
          }
        }
      }

      static VALUE update(VALUE) {
        // 跳过绘制的 lambda
        auto visitor_skip = [](auto& item) -> bool { return item.skip(); };
        // 当前绘制任务对应的 viewport 对象的指针
        viewport* v_ptr = nullptr;
        tables* p_tables = &(RGMDATA(tables));
        tilemap_manager& tm = RGMDATA(tilemap_manager);

        // 发送绘制任务的 lambda
        auto visitor_render = [p_tables, &tm, &v_ptr]<typename T>(T& item) {
          // 不处理 viewport
          if constexpr (std::same_as<T, viewport>) return;
          // 以 CRTP 形式继承自 drawable_object 的对象是数据的拥有者，刷新数据
          if constexpr (std::is_base_of_v<drawable_object<T>, T>) {
            item.refresh_object();
          }
          if constexpr (std::same_as<T, tilemap>) {
            // 如果是 tilemap，继续刷新 autotiles 属性
            // 注意这里会修改render_target，需要还原。
            item.autotiles << [](auto id) { worker << bitmap_create<3>{id}; };
            // tilemap 在绘制时要传递 p_tables
            worker << render<T>{&item, v_ptr, p_tables};
            // 刷新在 tm 中的存储的 tilemap_info
            z_index zi;
            zi << item.object;
            size_t depth = v_ptr ? 1 : 0;
            tm.setup(zi, item, depth);
          } else {
            // 生成通用的绘制任务
            worker << render<T>{&item, v_ptr};
          }
        };

        graphics_timer.step(4);
        graphics_timer.start();
        // 处理一次输入事件
        worker << base::poll_event{};
        graphics_timer.step(1);
        // 绘制开始
        worker << base::clear_screen{};
        // 遍历 drawables，如果是 Viewport，则再遍历一层
        drawables& data = RGMDATA(drawables);
        for (auto& [zi, item] : data) {
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
          v_ptr = &v;
          v.refresh_object();
          worker << before_render_viewport{v_ptr};
          for (auto& [sub_zi, sub_item] : v.m_data) {
            if (std::visit(visitor_skip, sub_item)) continue;
            update_tilemap(sub_zi, v_ptr);

            std::visit(visitor_render, sub_item);
          }
          update_tilemap(z_index{INT32_MAX, 0}, v_ptr);
          worker << after_render_viewport{v_ptr};
          // 清空 v_ptr
          v_ptr = nullptr;
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
        RGMLOAD(freeze_id, const uint64_t);
        RGMLOAD(current_id, const uint64_t);
        RGMLOAD(rate, double);
        RGMLOAD(transition_id, const uint64_t);
        RGMLOAD(vague, int);

        worker << base::poll_event{};
        // 绘制开始
        worker << base::clear_screen{};

        if (transition_id == 0) {
          worker << render_transition<1>{freeze_id, current_id, rate};
        } else {
          worker << render_transition<2>{freeze_id, current_id, rate,
                                         transition_id, vague};
        }
        // 绘制结束
        RGMWAIT(1);
        return Qnil;
      }

      static VALUE present(VALUE, VALUE scale_mode_) {
        RGMLOAD(scale_mode, int);

        worker << base::present_window{scale_mode};
        return Qnil;
      }

      static VALUE resize_screen(VALUE, VALUE width_, VALUE height_) {
        RGMLOAD(width, int);
        RGMLOAD(height, int);

        worker << base::resize_screen{width, height};
        return Qnil;
      }

      static VALUE resize_window(VALUE, VALUE width_, VALUE height_) {
        RGMLOAD(width, int);
        RGMLOAD(height, int);

        worker << base::resize_window{width, height};
        return Qnil;
      }

      static VALUE set_fullscreen(VALUE, VALUE mode_) {
        RGMLOAD(mode, int);

        worker << base::set_fullscreen{mode};
        return Qnil;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "graphics_update", wrapper::update,
                              0);
    rb_define_module_function(rb_mRGM_Base, "graphics_present",
                              wrapper::present, 1);
    rb_define_module_function(rb_mRGM_Base, "graphics_resize_screen",
                              wrapper::resize_screen, 2);
    rb_define_module_function(rb_mRGM_Base, "graphics_resize_window",
                              wrapper::resize_window, 2);
    rb_define_module_function(rb_mRGM_Base, "graphics_transition",
                              wrapper::transition, 5);
    rb_define_module_function(rb_mRGM_Base, "graphics_set_fullscreen",
                              wrapper::set_fullscreen, 1);
  }
};
}  // namespace rgm::rmxp