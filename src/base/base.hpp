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
#include "controller.hpp"
#include "core/core.hpp"
#include "counter.hpp"
#include "detail.hpp"
#include "embeded.hpp"
#include "init_ruby.hpp"
#include "init_sdl2.hpp"
#include "init_timer.hpp"
#include "kernel_ruby.hpp"
#include "music.hpp"
#include "render.hpp"
#include "renderstack.hpp"
#include "ruby_wrapper.hpp"
#include "sound.hpp"
#include "sound_pitch.hpp"
#include "surface.hpp"
#include "texture.hpp"
#include "timer.hpp"
#include "window.hpp"

namespace rgm::base {
/// @brief 执行 ruby 脚本的 task，运行游戏的主要逻辑（即 RGSS 脚本）
/// init_ruby 必须是第一个！
using tasks_ruby =
    std::tuple<init_ruby, init_embeded, init_timer, init_counter, init_surfaces,
               init_music, init_sound, init_config, init_render, init_window,
               interrupt_signal, music_finish_callback, controller_connect,
               controller_disconnect>;

/// @brief 执行渲染流程的 task，使用 SDL2 创建窗口，绘制画面并处理事件
/// init_sdl2 必须是第一个！
using tasks_render =
    std::tuple<init_sdl2, init_renderstack, init_textures, poll_event,
               clear_screen, present_window, resize_window, resize_screen,
               set_title, set_fullscreen>;

/// @brief 执行音乐播放的 task，使用 SDL2 Mixer 播放音乐和音效
using tasks_audio =
    std::tuple<music_create, music_dispose, music_play, music_fade_in,
               music_set_volume, music_set_position, music_resume, music_pause,
               music_halt, music_rewind, music_fade_out, music_get_volume,
               music_get_position, music_get_state, sound_create, sound_dispose,
               sound_play, sound_stop, sound_fade_in, sound_fade_out,
               sound_set_volume, sound_set_pitch, sound_get_state,
               sound_get_channel>;

/// @brief 执行 Table 操作的 task，这个 worker 用于一些耗时的计算
/// @name todo
using tasks_table = std::tuple<>;
}  // namespace rgm::base
