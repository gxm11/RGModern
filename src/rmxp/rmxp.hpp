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
#include "bitmap.hpp"
#include "blend_type.hpp"
#include "builtin.hpp"
#include "detail.hpp"
#include "drawable.hpp"
#include "drawable_object.hpp"
#include "event.hpp"
#include "extension.hpp"
#include "font.hpp"
#include "graphics.hpp"
#include "init_drawable.hpp"
#include "input.hpp"
#include "messagebox.hpp"
#include "mouse.hpp"
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
#include "textinput.hpp"
#include "tilemap_manager.hpp"
#include "title.hpp"
#include "viewport.hpp"
#include "word.hpp"
#include "zip.hpp"

namespace rgm::rmxp {
/** @brief 逻辑流程的 worker 的可执行任务列表 */
using tasks_main =
    std::tuple<init_extension, init_detail, init_bitmap, init_table,
               init_tilemap_manager, init_viewport, init_graphics, init_input,
               init_drawable_base, init_drawable<sprite>, init_drawable<window>,
               init_drawable<plane>, init_drawable<tilemap>, init_zip,
               init_font<true>, init_palette,  
               init_textinput, init_title, init_message, key_release, key_press,
               text_input, text_edit>;

/** @brief 渲染流程的 worker 的可执行任务列表 */
using tasks_render = std::tuple<
    init_shader, init_event, init_blend_type, init_font<false>,
    bitmap_create<1>, bitmap_create<2>, bitmap_create<3>, bitmap_create<4>,
    bitmap_dispose, bitmap_save_png, bitmap_capture_screen, bitmap_blt,
    bitmap_stretch_blt, bitmap_fill_rect, bitmap_hue_change, bitmap_draw_text,
    bitmap_get_pixel, bitmap_capture_palette, before_render_viewport,
    after_render_viewport, render<sprite>, render<plane>, render<window>,
    render<overlayer<window>>, render<tilemap>, render<overlayer<tilemap>>,
    render_transition<1>, render_transition<2>, textinput_start, textinput_stop,
    regist_external_data<1>, message_show>;

using tasks_audio = std::tuple<>;
}  // namespace rgm::rmxp
