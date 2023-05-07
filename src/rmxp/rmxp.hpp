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
#include "blend_type.hpp"
#include "builtin.hpp"
#include "controller.hpp"
#include "drawable.hpp"
#include "drawable_object.hpp"
#include "event.hpp"
#include "extension.hpp"
#include "font.hpp"
#include "graphics.hpp"
#include "init_drawable.hpp"
#include "input.hpp"
#include "messagebox.hpp"
#include "overlayer.hpp"
#include "palette.hpp"
#include "render_base.hpp"
#include "render_plane.hpp"
#include "render_sprite.hpp"
#include "render_tilemap.hpp"
#include "render_transition.hpp"
#include "render_viewport.hpp"
#include "render_window.hpp"
#include "shader/shader.hpp"
#include "table.hpp"
#include "tilemap_manager.hpp"
#include "viewport.hpp"
#include "word.hpp"

namespace rgm::rmxp {
/// @brief 执行 ruby 脚本的 task，运行游戏的主要逻辑（即 RGSS 脚本）
using tasks_ruby =
    std::tuple<init_extension, init_word, init_bitmap, init_table,
               init_tilemap_manager, init_viewport, init_graphics, init_input,
               init_controller, init_drawable_base, init_drawable<sprite>,
               init_drawable<window>, init_drawable<plane>,
               init_drawable<tilemap>, init_font<true>, init_palette,
               init_message, key_release, key_press, controller_axis_move,
               controller_button_release, controller_button_press>;

/// @brief 执行渲染流程的 task，使用 SDL2 创建窗口，绘制画面并处理事件
using tasks_render = std::tuple<
    shader::init_shader, init_event, init_blend_type, init_font<false>,
    bitmap_create<1>, bitmap_create<2>, bitmap_create<3>, bitmap_dispose,
    bitmap_save_png, bitmap_capture_screen, bitmap_blt, bitmap_stretch_blt,
    bitmap_fill_rect, bitmap_hue_change, bitmap_grayscale, bitmap_draw_text,
    bitmap_get_pixel, bitmap_capture_palette, bitmap_make_autotile,
    bitmap_reload_autotile, setup_default_viewport, before_render_viewport,
    after_render_viewport, render<sprite>, render<plane>, render<window>,
    render<overlayer<window>>, render<tilemap>, render<overlayer<tilemap>>,
    render_transition<1>, render_transition<2>, tilemap_set_info, message_show,
    controller_rumble, controller_rumble_triggers>;

/// @brief 执行音乐播放的 task，使用 SDL2 Mixer 播放音乐和音效
using tasks_audio = std::tuple<>;

/// @brief 执行 Table 操作的 task，这个 worker 用于一些耗时的计算
using tasks_table = std::tuple<>;
}  // namespace rgm::rmxp
