// Copyright (c) 2022 Xiaomi Guo
// Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan
// PSL v2. You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
// Mulan PSL v2 for more details.
#include "config.hpp"
#include "rmxp/rmxp.hpp"

/** @brief 运行逻辑流程的 worker */
using worker_main = rgm::base::worker_main<rgm::rmxp::tasks_main>;

/** @brief 运行渲染流程的 worker */
using worker_render = rgm::base::worker_render<rgm::rmxp::tasks_render>;

/** @brief 播放音乐音效的 worker */
using worker_audio = rgm::base::worker_audio<rgm::rmxp::tasks_audio>;

/** @brief 最终引擎由多个 worker 组合而来 */
using engine_t = rgm::core::scheduler<worker_audio, worker_render, worker_main>;

// magic_cast 的特化处理
RGMENGINE(engine_t);
