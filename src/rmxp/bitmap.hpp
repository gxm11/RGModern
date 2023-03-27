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
#include "base/base.hpp"
#include "blend_type.hpp"
#include "builtin.hpp"
#include "font.hpp"
#include "shader/shader.hpp"
#include "zip.hpp"

namespace rgm::rmxp {
/**
 * @brief 对应于 RGSS 中的 Bitmap 创建 SDL 纹理，纹理对象保存在 textures 中。
 *
 * @tparam size_t 创建方式
 */
template <size_t>
struct bitmap_create;

/**
 * @brief 任务：读取文件并创建 SDL 纹理。
 *
 * @tparam size_t=1 第 1 种特化
 */
template <>
struct bitmap_create<1> {
  /** @brief Bitmap 的 ID */
  uint64_t id;
  /** @brief 目标文件路径 */
  const char* path;

  void run(auto& worker) {
    cen::log_info(cen::log_category::render,
                  "[Bitmap] id = %lld, is created from %s", id, path);

    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    cen::texture texture = renderer.make_texture(path);
    cen::texture bitmap =
        stack.make_empty_texture(texture.width(), texture.height());

    texture.set_blend_mode(cen::blend_mode::none);
    renderer.set_target(bitmap);
    renderer.render(texture, cen::ipoint(0, 0));

    RGMDATA(base::textures).emplace(id, std::move(bitmap));
    renderer.set_target(stack.current());
  }
};

/**
 * @brief 任务：创建固定大小的空 SDL 纹理。
 *
 * @tparam size_t=2 第 2 种特化
 */
template <>
struct bitmap_create<2> {
  /** @brief Bitmap 的 ID */
  uint64_t id;
  /** @brief Bitmap 的宽 */
  int width;
  /** @brief Bitmap 的高 */
  int height;

  void run(auto& worker) {
    cen::log_debug(cen::log_category::render,
                   "[Bitmap] id = %lld, is created with area %d x %d", id,
                   width, height);

    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    cen::texture bitmap = stack.make_empty_texture(width, height);
    RGMDATA(base::textures).emplace(id, std::move(bitmap));
    renderer.set_target(stack.current());
  }
};

/**
 * @brief 任务：根据特定的Bitmap，创建自动元件图形
 *
 * @tparam size_t=3 第 3 种特化
 */
template <>
struct bitmap_create<3> {
  uint64_t id;

  // 版权声明：本文为CSDN博主「gouki04」的原创文章，
  // 遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
  // 原文链接：https://blog.csdn.net/gouki04/article/details/7107088
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
    cen::log_debug(cen::log_category::render,
                   "[Bitmap] id = %lld, is converted to autotile format", id);

    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);
    cen::texture& source = RGMDATA(base::textures).at(id);

    int height = source.height();
    int width = source.width();
    if (height <= 32) {
      height = 32;
      width = width - (width % 32);
    } else {
      height = 128;
      width = width - (width % 96);
    }
    // temp 将 autotile 的长宽对齐
    cen::texture temp = stack.make_empty_texture(width, height);
    renderer.set_target(temp);
    source.set_blend_mode(cen::blend_mode::none);
    renderer.render(source, cen::ipoint(0, 0));

    // autotile，帧数的变化体现在 x 轴，不同tileid对应的图片体现在 y 轴
    // 对于 height == 32 的图块来说正好不用改动。
    if (height == 32) {
      RGMDATA(base::textures).emplace(id + 1, std::move(temp));
      renderer.set_target(stack.current());
      return;
    }
    // 对于 height = 128，x 轴长度为 width / 3，y 轴长度为 48 * 32
    cen::texture autotile = stack.make_empty_texture(width / 3, 48 * 32);
    renderer.set_target(autotile);
    temp.set_blend_mode(cen::blend_mode::none);

    cen::irect src_rect(0, 0, 16, 16);
    cen::irect dst_rect(0, 0, 16, 16);

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
    RGMDATA(base::textures).emplace(id + 1, std::move(autotile));
    // autotile的创建是在Graphics.update中，tilemap << VALUE 时触发的
    // 从而需要还原 renderer target，否则会导致后面的绘制出错。
    renderer.set_target(stack.current());
  }
};

/**
 * @brief 任务：从资源文件中读取文件并创建 SDL 纹理。
 *
 * @tparam size_t=4 第 4 种特化
 */
template <>
struct bitmap_create<4> {
  /** @brief Bitmap 的 ID */
  uint64_t id;
  /** @brief 目标文件路径 */
  const char* path;

