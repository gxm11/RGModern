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
#include "blend_type.hpp"
#include "builtin.hpp"
#include "ext/external.hpp"
#include "font.hpp"
#include "shader/shader.hpp"

namespace rgm::rmxp {
/*
 * RGSS 中的 Bitmap 对应 SDL 的纹理，即 cen::texture
 * 以下均是对 RGSS 中 Bitmap 各项功能的实现。
 * 在以下注释中，Bitmap 和 SDL 的纹理代表的是同一个概念。
 */

/// @brief 创建 Bitmap 的任务，有多种不同的特化方式。
/// @tparam size_t 创建方式
/// @see ./src/base/textures.hpp
/// 所有的纹理对象保存在 base::textures 中。
/// 对应于 RGSS 中的 Bitmap#initialize
template <size_t>
struct bitmap_create;

/// @brief 读取文件并创建 Bitmap 对象。
/// @tparam size_t=1 第 1 种特化
template <>
struct bitmap_create<1> {
  /// @brief Bitmap 的 ID
  uint64_t id;

  /// @brief 目标文件路径
  std::string_view path;

  void run(auto& worker) {
    cen::log_debug("[Bitmap] id = %lld, is created from %s", id, path.data());

    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    /*
     * 这里读取了 texture 后，又创建了一个空白的 target texture，把 texture
     * 绘制到空白 texture 上。这样做是因为从文件读取的 texture 不能作为
     * render target，从而不能调用 blt / set_pixel 等方法。
     */
    cen::texture texture = renderer.make_texture(path.data());

    /* 使用 base::renderstack::make_empty_texture 创建空白的 texture */
    cen::texture bitmap =
        stack.make_empty_texture(texture.width(), texture.height());

    texture.set_blend_mode(cen::blend_mode::none);
    renderer.set_target(bitmap);
    renderer.render(texture, cen::ipoint(0, 0));

    RGMDATA(base::textures).emplace(id, std::move(bitmap));

    /* 还原 target 为渲染栈的栈顶 */
    renderer.set_target(stack.current());
  }
};

/// @brief 创建固定大小的 Bitmap 对象
/// @tparam size_t=2 第 2 种特化
template <>
struct bitmap_create<2> {
  /// @brief Bitmap 的 ID
  uint64_t id;

  /// @brief Bitmap 的宽
  int width;

  /// @brief Bitmap 的高
  int height;

  void run(auto& worker) {
    cen::log_debug("[Bitmap] id = %lld, is created with area %d x %d", id,
                   width, height);

    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    /* 使用 base::renderstack::make_empty_texture 创建空白的 texture */
    cen::texture bitmap = stack.make_empty_texture(width, height);

    RGMDATA(base::textures).emplace(id, std::move(bitmap));

    /* 还原 target 为渲染栈的栈顶 */
    renderer.set_target(stack.current());
  }
};

/// @brief 从外部资源包中读取文件并创建 Bitmap 对象
/// @tparam size_t=3 第 3 种特化
template <>
struct bitmap_create<3> {
  /// @brief Bitmap 的 ID
  uint64_t id;

  /// @brief 目标文件路径
  std::string_view path;

  void run(auto& worker) {
    cen::log_debug("[Bitmap] id = %lld, is created from %s%s", id,
                   config::resource_prefix.data(), path.data());

    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);
    ext::zip_data_external& z = RGMDATA(ext::zip_data_external);

    auto opt = z.load_texture(path, renderer);
    /*
     * 使用 SDL_Texture* 作为构造参数，将转移所有权给此 cen::texture 对象
     * 局域变量 texture 将在此函数结束后自动析构。
     */
    if (!opt) {
      throw std::invalid_argument(
          "Failed to load SDL_Texture from external resource!");
    }

    cen::texture& texture = opt.value();

    /* 使用 base::renderstack::make_empty_texture 创建空白的 texture */
    cen::texture bitmap =
        stack.make_empty_texture(texture.width(), texture.height());

    texture.set_blend_mode(cen::blend_mode::none);
    renderer.set_target(bitmap);
    renderer.render(texture, cen::ipoint(0, 0));

