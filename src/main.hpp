// zlib License

// copyright (C) 2023 Guoxiaomi and Krimiston

// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.

// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:

// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "rmxp/rmxp.hpp"

namespace rgm {
/** @brief 运行逻辑流程的 worker */
using worker_main_sync =
    core::worker<base::kernel_ruby, base::tasks_main, rmxp::tasks_main>;
/** @brief 运行渲染流程的 worker */
using worker_render_sync =
    core::worker<core::kernel_passive, base::tasks_render, rmxp::tasks_render>;
/** @brief 播放音乐音效的 worker */
using worker_audio_sync =
    core::worker<core::kernel_passive, base::tasks_audio, rmxp::tasks_audio>;
/** @brief 最终引擎由多个 worker 组合而来 */
using engine_sync_t =
    core::scheduler<core::cooperation::exclusive, worker_render_sync,
                    worker_audio_sync, worker_main_sync>;
// magic_cast 的特化处理
RGMENGINE(engine_sync_t);

// 异步的 scheduler 和 worker，特征是 task 里包含了 core::synchronize_signal
using worker_main_async =
    core::worker<base::kernel_ruby, std::tuple<core::synchronize_signal<0>>,
                 base::tasks_main, rmxp::tasks_main>;
using worker_render_async =
    core::worker<core::kernel_passive, std::tuple<core::synchronize_signal<1>>,
                 base::tasks_render, rmxp::tasks_render>;
using worker_audio_async =
    core::worker<core::kernel_passive, std::tuple<core::synchronize_signal<2>>,
                 base::tasks_audio, rmxp::tasks_audio>;

using engine_async_t =
    core::scheduler<core::cooperation::asynchronous, worker_render_async,
                    worker_audio_async, worker_main_async>;

RGMENGINE(engine_async_t);
}  // namespace rgm