  void run(auto& worker) {
    cen::log_info(cen::log_category::render,
                  "[Bitmap] id = %lld, is created from external://%s", id,
                  path);

    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);
    zip_data_external& z = RGMDATA(zip_data_external);

    SDL_Texture* ptr = z.load_texture(path, renderer);
    cen::texture texture{ptr};

    cen::texture bitmap =
        stack.make_empty_texture(texture.width(), texture.height());

    texture.set_blend_mode(cen::blend_mode::none);
    renderer.set_target(bitmap);
    renderer.render(texture, cen::ipoint(0, 0));

    RGMDATA(base::textures).emplace(id, std::move(bitmap));
    renderer.set_target(stack.current());
  }
};

/**
 * @brief 任务：释放指定 ID 的 Bitmap
 */
struct bitmap_dispose {
  /** @brief Bitmap 的 ID */
  uint64_t id;

  void run(auto& worker) {
    cen::log_debug(cen::log_category::render, "[Bitmap] id = %lld, is disposed",
                   id);

    RGMDATA(base::textures).erase(id);
    // +1 for autotiles
    RGMDATA(base::textures).erase(id + 1);
  }
};

struct bitmap_blt {
  rect r;
  uint64_t id;
  uint64_t src_id;
  int x;
  int y;
  int opacity;

  void run(auto& worker) {
    cen::texture& bitmap = RGMDATA(base::textures).at(id);
    cen::texture& src_bitmap = RGMDATA(base::textures).at(src_id);
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    const cen::irect src_rect(r.x, r.y, r.width, r.height);
    const cen::irect dst_rect(x, y, r.width, r.height);

    src_bitmap.set_blend_mode(cen::blend_mode::blend);
    src_bitmap.set_alpha_mod(opacity);
    renderer.set_target(bitmap);
    renderer.render(src_bitmap, src_rect, dst_rect);
    renderer.set_target(stack.current());
  }
};

struct bitmap_stretch_blt {
  rect dst_r;
  rect src_r;
  uint64_t id;
  uint64_t src_id;
  int opacity;

  void run(auto& worker) {
    cen::texture& bitmap = RGMDATA(base::textures).at(id);
    cen::texture& src_bitmap = RGMDATA(base::textures).at(src_id);
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    const cen::irect src_rect(src_r.x, src_r.y, src_r.width, src_r.height);
    const cen::irect dst_rect(dst_r.x, dst_r.y, dst_r.width, dst_r.height);

    src_bitmap.set_blend_mode(cen::blend_mode::blend);
    src_bitmap.set_alpha_mod(opacity);
    renderer.set_target(bitmap);
    renderer.render(src_bitmap, src_rect, dst_rect);
    renderer.set_target(stack.current());
  }
};

struct bitmap_fill_rect {
  rect r;
  uint64_t id;
  color c;

  void run(auto& worker) {
    cen::texture& bitmap = RGMDATA(base::textures).at(id);
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    renderer.set_target(bitmap);
    renderer.set_blend_mode(cen::blend_mode::none);
    renderer.set_color(cen::color(c.red, c.green, c.blue, c.alpha));
    renderer.fill_rect(cen::irect(r.x, r.y, r.width, r.height));
    renderer.set_target(stack.current());
  }
};

struct bitmap_hue_change {
  uint64_t id;
  int hue;

  void run(auto& worker) {
    if (hue % 360 == 0) return;

    cen::texture& bitmap = RGMDATA(base::textures).at(id);
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    cen::texture empty =
        stack.make_empty_texture(bitmap.width(), bitmap.height());

    bitmap.set_blend_mode(cen::blend_mode::none);
    bitmap.set_alpha_mod(255);
    renderer.set_target(empty);
    renderer.render(bitmap, cen::ipoint(0, 0));

    renderer.set_target(bitmap);

    // 如果不添加 GL_bind 和 unbind，对画面绘制没有影响，
    // 但在 hue_change 后立刻 save_png 会出现问题。
    constexpr bool enable_bind = true;
    if (enable_bind && shader::driver == shader::opengl) {
      SDL_GL_BindTexture(empty.get(), nullptr, nullptr);
      shader_hue shader(hue);

      renderer.render(empty, cen::ipoint(0, 0));
      SDL_GL_UnbindTexture(empty.get());
    } else {
      shader_hue shader(hue);

      renderer.render(empty, cen::ipoint(0, 0));
    }
    renderer.set_target(stack.current());
  }
};

struct bitmap_draw_text {
  rect r;
  uint64_t id;
  const char* text;
  color c;
  int font_id;
  int font_size;
  uint8_t align;
  bool font_bold;
  bool font_italic;
  bool font_underlined;
  bool font_strikethrough;
  bool font_solid;