    RGMDATA(base::textures).emplace(id, std::move(bitmap));

    /* 还原 target 为渲染栈的栈顶 */
    renderer.set_target(stack.current());
  }
};

/// @brief 释放指定 ID 的 Bitmap
/// 对应于 RGSS 中的 Bitmap#dispose
struct bitmap_dispose {
  /// @brief Bitmap 的 ID
  uint64_t id;

  void run(auto& worker) {
    cen::log_debug("[Bitmap] id = %lld, is disposed", id);

    /* 如果 id 不是合法的 id，则立刻返回 */
    if (id % base::counter::increament != 0) return;

    RGMDATA(base::textures).erase(id);

    /* 释放其他关联的 Bitmap，id + 1 是自动元件 */
    RGMDATA(base::textures).erase(id + 1);
  }
};

/// @brief 将一个 Bitmap 绘制到另一个 Bitmap 上
/// 对应于 RGSS 中的 Bitmap#blt
struct bitmap_blt {
  /// @brief 从源上截取的部分区域，作为绘制内容
  rect r;

  /// @brief Bitmap 的 ID，当前 Bitmap 是绘制的目标
  uint64_t id;

  /// @brief 作为源的 Bitmap 的 ID
  uint64_t src_id;

  /// @brief 绘制到当前 Bitmap 的对应区域的 X 坐标
  int x;

  /// @brief 绘制到当前 Bitmap 的对应区域的 Y 坐标
  int y;

  /// @brief 透明度。0 则相当于没有绘制效果，255 则相当于完全覆盖。
  int opacity;

  void run(auto& worker) {
    if (opacity <= 0) return;

    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    cen::texture& bitmap = RGMDATA(base::textures).at(id);
    cen::texture& src_bitmap = RGMDATA(base::textures).at(src_id);

    const cen::irect src_rect(r.x, r.y, r.width, r.height);
    const cen::irect dst_rect(x, y, r.width, r.height);

    src_bitmap.set_blend_mode(cen::blend_mode::blend);
    src_bitmap.set_alpha_mod(opacity);

    renderer.set_target(bitmap);
    renderer.render(src_bitmap, src_rect, dst_rect);

    /* 还原 target 为渲染栈的栈顶 */
    renderer.set_target(stack.current());
  }
};

/// @brief 将一个 Bitmap 带缩放地绘制到另一个 Bitmap 上
/// 对应于 RGSS 中的 Bitmap#stretch_blt
struct bitmap_stretch_blt {
  /// @brief 绘制的目标区域
  rect dst_r;

  /// @brief 从源上截取的部分区域，作为绘制内容
  rect src_r;

  /// @brief Bitmap 的 ID。当前 Bitmap 是绘制的目标
  uint64_t id;

  /// @brief 作为源的 Bitmap 的 ID
  uint64_t src_id;

  /// @brief 透明度。0 则相当于没有绘制效果，255 则相当于完全覆盖。
  int opacity;

  void run(auto& worker) {
    if (opacity <= 0) return;

    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    cen::texture& bitmap = RGMDATA(base::textures).at(id);
    cen::texture& src_bitmap = RGMDATA(base::textures).at(src_id);

    const cen::irect src_rect(src_r.x, src_r.y, src_r.width, src_r.height);
    const cen::irect dst_rect(dst_r.x, dst_r.y, dst_r.width, dst_r.height);

    src_bitmap.set_blend_mode(cen::blend_mode::blend);
    src_bitmap.set_alpha_mod(opacity);
    renderer.set_target(bitmap);
    renderer.render(src_bitmap, src_rect, dst_rect);

    /* 还原 target 为渲染栈的栈顶 */
    renderer.set_target(stack.current());
  }
};

/// @brief 将 Bitmap 中的矩形区域填充特定的颜色
/// 填充的颜色会无视透明度等限制，相当于直接改像素值。
/// 对应于 RGSS 中的 Bitmap#fill_rect
struct bitmap_fill_rect {
  /// @brief 要填充颜色的矩形区域
  rect r;

  /// @brief Bitmap 的 ID
  uint64_t id;

