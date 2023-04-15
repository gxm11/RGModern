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
#include "core/core.hpp"
#include "init_sdl2.hpp"

namespace rgm::base {
/// @brief 管理分层渲染，内置 stack 数据结构并提供缓存避免频繁申请 texture
/// @name data
struct renderstack {
  /*
  渲染栈（Stack）的层级：
  +---------------------------------------------+
  |   Drawables 的原始纹理（texture），不在栈中   |
  +---------------------------------------------+
  |             Drawables 的效果处理             |
  +---------------------------------------------+
  |              有视口的 Drawables              |
  +---------------------------------------------+
  |     视口（Viewport）和无视口的 Drawables*     |
  +---------------------------------------------+
  |                 屏幕（Screen）               |
  +---------------------------------------------+
  |             窗口（Window），不在栈中          |
  +---------------------------------------------+
  * 其中无视口的 Drawables 所有级别都向下一移动层

  渲染栈会依次绘制所有的 Drawables。
  存在视口时，相应的内容绘制到视口上，否则绘制到屏幕上。
  视口内的 Drawables 绘制完毕时，视口退栈，其画面绘制到屏幕上。
  所有内容绘制完毕时，屏幕的内容绘制到窗口上。
  渲染未开始时，渲染栈只存在屏幕这一层。
  */

  /// @brief 缓存使用的图片大小基数
  static constexpr int cache_base_size = 48;

  /// @brief 缓存使用的图片大小每次膨胀的倍数
  static constexpr int cache_extend_ratio = 2;

  /// @brief 渲染器支持的最大 texture size
  static constexpr int max_texture_size = 32768;

  /// @brief 渲染栈，存放多层渲染结构对应的 texture
  std::vector<cen::texture> stack;

  /// @brief 缓存一定大小的 texture 避免重复申请内存或显存
  std::list<cen::texture> cache;

  /// @brief 渲染器的 handle，没有所有权
  cen::renderer_handle renderer;

  /// @brief 辅助计算返回不小于当前的长和宽的2的最小幂次
  /// @param width 图片的宽
  /// @param height 图片的高
  /// @return int 能放下这个图片的最小正方形 texture 的大小
  constexpr static int best_value(int width, int height) {
    int least_value = std::max(width, height);
    if (least_value > max_texture_size) {
      throw std::invalid_argument("Texture size must be less than 32768.");
    }
    int value = cache_base_size;
    while (value < least_value) {
      value *= cache_extend_ratio;
    }
    return value;
  }

  /// @brief 构造函数，不执行任何操作，初始化操作在 setup 里 */
  explicit renderstack() : stack(), cache(), renderer(nullptr) {}

  /// @brief 配置 renderstack，进行初始化操作
  /// @param renderer SDL2 渲染器的引用
  /// @param width 屏幕的宽度
  /// @param height 屏幕的高度
  void setup(cen::renderer& renderer, int width, int height) {
    /* 给 renderer_handle 成员赋值 */
    this->renderer = cen::renderer_handle(renderer);

    /*  创建缓存 cache，初始有 5 个 texture 在缓存中 */
    int size = cache_base_size;
    for (size_t i = 0; i < 5; ++i) {
      cen::texture empty = make_empty_texture(size, size);
      cache.push_back(std::move(empty));
      size *= cache_extend_ratio;
    }

    /* 创建与屏幕大小相同的 screen 并放入 stack 中 */
    cen::texture screen = make_empty_texture(width, height);
    screen.set_scale_mode(cen::scale_mode::nearest);
    stack.push_back(std::move(screen));
  }

  /// @brief 创建一个指定大小的空 texture
  /// @param width 图片的宽
  /// @param height 图片的高
  /// 创建空 texture 的通用方案
  cen::texture make_empty_texture(int width, int height) {
    cen::texture empty = renderer.make_texture(cen::iarea{width, height},
                                               cen::pixel_format::bgra32,
                                               cen::texture_access::target);
    empty.set_blend_mode(cen::blend_mode::none);

    /* 清空 texture 的内容 */
    renderer.set_target(empty);
    renderer.set_blend_mode(cen::blend_mode::none);
    renderer.clear_with(cen::colors::transparent);

    return empty;
  }