  void run(auto& worker) {
    cen::texture& bitmap = RGMDATA(base::textures).at(id);
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    cen::font& font = RGMDATA(font_manager<false>).get(font_id, font_size);
    base::renderstack& stack = RGMDATA(base::renderstack);

    // set font
    font.reset_style();
    font.set_bold(font_bold);
    font.set_italic(font_italic);
    font.set_underlined(font_underlined);
    font.set_strikethrough(font_strikethrough);

    std::unique_ptr<cen::surface> ptr;
    if (font_solid) {
      ptr = std::make_unique<cen::surface>(font.render_solid_utf8(
          text, cen::color(c.red, c.green, c.blue, c.alpha)));
    } else {
      ptr = std::make_unique<cen::surface>(font.render_blended_utf8(
          text, cen::color(c.red, c.green, c.blue, c.alpha)));
    }
    const cen::surface& s = *ptr;

    int _height = s.height();
    int _width = s.width();
    int x = r.x;
    int y = r.y;
    int width = r.width;
    int height = r.height;

    if (_height < height) {
      y += (height - _height) / 2;
      height = _height;
    } else {
      _height = height;
    }
    if (_width < width) {
      switch (align) {
        case 0:
        default:  // left
          break;
        case 1:  // center
          x += (width - _width) / 2;
          break;
        case 2:  // right
          x += width - _width;
          break;
      }
      width = _width;
    } else {
      _width = width * 5 / 3;
    }

    cen::texture texture = renderer.make_texture(s);
    texture.set_blend_mode(cen::blend_mode::blend);

    renderer.set_target(bitmap);
    renderer.render(texture, cen::irect(0, 0, _width, _height),
                    cen::irect(x, y, width, height));
    font.reset_style();
    renderer.set_target(stack.current());
  }
};

struct bitmap_get_pixel {
  uint64_t id;
  int x;
  int y;
  uint8_t* p_pixel;

  void run(auto& worker) {
    cen::texture& bitmap = RGMDATA(base::textures).at(id);
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    renderer.set_target(bitmap);
    SDL_Rect rect{x, y, 1, 1};
    SDL_RenderReadPixels(renderer.get(), &rect,
                         static_cast<uint32_t>(cen::pixel_format::bgra32),
                         p_pixel, bitmap.width() * 4);
    renderer.set_target(stack.current());
  }
};

struct bitmap_save_png {
  uint64_t id;
  const char* path;

  void run(auto& worker) {
    cen::log_info(cen::log_category::render,
                  "[Bitmap] id = %lld, is saved to %s", id, path);

    cen::texture& bitmap = RGMDATA(base::textures).at(id);
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    // 在 renderer.present 之后的第一次 save_png 会导致失败。
    // 实际上 texture是正常的，只是 save_png 会导致这个 texture 出问题。
    // 但期间如果执行了一次绘制指令就没问题。
    // 这样考虑的话，可以创建一个新的texture，绘制上去再save_png。
    cen::texture target =
        stack.make_empty_texture(bitmap.width(), bitmap.height());
    renderer.set_target(target);
    bitmap.set_blend_mode(cen::blend_mode::none);
    renderer.render(bitmap, cen::ipoint(0, 0));

    renderer.capture(cen::pixel_format::bgra32).save_as_png(path);
    renderer.set_target(stack.current());
  }
};

struct bitmap_capture_screen {
  uint64_t id;

  void run(auto& worker) {
    cen::log_debug(cen::log_category::render,
                   "[Bitmap] id = %lld, is created from screen capturing", id);

    base::renderstack& stack = RGMDATA(base::renderstack);
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    if constexpr (config::check_renderstack) {
      if (stack.stack.size() != 1) {
        cen::log_error(
            "capture screen failed, the stack depth is not equal to 1!");
        throw std::length_error{"renderstack in bitmap capture screen"};
      }
    }
    cen::texture& bitmap = RGMDATA(base::textures).at(id);
    renderer.set_target(bitmap);
    renderer.set_blend_mode(cen::blend_mode::none);
    renderer.render(stack.current(), cen::ipoint(0, 0));
    renderer.set_target(stack.current());
  }
};

struct bitmap_capture_palette {
  uint64_t id;
  std::unique_ptr<cen::surface> ptr;