  /// @brief 用来填充的颜色
  color c;

  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    cen::texture& bitmap = RGMDATA(base::textures).at(id);

    renderer.set_target(bitmap);

    /* 此处混合模式使用 none 而不是 blend */
    renderer.set_blend_mode(cen::blend_mode::none);
    renderer.set_color(cen::color(c.red, c.green, c.blue, c.alpha));
    renderer.fill_rect(cen::irect(r.x, r.y, r.width, r.height));

    /* 还原 target 为渲染栈的栈顶 */
    renderer.set_target(stack.current());
  }
};

/// @brief bitmap_shader_helper
/// @tparam shader 的类型，目前有 shader_gray / shader_hue 两种可用
/// @tparam ...Args shader 的构造函数所需的参数类型
/// 此模板任务会调用 T_shader 修改 Bitmap 中所有的像素。
/// 使用方法见 bitmap_hue_change 和 bitmap_grayscale。
/// 此模板任务不在 Graphics.update 中被自动调用，而是立即生效。
/// @see ./src/shader/shader_base.hpp
template <typename T_shader, typename... Args>
struct bitmap_shader_helper {
  template <typename T_worker>
  static void apply(T_worker& worker, uint64_t bitmap_id, Args... args) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    cen::texture& bitmap = RGMDATA(base::textures).at(bitmap_id);

    /* 使用 base::renderstack::make_empty_texture 创建空白的 texture */
    cen::texture empty =
        stack.make_empty_texture(bitmap.width(), bitmap.height());

    /* 将原始 Bitmap 绘制到 empty 上 */
    bitmap.set_blend_mode(cen::blend_mode::none);
    bitmap.set_alpha_mod(255);
    renderer.set_target(empty);
    renderer.render(bitmap, cen::ipoint(0, 0));

    /* 将 empty 绘制到 Bitmap 上，并启用 shader_hue */
    renderer.set_target(bitmap);

    if (config::opengl) {
      /*
       * 如果不添加 GL_bind 和 unbind，对画面绘制没有影响，
       * 但在 hue_change 后立刻 save_png 会出现问题。
       * 实际上这里 bind empty 或者 bitmap 都无所谓。
       */
      SDL_GL_BindTexture(empty.get(), nullptr, nullptr);
      /* 构造 shader 对象 */
      T_shader shader(args...);

      renderer.render(empty, cen::ipoint(0, 0));
      SDL_GL_UnbindTexture(empty.get());
    } else {
      /* 构造 shader 对象 */
      T_shader shader(args...);

      renderer.render(empty, cen::ipoint(0, 0));
    }

    /* 还原 target 为渲染栈的栈顶 */
    renderer.set_target(stack.current());
  }
};

/// @brief 修改 Bitmap 的色相
/// 此任务会调用 shader_hue 修改 Bitmap 中所有的像素。
/// 对应于 RGSS 中的 Bitmap#hue_change
/// @see bitmap_shader_helper
struct bitmap_hue_change {
  /// @brief Bitmap 的 ID
  uint64_t id;

  /// @brief 色相的变化值，以 360 为周期
  int hue;

  void run(auto& worker) {
    if (hue % 360 == 0) return;

    bitmap_shader_helper<shader_hue, int>::apply(worker, id, hue);
  }
};

/// @brief 将 Bitmap 变成灰度图
/// 此任务会调用 shader_grayscale 修改 Bitmap 中所有的像素。
/// 对应于 RGSS 中的 Bitmap#grayscale
/// @see bitmap_shader_helper
struct bitmap_grayscale {
  uint64_t id;

  void run(auto& worker) {
    bitmap_shader_helper<shader_gray>::apply(worker, id);
  }
};

/// @brief 在 Bitmap 上绘制文字
/// 对应于 RGSS 中的 Bitmap#draw_text，但多了 3 个新特效：
/// 1. underlined，下划线
/// 2. strikethrough，删除线
/// 3. solid，使用更快速的（quick and dirty）绘制方式
struct bitmap_draw_text {
  /// @brief 绘制文字的目标区域
  rect r;

  /// @brief Bitmap 的 ID
  uint64_t id;

  /// @brief 要绘制的文字
  std::string_view text;

