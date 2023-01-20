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
/** @brief 执行逻辑流程的 worker，运行 ruby 脚本 */
template <typename... Args>
using worker_main = core::worker<kernel_ruby, core::synchronize_signal<0>,
                                 init_ruby, init_synchronize, init_counter,
                                 init_surfaces, interrupt_signal, Args...>;

/** @brief 执行渲染流程的 worker，使用 SDL2 创建窗口，绘制画面 */
template <typename... Args>
using worker_render =
    core::worker<core::kernel_passive, core::synchronize_signal<1>, init_sdl2,
                 init_renderstack, init_textures, poll_event, clear_screen,
                 present_window, resize_window, resize_screen, set_title,
                 set_fullscreen, Args...>;

/** @brief 执行音乐播放的 worker，使用 SDL2 Mixer 播放音乐和音效 */
template <typename... Args>
using worker_audio =
    core::worker<core::kernel_passive, core::synchronize_signal<2>,
                 init_music_manager, init_sound_manager, bgm_play, bgm_stop,
                 me_play, me_stop, music_finish, bgm_pos, bgs_play, bgs_stop,
                 se_play, se_stop, Args...>;
}  // namespace rgm::base
