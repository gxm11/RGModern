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
#include "cen_library.hpp"
#include "core/core.hpp"
#include "counter.hpp"
#include "detail.hpp"
#include "incbin.hpp"
#include "init_ruby.hpp"
#include "init_sdl2.hpp"
#include "kernel_ruby.hpp"
#include "music.hpp"
#include "music_manager.hpp"
#include "render.hpp"
#include "renderstack.hpp"
#include "ruby.hpp"
#include "signal.hpp"
#include "sound_manager.hpp"
#include "sound_pitch.hpp"
#include "surface.hpp"
#include "textures.hpp"
#include "timer.hpp"

namespace rgm::base {
/** @brief 执行逻辑流程的 task，运行 ruby 脚本 */
using tasks_main = std::tuple<init_ruby, init_synchronize, init_counter,
                              init_surfaces, interrupt_signal, init_music>;
/** @brief 执行渲染流程的 task，使用 SDL2 创建窗口，绘制画面 */
using tasks_render =
    std::tuple<init_sdl2, init_renderstack, init_textures, poll_event,
               clear_screen, present_window, resize_window, resize_screen,
               set_title, set_fullscreen>;
/** @brief 执行音乐播放的 task，使用 SDL2 Mixer 播放音乐和音效 */
using tasks_audio =
    std::tuple<init_music_manager, init_sound_manager, bgm_play, bgm_stop,
               me_play, me_stop, music_finish, bgm_pos, bgs_play, bgs_stop,
               se_play, se_stop, music_create, music_dispose, music_play,
               music_fade_in, music_set_volume, music_set_position, music_pause,
               music_halt, music_fade_out, music_is_playing>;
}  // namespace rgm::base