  /// @brief 文字的颜色
  color c;

  /// @brief 文字的字体 ID
  int font_id;

  /// @brief 文字的字体大小（字号）
  int font_size;

  /// @brief 文字的对齐模式，0：左对齐，1：居中，2：右对齐
  uint8_t align;

  /// @brief 是否启用加粗
  bool font_bold;

  /// @brief 是否启用斜体
  bool font_italic;

  /// @brief 是否启用下划线
  bool font_underlined;

  /// @brief 是否启用删除线
  bool font_strikethrough;

  /// @brief 是否启用 solid 风格
  bool font_solid;

  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    cen::font& font = RGMDATA(font_manager<false>).get(font_id, font_size);
    cen::texture& bitmap = RGMDATA(base::textures).at(id);

    /* 设置字体 */
    font.reset_style();
    font.set_bold(font_bold);
    font.set_italic(font_italic);
    font.set_underlined(font_underlined);
    font.set_strikethrough(font_strikethrough);

    /* 绘制文字，绘制的结果是一个 cen::surface */
    std::unique_ptr<cen::surface> ptr;
    if (font_solid) {
      ptr = std::make_unique<cen::surface>(font.render_solid_utf8(
          text.data(), cen::color(c.red, c.green, c.blue, c.alpha)));
    } else {
      ptr = std::make_unique<cen::surface>(font.render_blended_utf8(
          text.data(), cen::color(c.red, c.green, c.blue, c.alpha)));
    }
    if (!ptr) return;

    cen::texture texture = renderer.make_texture(*ptr);

    /* 根据对齐方式，设置文字绘制到 Bitmap 上的位置 */
    int x = r.x;
    int y = r.y;
    int width = r.width;
    int height = r.height;

    int _width = ptr->width();
    int _height = ptr->height();

    if (_height < height) {
      y += (height - _height) / 2;
      height = _height;
    } else {
      _height = height;
    }
    if (_width < width) {
      switch (align) {
        case 0:
        default:
          /* 左对齐 */
          break;
        case 1:
          /* 居中 */
          x += (width - _width) / 2;
          break;
        case 2:
          /* 右对齐 */
          x += width - _width;
          break;
      }
      width = _width;
    } else {
      /* 当文字超过了预设的宽度时，将文字缩小到原来的 60% 大小 */
      _width = width * 5 / 3;
    }

    texture.set_blend_mode(cen::blend_mode::blend);

    renderer.set_target(bitmap);
    renderer.render(texture, cen::irect(0, 0, _width, _height),
                    cen::irect(x, y, width, height));

    /* 还原字体对象 */
    font.reset_style();

    /* 还原 target 为渲染栈的栈顶 */
    renderer.set_target(stack.current());
  }
};

/// @brief 获取指定位置的像素值
/// 对应于 RGSS 中的 Bitmap#get_pixel，这是一个同步函数。
/// 这个方法通常会非常慢，不建议经常使用，请使用 Palette 类替代
/// 目前 opengl 和 direct3d9 测试通过，但 d3d11 仍然存在问题。
/// 参见：https://github.com/libsdl-org/SDL/issues/4782
struct bitmap_get_pixel {
  /// @brief Bitmap 的 ID
  uint64_t id;

  /// @brief 像素点的 X 坐标
  int x;

  /// @brief 像素点的 Y 坐标
  int y;

  /// @brief 用来存储 Pixels 的指针
  uint8_t* p_pixel;

  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    cen::texture& bitmap = RGMDATA(base::textures).at(id);

    renderer.set_target(bitmap);
    /* 一次性可以获取目标矩形里的全部像素值，但是这里矩形的长和宽都是 1 */
    SDL_Rect rect{x, y, 1, 1};
    SDL_RenderReadPixels(renderer.get(), &rect,
                         static_cast<uint32_t>(config::texture_format), p_pixel,
                         bitmap.width() * 4);

    /* 还原 target 为渲染栈的栈顶 */
    renderer.set_target(stack.current());
  }
};

