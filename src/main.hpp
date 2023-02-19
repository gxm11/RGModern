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

namespace rgm {

/** @brief 运行逻辑流程的 worker */
// using worker_main = rgm::base::worker_main<rgm::rmxp::tasks_main>;
/** @brief 运行渲染流程的 worker */
// using worker_render = rgm::base::worker_render<rgm::rmxp::tasks_render>;

/** @brief 播放音乐音效的 worker */
// using worker_audio = rgm::base::worker_audio<rgm::rmxp::tasks_audio>;

using worker_main_sync =
    core::worker<base::kernel_ruby, base::tasks_main, rmxp::tasks_main>;
using worker_render_sync =
    core::worker<core::kernel_passive, base::tasks_render, rmxp::tasks_render>;
using worker_audio_sync =
    core::worker<core::kernel_passive, base::tasks_audio, rmxp::tasks_audio>;

using worker_main_async =
    core::worker<base::kernel_ruby, core::synchronize_signal<0>,
                 base::tasks_main, rmxp::tasks_main>;
using worker_render_async =
    core::worker<core::kernel_passive, core::synchronize_signal<1>,
                 base::tasks_render, rmxp::tasks_render>;
using worker_audio_async =
    core::worker<core::kernel_passive, core::synchronize_signal<2>,
                 base::tasks_audio, rmxp::tasks_audio>;

/** @brief 最终引擎由多个 worker 组合而来 */
// using engine_t = rgm::core::scheduler<worker_main, worker_render,
// worker_audio>;

using engine_sync_t = core::scheduler<std::false_type, worker_main_sync,
                                      worker_render_sync, worker_audio_sync>;
using engine_async_t = core::scheduler<std::true_type, worker_main_async,
                                       worker_render_async, worker_audio_async>;

// magic_cast 的特化处理
RGMENGINE(engine_sync_t, false);
RGMENGINE(engine_async_t, true);
}  // namespace rgm