  void run(auto& worker) {
    cen::texture& bitmap = RGMDATA(base::textures).at(id);
    cen::renderer& renderer = RGMDATA(base::cen_library).renderer;
    base::renderstack& stack = RGMDATA(base::renderstack);

    cen::texture texture = renderer.make_texture(*ptr);

    renderer.set_target(bitmap);
    texture.set_blend_mode(cen::blend_mode::none);
    renderer.render(texture, cen::ipoint(0, 0));
    // renderer.reset_target();
    renderer.set_target(stack.current());
  }
};

/**
 * @brief 在 ruby 中定义操作 Bitmap 的相关函数。
 */
struct init_bitmap {
  static void before(auto& this_worker) {
    static decltype(auto) worker = this_worker;

    /** wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /** RGM::Base.bitmap_create 方法 */
      static VALUE create(VALUE, VALUE id_, VALUE width_, VALUE height_) {
        RGMLOAD(id, uint64_t);

        if (height_ == Qnil) {
          RGMLOAD2(path, const char*, width_);

          int ret = strncmp(config::resource_prefix.data(), path,
                            config::resource_prefix.size());
          if (ret == 0) {
            // 增加指针的值，相当于截取字符串的后半部分。
            const char* path2 = path + config::resource_prefix.size();
            worker >> bitmap_create<4>{id, path2};
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

      /** RGM::Base.bitmap_dispose 方法 */
      static VALUE dispose(VALUE, VALUE id_) {
        RGMLOAD(id, uint64_t);

        worker >> bitmap_dispose{id};
        return Qnil;
      }

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

      static VALUE fill_rect(VALUE, VALUE id_, VALUE rect_, VALUE color_) {
        RGMLOAD(id, uint64_t);

        rect r;
        r << rect_;
        color c;
        c << color_;
        worker >> bitmap_fill_rect{r, id, c};
        return Qnil;
      }

      static VALUE text_size(VALUE, VALUE font_, VALUE text_) {
        RGMLOAD(text, const char*);

        int id = detail::get<word::id, int>(font_);
        int size = detail::get<word::size, int>(font_);

        font_manager<true>& fonts = RGMDATA(font_manager<true>);
        cen::font& font = fonts.get(id, size);

        int width, height;
        if (TTF_SizeUTF8(font.get(), text, &width, &height) == 0) {
          return INT2FIX(height * 65536 + width);
        }
        return INT2FIX(0);
      }

      static VALUE draw_text(VALUE, VALUE id_, VALUE font_, VALUE rect_,
                             VALUE text_, VALUE align_) {
        RGMLOAD(id, uint64_t);
        RGMLOAD(align, uint8_t);
        RGMLOAD(text, const char*);

        if (RSTRING_LEN(text_) == 0) return Qnil;

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

      // 参见：https://github.com/libsdl-org/SDL/issues/4782
      // 实际测试 d3d11 问题仍然存在，好在 opengl 和 direct3d9 测试通过。
      static VALUE get_pixel(VALUE, VALUE id_, VALUE x_, VALUE y_) {
        RGMLOAD(id, uint64_t);
        RGMLOAD(x, int);
        RGMLOAD(y, int);

        std::array<uint8_t, 4> pixels{};
        worker >> bitmap_get_pixel{id, x, y, pixels.data()};
        RGMWAIT(1);
        uint32_t color = (pixels[3] << 24) + (pixels[2]) + (pixels[1] << 8) +
                         (pixels[0] << 16);
        return UINT2NUM(color);
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_BASE = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_BASE, "bitmap_create", wrapper::create,
                              3);
    rb_define_module_function(rb_mRGM_BASE, "bitmap_dispose", wrapper::dispose,
                              1);
    rb_define_module_function(rb_mRGM_BASE, "bitmap_blt", wrapper::blt, 6);
    rb_define_module_function(rb_mRGM_BASE, "bitmap_stretch_blt",
                              wrapper::stretch_blt, 5);
    rb_define_module_function(rb_mRGM_BASE, "bitmap_fill_rect",
                              wrapper::fill_rect, 3);
    rb_define_module_function(rb_mRGM_BASE, "bitmap_draw_text",
                              wrapper::draw_text, 5);
    rb_define_module_function(rb_mRGM_BASE, "bitmap_text_size",
                              wrapper::text_size, 2);
    rb_define_module_function(rb_mRGM_BASE, "bitmap_get_pixel",
                              wrapper::get_pixel, 3);

    RGMBIND(rb_mRGM_BASE, "bitmap_hue_change", bitmap_hue_change, 2);
    RGMBIND(rb_mRGM_BASE, "bitmap_save_png", bitmap_save_png, 2);
    RGMBIND(rb_mRGM_BASE, "bitmap_capture_screen", bitmap_capture_screen, 1);
  }
};
}  // namespace rgm::rmxp