/// @brief 将 Bitmap 保存到文件
/// RGSS 中没有对应的函数。此方法可能会比较慢，不建议经常使用。
struct bitmap_save_png {
  /// @brief Bitmap 的 ID
  uint64_t id;

  /// @brief 存储的文件路径
  std::string_view path;

  void run(auto& worker) {
    cen::log_warn("[Bitmap] id = %lld, is saved to %s", id, path);

    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    cen::texture& bitmap = RGMDATA(base::textures).at(id);

    /*
     * 在 renderer.present 之后的第一次 save_png 会导致失败。
     * 实际上 texture是正常的，只是 save_png 会导致这个 texture 出问题。
     * 但期间如果执行了一次绘制指令就没问题。
     * 这样考虑的话，可以创建一个新的texture，绘制上去再save_png。
     */

    /* 使用 base::renderstack::make_empty_texture 创建空白的 texture */
    cen::texture empty =
        stack.make_empty_texture(bitmap.width(), bitmap.height());

    /* 将原始 Bitmap 绘制到 empty 上 */
    bitmap.set_blend_mode(cen::blend_mode::none);
    bitmap.set_alpha_mod(255);
    renderer.set_target(empty);
    renderer.render(bitmap, cen::ipoint(0, 0));

    /* 截取并存储 */
    renderer.capture(config::texture_format).save_as_png(path.data());

    /* 还原 target 为渲染栈的栈顶 */
    renderer.set_target(stack.current());
  }
};

/// @brief 将当前屏幕的内容绘制到此 Bitmap 上
/// RGSS 中没有对应的函数。
/// 此方法用来实现截屏。实际上绘制的是上一帧的内容。这符合 RGSS 的设计。
/// RGSS 中只在 Graphics.update 的时候画面才会变化，所以截屏始终是上一帧。
struct bitmap_capture_screen {
  /// @brief Bitmap 的 ID
  uint64_t id;

  void run(auto& worker) {
    cen::log_debug("[Bitmap] id = %lld, is created from screen capturing", id);

    base::renderstack& stack = RGMDATA(base::renderstack);
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;

    /* 开发模式检查是否有 renderstack 的出入栈错误 */
    if constexpr (config::develop) {
      if (stack.stack.size() != 1) {
        cen::log_error(
            "capture screen failed, the stack depth is not equal to 1!");
        throw std::length_error{"renderstack in bitmap capture screen"};
      }
    }

    cen::texture& bitmap = RGMDATA(base::textures).at(id);

    /* 注意这里是 stack.current()，也就是上一帧绘制的内容 */
    renderer.set_target(bitmap);
    renderer.set_blend_mode(cen::blend_mode::none);
    renderer.render(
        stack.current(),
        cen::irect{0, 0, stack.current().width(), stack.current().height()},
        cen::irect{0, 0, bitmap.width(), bitmap.height()});

    /* 还原 target 为渲染栈的栈顶 */
    renderer.set_target(stack.current());
  }
};

/// @brief 将一个调色盘对象（Palette）的内容绘制到 Bitmap 的左上角
/// RGSS 中没有对应的函数。此函数在 Palette#convert_to_bitmap 中使用。
struct bitmap_capture_palette {
  /// @brief Bitmap 的 ID
  uint64_t id;

  /// @brief Palette 对象关联的 surface 的一个副本
  std::unique_ptr<cen::surface> ptr;

  void run(auto& worker) {
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    cen::texture& bitmap = RGMDATA(base::textures).at(id);

    cen::texture texture = renderer.make_texture(*ptr);

    renderer.set_target(bitmap);
    texture.set_blend_mode(cen::blend_mode::none);
    renderer.render(texture, cen::ipoint(0, 0));

    /* 还原 target 为渲染栈的栈顶 */
    renderer.set_target(stack.current());
  }
};

