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
#include "external.hpp"
#include "mouse.hpp"
#include "ping.hpp"
#include "ruby_wrapper.hpp"
#include "textinput.hpp"

namespace rgm::ext {
/// @brief 执行 ruby 脚本的 task，运行游戏的主要逻辑（即 RGSS 脚本）
using tasks_ruby =
    std::tuple<init_textinput, init_external, init_ping, init_mouse, text_input,
               text_edit, mouse_motion, mouse_press, mouse_release, mouse_wheel,
               ruby_callback, regist_external_data<0>>;

/// @brief 执行渲染流程的 task，使用 SDL2 创建窗口，绘制画面并处理事件
using tasks_render =
    std::tuple<init_text_event, init_mouse_event, textinput_start,
               textinput_stop, regist_external_data<1>>;

/// @brief 执行音乐播放的 task，使用 SDL2 Mixer 播放音乐和音效
using tasks_audio = std::tuple<>;

/// @brief 执行旁路操作的 task，这个 worker 用于一些耗时的计算
using tasks_aside = std::tuple<ruby_async<ping>>;
}  // namespace rgm::ext