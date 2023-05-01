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
#include "bitmap.hpp"

namespace rgm::rmxp {
/*
 * RGModern 新增了 Palette 类，对应于 SDL 的表面，即 cen::surface
 * 此类相当于储存在内存中的 Bitmap，可以自由操作像素。
 * 以下均是对 Palette 类各项功能的实现。
 * 由于是内存中的数据操作，几乎所有操作都是同步的。
 * 在以下注释中，Palette 和 SDL 的表面代表的是同一个概念。
 */
struct init_palette {
  static void before(auto& this_worker) {
    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /* ruby method: Palette#create -> surfaces::emplace */
      static VALUE create(VALUE, VALUE id_, VALUE width_, VALUE height_) {
        RGMLOAD(id, uint64_t);

        base::surfaces& surfaces = RGMDATA(base::surfaces);

        if (height_ == Qnil) {
          /* 读取文件并创建 Palette 对象 */
          RGMLOAD2(path, std::string_view, width_);

          std::unique_ptr<cen::surface> ptr;
          if (path.starts_with(config::resource_prefix)) {
            /* 移除开头的 config::resource_prefix */
            std::string_view path2 =
                path.substr(config::resource_prefix.size());

            /* 从外部资源包中读取图片 */
            cen::log_debug("[Palette] id = %lld, is created from %s%s", id,
                           config::resource_prefix.data(), path2);

            ext::zip_data_external& z = RGMDATA(ext::zip_data_external);
            SDL_Surface* ptr2 = z.load_surface(path2);
            ptr = std::make_unique<cen::surface>(ptr2);
          } else {
            cen::log_debug("[Palette] id = %lld, is created from %s", id,
                           path.data());

            ptr = std::make_unique<cen::surface>(path.data());
          }
          /* 转换 Palette 的格式为 rgba32 */
          cen::surface s2 = ptr->convert_to(config::surface_format);

          /* 添加到容器中统一管理 */
          surfaces.emplace(id, std::move(s2));
        } else {
          /* 根据指定大小创建 Palette 对象 */
          RGMLOAD(width, int);
          RGMLOAD(height, int);

          cen::log_debug("[Palette] id = %lld, is created with area %d x %d",
                         id, width, height);

          cen::surface s(cen::iarea{width, height}, config::surface_format);
          surfaces.emplace(id, std::move(s));
        }
        return Qnil;
      }

      /* ruby method: Palette#dispose -> surfaces::erase */
      static VALUE dispose(VALUE, VALUE id_) {
        RGMLOAD(id, uint64_t);

        base::surfaces& surfaces = RGMDATA(base::surfaces);

        cen::log_debug("[Palette] id = %lld, is disposed", id);

        surfaces.erase(id);
        return Qnil;
      }

      /* ruby method: Palette#get_pixel -> cen::surface::pixel_data */
      static VALUE get_pixel(VALUE, VALUE id_, VALUE x_, VALUE y_) {
        RGMLOAD(id, uint64_t);
        RGMLOAD(x, int);
        RGMLOAD(y, int);

        base::surfaces& surfaces = RGMDATA(base::surfaces);

        cen::surface& s = surfaces.at(id);
        uint32_t* ptr = reinterpret_cast<uint32_t*>(s.pixel_data());

        /* 位置越界返回 0 值 */
        if (x < 0 || x >= s.width()) return INT2FIX(0);
        if (y < 0 || y >= s.height()) return INT2FIX(0);

        size_t index = x + y * s.width();
        uint32_t color = ptr[index];

        return UINT2NUM(color);
      }

      /* ruby method: Palette#get_pixel -> cen::surface::pixel_data */
      static VALUE set_pixel(VALUE, VALUE id_, VALUE x_, VALUE y_,
                             VALUE color_) {
        RGMLOAD(id, uint64_t);
        RGMLOAD(x, int);
        RGMLOAD(y, int);

        base::surfaces& surfaces = RGMDATA(base::surfaces);

        cen::surface& s = surfaces.at(id);
        uint32_t* ptr = reinterpret_cast<uint32_t*>(s.pixel_data());
        if (x < 0 || x >= s.width()) return Qnil;
        if (y < 0 || y >= s.height()) return Qnil;
        size_t index = x + y * s.width();
        color c;
        c << color_;
        uint32_t color =
            c.red | (c.green << 8) | (c.blue << 16) | (c.alpha << 24);
        ptr[index] = color;
        return Qnil;
      }

      /* ruby method: Palette#save_png -> cen::surface::save_as_png */
      static VALUE save_png(VALUE, VALUE id_, VALUE path_) {
        RGMLOAD(path, std::string_view);
        RGMLOAD(id, uint64_t);

        base::surfaces& surfaces = RGMDATA(base::surfaces);

        cen::log_info("[Palette] id = %lld, is saved to %s", id, path);

        cen::surface& s = surfaces.at(id);
        s.save_as_png(path.data());
        return Qnil;
      }

      /* ruby method: Palette#convert_to_bitmap -> bitmap_capture_palette */
      static VALUE convert_to_bitmap(VALUE, VALUE id_, VALUE bitmap_id_,
                                     VALUE rect_) {
        RGMLOAD(id, uint64_t);
        RGMLOAD(bitmap_id, uint64_t);

        rect r;
        r << rect_;

        base::surfaces& surfaces = RGMDATA(base::surfaces);
        cen::surface& s = surfaces.at(id);

        /* 复制一份像素数据，然后异步执行 */
        auto ptr = std::make_unique<cen::surface>(cen::iarea{r.width, r.height},
                                                  config::surface_format);
        SDL_Rect src{r.x, r.y, r.width, r.height};
        SDL_Rect dst{0, 0, r.width, r.height};
        SDL_BlitSurface(s.get(), &src, ptr->get(), &dst);

        worker >> bitmap_capture_palette{bitmap_id, std::move(ptr)};

        return Qnil;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "palette_create", wrapper::create,
                              3);
    rb_define_module_function(rb_mRGM_Base, "palette_dispose", wrapper::dispose,
                              1);
    rb_define_module_function(rb_mRGM_Base, "palette_get_pixel",
                              wrapper::get_pixel, 3);
    rb_define_module_function(rb_mRGM_Base, "palette_set_pixel",
                              wrapper::set_pixel, 4);
    rb_define_module_function(rb_mRGM_Base, "palette_save_png",
                              wrapper::save_png, 2);
    rb_define_module_function(rb_mRGM_Base, "palette_convert_to_bitmap",
                              wrapper::convert_to_bitmap, 3);
  }
};
}  // namespace rgm::rmxp