/// @brief 根据原始的 Bitmap，展开自动元件图形为新的 Bitmap
/// 新的 Bitmap 会放置到 id + 1 的位置存储。
/// 新的 Bitmap 的创建是在Graphics.update中，tilemap << VALUE 时触发的。
/// @see ./src/rmxp/graphics.hpp
/// XP 的自动元件图有 2 种格式：
/// 1. 32 高度，此自动元件不存在拼接带来的变化，始终显示相同的内容
/// 2. 128 高度，此自动元件会根据周围是否有此元件改变显示的内容，即 tileid。
/// 此处参考了 gouki04 的文章：
/// 版权声明：本文为CSDN博主「gouki04」的原创文章，
/// 遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
/// 原文链接：https://blog.csdn.net/gouki04/article/details/7107088
struct bitmap_make_autotile {
  /// @brief 自动元件原始图片 Bitmap 的 ID
  uint64_t id;

  /// @brief 自动元件映射关系表
  static constexpr uint32_t autotile_map[48] = {
      0x1a1b2021, 0x041b2021, 0x1a052021, 0x04052021, 0x1a1b200b, 0x041b200b,
      0x1a05200b, 0x0405200b, 0x1a1b0a21, 0x041b0a21, 0x1a050a21, 0x04050a21,
      0x1a1b0a0b, 0x041b0a0b, 0x1a050a0b, 0x04050a0b, 0x18191e1f, 0x18051e1f,
      0x18191e0b, 0x18051e0b, 0x0e0f1415, 0x0e0f140b, 0x0e0f0a15, 0x0e0f0a0b,
      0x1c1d2223, 0x1c1d0a23, 0x041d2223, 0x041d0a23, 0x1a1b2c2d, 0x04272c2d,
      0x26052c2d, 0x04052c2d, 0x181d1e23, 0x0e0f2c2d, 0x0c0d1213, 0x0c0d120b,
      0x10111617, 0x10110a17, 0x28292e2f, 0x04292e2f, 0x24252a2b, 0x24052a2b,
      0x0c111217, 0x0c0d2a2b, 0x24292a2f, 0x10112e2f, 0x0c112a2f, 0x0c112a2f};

  void run(auto& worker) {
    cen::log_debug("[Bitmap] id = %lld, is converted to autotile format", id);

    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);
    base::textures& textures = RGMDATA(base::textures);

    cen::texture& source = textures.at(id);

    /* 移除已经存在的自动元件，重新绘制 */
    textures.erase(id + 1);

    /* 如果自动元件的格式不正确，则补全成正确的格式 */
    int height = source.height();
    int width = source.width();
    if (height <= 32) {
      height = 32;
      width = width - (width % 32);
    } else {
      height = 128;
      width = width - (width % 96);
    }

    /* 用透明像素将 autotile 的补全成正确格式后，保存到 temp 中 */
    /* 使用 base::renderstack::make_empty_texture 创建空白的 texture */
    cen::texture temp = stack.make_empty_texture(width, height);
    renderer.set_target(temp);
    source.set_blend_mode(cen::blend_mode::none);
    renderer.render(source, cen::ipoint(0, 0));

    /*
     * RGM 中 autotile 的布局，其 X 方向变化表示的是随帧数改变的动画，
     * 其 Y 方向变化表示的是因为周围有其他自动元件导致的画面变化
     * 所以 Y 方向高度是固定的 32 或 48x32。
     */

    /* 对于 height == 32 的图块来说正好不用改动。*/
    if (height == 32) {
      /* 将 autotile 存储到 id + 1 的位置 */
      textures.emplace(id + 1, std::move(temp));
      /* 还原 target 为渲染栈的栈顶 */
      renderer.set_target(stack.current());
      return;
    }

    /* 对于 height = 128，x 轴长度为 width / 3，y 轴长度为 48x32 */
    /* 使用 base::renderstack::make_empty_texture 创建空白的 texture */
    cen::texture autotile = stack.make_empty_texture(width / 3, 48 * 32);
    renderer.set_target(autotile);
    temp.set_blend_mode(cen::blend_mode::none);

    cen::irect src_rect(0, 0, 16, 16);
    cen::irect dst_rect(0, 0, 16, 16);

