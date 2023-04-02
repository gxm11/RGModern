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

#pragma once
#include <list>
#include <vector>

#include "core/core.hpp"
#include "init_sdl2.hpp"

namespace rgm::base {
/**
 * @brief 管理渲染方式的数据结构。
 */
struct renderstack {
  /**
   * @brief 存放多级渲染结构对应 texture 的栈。
   * @note 可能的结构，从底到顶如下：
   * 1. window，实际上不在栈中
   * 2. screen
   * 3. viewport
   * 4. sprite
   */
  std::vector<cen::texture> stack;

  /** @brief 缓存一定大小的 texture 避免重复申请内存或显存 */
  std::list<cen::texture> cache;

  cen::renderer_handle renderer;

  /**
   * @brief 返回不小于当前的长和宽的2的最小幂次。
   *
   * @param width 图片的宽。
   * @param height 图片的长。
   * @return constexpr int 刚好能放下这个图片的最小 texture 的大小。
   */
  constexpr static int best_value(int width, int height) {
    int least_value = (width > height) ? width : height;
    if (least_value > 65535) {
      throw std::invalid_argument("Texture size must be less than 65535.");
    }
    int value = 32;
    while (value < least_value) {
      value *= 2;
    }
    return value;
  }

  /** @brief 因为构造时 SDL 还没有初始化，所以不执行任何操作 */
  explicit renderstack() : stack(), cache(), renderer(nullptr) {}

  /**
   * @brief 配置 renderstack
   *
   * @param renderer
   * @param window
   * @note 包括以下内容：
   * 1. 创建缓存 cache
   * 2. 创建与屏幕大小相同的 screen 并放入 stack 中
   */
  void setup(cen::renderer& renderer, cen::window& window) {
    this->renderer = cen::renderer_handle(renderer);

    for (int size : {48, 96, 192, 384, 768}) {
      cen::texture empty = make_empty_texture(size, size);
      cache.push_back(std::move(empty));
    }

    cen::texture screen = make_empty_texture(window.width(), window.height());
    screen.set_scale_mode(cen::scale_mode::nearest);
    stack.push_back(std::move(screen));
  }

  cen::texture make_empty_texture(int width, int height) {
    cen::texture empty = renderer.make_texture(cen::iarea{width, height},
                                               cen::pixel_format::bgra32,
                                               cen::texture_access::target);
    empty.set_blend_mode(cen::blend_mode::none);

    renderer.set_target(empty);
    renderer.set_blend_mode(cen::blend_mode::none);
    renderer.clear_with(cen::colors::transparent);

    // copy elision here
    return empty;
  }

  /**
   * @brief 从缓存中获取一个尺寸不小于 width x height 的 texture。
   *
   * @param width texture 的最小宽度。
   * @param height texture 的最小高度。
   * @return true 获取成功，并且放置此 texture 到 stack 的栈顶。
   * @return false 获取失败，需要创建新的 texture。
   */
  void push_texture(int width, int height) {
    auto condition = [=](auto& x) {
      return x.width() >= width && x.height() >= height;
    };
    auto it = std::find_if(cache.begin(), cache.end(), condition);
    if (it != cache.end()) {
      stack.push_back(std::move(*it));
      cache.erase(it);
    } else {
      const int size = best_value(width, height);
      cen::texture empty = make_empty_texture(size, size);
      stack.push_back(std::move(empty));
    }
  }

  /**
   * @brief 将栈顶的最后一个 texture 放回到缓存里
   *
   */
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

  /**
   * @brief 将一个尺寸不小于 width x height 的 texture 放到栈顶
   *
   * @param renderer
   * @param width texture 的最小宽度。
   * @param height texture 的最小高度。
   * @note texture 会被设置为渲染目标，其内容会被清空。
   */
  void push_empty_layer(int width, int height) {
    push_texture(width, height);

    renderer.set_target(current());
    renderer.set_clip(cen::irect(0, 0, width, height));
    renderer.set_blend_mode(cen::blend_mode::none);

    renderer.clear_with(cen::colors::transparent);
  }

  /**
   * @brief 取出当前栈顶 texture 的特定区域作为新的 texture 放到栈顶
   *
   * @param renderer
   * @param x 区域左上角的横坐标
   * @param y 区域左上角的纵坐标
   * @param width 区域的宽
   * @param height 区域的高
   */
  void push_capture_layer(int x, int y, int width, int height) {
    cen::texture& last = current();
    last.set_blend_mode(cen::blend_mode::none);

    push_empty_layer(width, height);
    renderer.render(last, cen::irect(x, y, width, height),
                    cen::irect(0, 0, width, height));
  }

  /** @brief 合并上下两个 texture 的方案 */
  using process_t = std::function<void(cen::texture& up, cen::texture& down)>;

  /**
   * @brief 将栈顶的 texture 绘制到下一层，并移除栈顶的 texture。
   *
   * @param process 绘制方案。
   */
  void merge(process_t process) {
    size_t depth = stack.size();
    if constexpr (config::develop) {
      if (depth < 2) {
        cen::log_error("Merge failed, the stack depth is less than 2!");
        throw std::length_error{"renderstack in merge"};
      }
    }
    process(stack.back(), stack[depth - 2]);
    pop_texture();
  }

  /**
   * @brief 将目标 texture 绘制到栈顶的 texture 之上。
   *
   * @param process 绘制方案。
   * @param texture 目标 texture
   */
  void merge(process_t process, cen::texture& texture) {
    if constexpr (config::develop) {
      if (stack.empty()) {
        cen::log_error("Merge failed, the stack is empty!");
        throw std::length_error{"renderstack in merge2"};
      }
    }
    process(texture, stack.back());
  }

  /** @brief 返回对栈顶的 texture 的引用 */
  cen::texture& current() { return stack.back(); }

  /** @brief 清空储存的内容 */
  void clear() {
    stack.clear();
    cache.clear();
  }
};

/** @brief 将 renderstack 类型的变量添加到 worker 的 datalist 中 */
struct init_renderstack {
  using data = std::tuple<renderstack>;

  static void before(auto& worker) {
    cen::renderer& renderer = RGMDATA(cen_library).renderer;
    cen::window& window = RGMDATA(cen_library).window;
    renderstack& stack = RGMDATA(renderstack);

    stack.setup(renderer, window);
  }

  static void after(auto& worker) { RGMDATA(renderstack).clear(); }
};
}  // namespace rgm::base