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
#include "render.hpp"
#include "renderstack.hpp"
#include "ruby.hpp"
#include "ruby_wrapper.hpp"
#include "signal.hpp"
#include "sound.hpp"
#include "sound_pitch.hpp"
#include "surface.hpp"
#include "textures.hpp"
#include "timer.hpp"

namespace rgm::base {
/** @brief 执行逻辑流程的 task，运行 ruby 脚本 */
using tasks_main =
    std::tuple<init_ruby, init_synchronize, init_counter, init_surfaces,
               interrupt_signal, init_music, music_finish_callback, init_sound>;
/** @brief 执行渲染流程的 task，使用 SDL2 创建窗口，绘制画面 */
using tasks_render =
    std::tuple<init_sdl2, init_renderstack, init_textures, poll_event,
               clear_screen, present_window, resize_window, resize_screen,
               set_title, set_fullscreen>;
/** @brief 执行音乐播放的 task，使用 SDL2 Mixer 播放音乐和音效 */
using tasks_audio = std::tuple<
    music_create, music_dispose,
    music_play, music_fade_in, music_set_volume, music_set_position,
    music_resume, music_pause, music_halt, music_rewind, music_fade_out,
    music_get_volume, music_get_position, music_get_state, sound_create,
    sound_dispose, sound_play, sound_stop, sound_fade_in, sound_fade_out,
    sound_set_volume, sound_set_pitch, sound_get_state, sound_get_channel>;
}  // namespace rgm::base