    /* 查表绘制 autotile 的内容 */
    int sx, sy, dx, dy, index;
    for (int i = 0; i < width / 96; ++i) {
      for (int j = 0; j < 48; ++j) {
        for (int k = 0; k < 4; ++k) {
          index = (autotile_map[j] >> (24 - 8 * k)) & 255;
          sx = i * 96 + (index % 6) * 16;
          sy = index / 6 * 16;
          dx = i * 32 + ((k & 1) ? 16 : 0);
          dy = j * 32 + ((k & 2) ? 16 : 0);

          src_rect.set_position(sx, sy);
          dst_rect.set_position(dx, dy);
          renderer.render(temp, src_rect, dst_rect);
        }
      }
    }

    /* 将 autotile 存储到 id + 1 的位置 */
    textures.emplace(id + 1, std::move(autotile));

    /* 还原 target 为渲染栈的栈顶 */
    renderer.set_target(stack.current());
  }
};

/// @brief 重新加载自动元件
/// RGSS 中没有对应的函数。在 RPG::Cache.reload 中使用，
/// 重新加载 Bitmap 后，相应的自动元件也要重新绘制。
struct bitmap_reload_autotile {
  /// @brief 储存原始文件的 Bitmap 的 ID
  uint64_t id;

  void run(auto& worker) {
    base::textures& textures = RGMDATA(base::textures);

    if (textures.find(id + 1) != textures.end()) {
      worker >> bitmap_make_autotile{id};
    }
  }
};