  /// @brief 将一个尺寸不小于 width x height 的 texture，添加到栈顶
  /// @param width texture 的最小宽度
  /// @param height texture 的最小高度
  void push_texture(int width, int height) {
    /* 优先从缓存中查找是否有满足条件的 texture */
    auto condition = [=](auto& x) {
      return x.width() >= width && x.height() >= height;
    };
    auto it = std::find_if(cache.begin(), cache.end(), condition);

    if (it != cache.end()) [[likely]] {
      stack.push_back(std::move(*it));
      cache.erase(it);
    } else [[unlikely]] {
      /* 少数的情况下，缓存中的 texture 都太小了，则创建一个足够大的 */
      const int size = best_value(width, height);
      cen::texture empty = make_empty_texture(size, size);
      stack.push_back(std::move(empty));
    }
  }

  /// @brief 将栈顶的 texture 放回到缓存里，并按照顺序排好
  void pop_texture() {
    int width = stack.back().width();
    int height = stack.back().height();
    auto condition = [=](auto& x) {
      return x.width() >= width && x.height() >= height;
    };
    auto it = std::find_if(cache.begin(), cache.end(), condition);
    cache.insert(it, std::move(stack.back()));
    stack.pop_back();
  }

  /// @brief 将一个尺寸不小于 width x height 的 texture 放到栈顶
  /// @param width texture 的最小宽度。
  /// @param height texture 的最小高度。
  /// 此 texture 的内容会被清空，且设置为渲染目标，并设置 clip 限制绘制区域。
  void push_empty_layer(int width, int height) {
    push_texture(width, height);

    renderer.set_target(current());
    renderer.set_clip(cen::irect(0, 0, width, height));
    renderer.set_blend_mode(cen::blend_mode::none);

    renderer.clear_with(cen::colors::transparent);
  }

  /// @brief 取出当前栈顶 texture 的特定区域作为新的 texture 放到栈顶
  /// @param x 区域左上角的横坐标
  /// @param y 区域左上角的纵坐标
  /// @param width 区域的宽
  /// @param height 区域的高
  /// 当前栈顶 texture 的此区域的内容会被绘制到新的 texture 中
  void push_capture_layer(int x, int y, int width, int height) {
    cen::texture& last = current();
    last.set_blend_mode(cen::blend_mode::none);

    push_empty_layer(width, height);
    renderer.render(last, cen::irect(x, y, width, height),
                    cen::irect(0, 0, width, height));
  }

  /// @brief 合并两个 texture 的方案的类，输入参数是 up 和 down 两个 texture
  /// 此方案的目标是把 up 的内容绘制到 down 上，方案通常是一个 lambda 表达式。
  using process_t = std::function<void(cen::texture& up, cen::texture& down)>;

  /// @brief 将栈顶的 texture 绘制到下一层，并移除栈顶的 texture。
  /// @param process 合并方案
  void merge(process_t process) {
    size_t depth = stack.size();

    /* 开发模式检查是否有 renderstack 的出入栈错误 */
    if constexpr (config::develop) {
      if (depth < 2) {
        cen::log_error("Merge failed, the stack depth is less than 2!");
        throw std::length_error{"renderstack in merge"};
      }
    }
    process(stack.back(), stack[depth - 2]);
    pop_texture();
  }

  /// @brief 将目标 texture 绘制到栈顶的 texture 之上。
  /// @param process 合并方案
  /// @param texture 目标 texture
  void merge(process_t process, cen::texture& texture) {
    if constexpr (config::develop) {
      if (stack.empty()) {
        cen::log_error("Merge failed, the stack is empty!");
        throw std::length_error{"renderstack in merge2"};
      }
    }
    process(texture, stack.back());
  }

  /// @brief 返回对栈顶的 texture 的引用
  cen::texture& current() { return stack.back(); }

  /// @brief 清空 stack 和 缓存的 textures
  void clear() {
    stack.clear();
    cache.clear();
  }
};

/// @brief 数据类 renderstack 相关的初始化类
/// @name task
struct init_renderstack {
  using data = std::tuple<renderstack>;

  static void before(auto& worker) {
    cen::renderer& renderer = RGMDATA(cen_library).renderer;
    renderstack& stack = RGMDATA(renderstack);

    /* stack 的底层大小正是 screen 的大小 */
    stack.setup(renderer, config::screen_width, config::screen_height);
  }

  static void after(auto& worker) { RGMDATA(renderstack).clear(); }
};
}  // namespace rgm::base