/// @brief Ruby 中 Bitmap 类的初始化类，定义了大量的操作函数。
struct init_bitmap {
  static void before(auto& this_worker) {
    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /* ruby method: Bitmap#new -> bitmap_create */
      static VALUE create(VALUE, VALUE id_, VALUE width_, VALUE height_) {
        RGMLOAD(id, uint64_t);

        if (height_ == Qnil) {
          RGMLOAD2(path, std::string_view, width_);
          if (path.starts_with(config::resource_prefix)) {
            /* 移除开头的 config::resource_prefix */
            std::string_view path2 =
                path.substr(config::resource_prefix.size());
            worker >> bitmap_create<3>{id, path2};
          } else {
            worker >> bitmap_create<1>{id, path};
          }
        } else {
          RGMLOAD(width, int);
          RGMLOAD(height, int);

          worker >> bitmap_create<2>{id, width, height};
        }
        return Qnil;
      }

      /* ruby method: Bitmap#blt -> bitmap_blt */
      static VALUE blt(VALUE, VALUE id_, VALUE x_, VALUE y_, VALUE src_id_,
                       VALUE rect_, VALUE opacity_) {
        RGMLOAD(id, uint64_t);
        RGMLOAD(src_id, uint64_t);
        RGMLOAD(x, int);
        RGMLOAD(y, int);
        RGMLOAD(opacity, int);

        rect r;
        r << rect_;

        worker >> bitmap_blt{r, id, src_id, x, y, opacity};
        return Qnil;
      }

      /* ruby method: Bitmap#stretch_blt -> bitmap_stretch_blt */
      static VALUE stretch_blt(VALUE, VALUE id_, VALUE dst_rect_, VALUE src_id_,
                               VALUE src_rect_, VALUE opacity_) {
        RGMLOAD(id, uint64_t);
        RGMLOAD(src_id, uint64_t);
        RGMLOAD(opacity, int);

        rect dst_r;
        dst_r << dst_rect_;
        rect src_r;
        src_r << src_rect_;

        worker >> bitmap_stretch_blt{dst_r, src_r, id, src_id, opacity};
        return Qnil;
      }

      /* ruby method: Bitmap#fill_rect -> bitmap_fill_rect */
      static VALUE fill_rect(VALUE, VALUE id_, VALUE rect_, VALUE color_) {
        RGMLOAD(id, uint64_t);

        rect r;
        r << rect_;
        color c;
        c << color_;

        worker >> bitmap_fill_rect{r, id, c};
        return Qnil;
      }

      /* ruby method: Bitmap#text_size -> bitmap_text_size */
      static VALUE text_size(VALUE, VALUE font_, VALUE text_) {
        RGMLOAD(text, const char*);

        int id = detail::get<word::id, int>(font_);
        int size = detail::get<word::size, int>(font_);

        font_manager<true>& fonts = RGMDATA(font_manager<true>);
        cen::font& font = fonts.get(id, size);

        int width = 0;
        int height = 0;
        if (TTF_SizeUTF8(font.get(), text, &width, &height) == 0) {
          /* 一般情况下这两个数都不会太大，以防万一还是夹一下 */
          width = std::clamp(width, 0, 65535);
          height = std::clamp(height, 0, 65535);

          /* 将高和宽打包到一起传过去 */
          return INT2FIX(height * 65536 + width);
        }

        /* 如果 TTF_SizeUTF8 执行失败则返回 0 */
        return INT2FIX(0);
      }

      /* ruby method: Bitmap#draw_text -> bitmap_draw_text */
      static VALUE draw_text(VALUE, VALUE id_, VALUE font_, VALUE rect_,
                             VALUE text_, VALUE align_) {
        RGMLOAD(id, uint64_t);
        RGMLOAD(align, uint8_t);
        RGMLOAD(text, std::string_view);

        if (text.size() == 0) return Qnil;

        color c;
        c << detail::get<word::color>(font_);
        rect r;
        r << rect_;

        int font_id = detail::get<word::id, int>(font_);
        int font_size = detail::get<word::size, int>(font_);
        bool font_bold = detail::get<word::bold, bool>(font_);
        bool font_italic = detail::get<word::italic, bool>(font_);
        bool font_underlined = detail::get<word::underlined, bool>(font_);
        bool font_strikethrough = detail::get<word::strikethrough, bool>(font_);
        bool font_solid = detail::get<word::solid, bool>(font_);

        worker >> bitmap_draw_text{r,
                                   id,
                                   text,
                                   c,
                                   font_id,
                                   font_size,
                                   align,
                                   font_bold,
                                   font_italic,
                                   font_underlined,
                                   font_strikethrough,
                                   font_solid};
        return Qnil;
      }

      /* ruby method: Bitmap#get_pixel -> bitmap_get_pixel */
      static VALUE get_pixel(VALUE, VALUE id_, VALUE x_, VALUE y_) {
        RGMLOAD(id, uint64_t);
        RGMLOAD(x, int);
        RGMLOAD(y, int);

        std::array<uint8_t, 4> pixels{};
        worker >> bitmap_get_pixel{id, x, y, pixels.data()};

        RGMWAIT(1);

        /*
         * 获取的 pixels 是 bgra 格式，此处转换成 rgba 格式以适配 RGSS。
         * 转换后的值可能超出了 FIXNUM 的范围，故使用 UINT2NUM 宏。
         */
        uint32_t color = (pixels[3] << 24) + (pixels[2]) + (pixels[1] << 8) +
                         (pixels[0] << 16);
        return UINT2NUM(color);
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "bitmap_create", wrapper::create,
                              3);
    rb_define_module_function(rb_mRGM_Base, "bitmap_blt", wrapper::blt, 6);
    rb_define_module_function(rb_mRGM_Base, "bitmap_stretch_blt",
                              wrapper::stretch_blt, 5);
    rb_define_module_function(rb_mRGM_Base, "bitmap_fill_rect",
                              wrapper::fill_rect, 3);
    rb_define_module_function(rb_mRGM_Base, "bitmap_draw_text",
                              wrapper::draw_text, 5);
    rb_define_module_function(rb_mRGM_Base, "bitmap_text_size",
                              wrapper::text_size, 2);
    rb_define_module_function(rb_mRGM_Base, "bitmap_get_pixel",
                              wrapper::get_pixel, 3);

    RGMBIND(rb_mRGM_Base, "bitmap_dispose", bitmap_dispose, 1);
    RGMBIND(rb_mRGM_Base, "bitmap_hue_change", bitmap_hue_change, 2);
    RGMBIND(rb_mRGM_Base, "bitmap_grayscale", bitmap_grayscale, 1);
    RGMBIND(rb_mRGM_Base, "bitmap_save_png", bitmap_save_png, 2);
    RGMBIND(rb_mRGM_Base, "bitmap_capture_screen", bitmap_capture_screen, 1);
    RGMBIND(rb_mRGM_Base, "bitmap_reload_autotile", bitmap_reload_autotile, 1);
  }
};
}  // namespace rgm::rmxp