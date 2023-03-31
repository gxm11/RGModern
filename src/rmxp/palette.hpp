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
#include "base/base.hpp"
#include "detail.hpp"

namespace rgm::rmxp {
struct init_palette {
  static void before(auto& this_worker) {
    static decltype(auto) worker = this_worker;

    /** wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      static VALUE create(VALUE, VALUE id_, VALUE width_, VALUE height_) {
        RGMLOAD(id, uint64_t);
        base::surfaces& surfaces = RGMDATA(base::surfaces);

        if (height_ == Qnil) {
          RGMLOAD2(path, const char*, width_);

          int ret = strncmp(config::resource_prefix.data(), path,
                            config::resource_prefix.size());
          std::unique_ptr<cen::surface> ptr;
          if (ret == 0) {
            const char* path2 = path + config::resource_prefix.size();
            zip_data_external& z = RGMDATA(zip_data_external);
            SDL_Surface* ptr2 = z.load_surface(path2);
            ptr = std::make_unique<cen::surface>(ptr2);
            cen::log_info(
                          "[Palette] id = %lld, is created from external://%s",
                          id, path2);
          } else {
            ptr = std::make_unique<cen::surface>(path);
            cen::log_info(
                          "[Palette] id = %lld, is created from %s", id, path);
          }
          cen::surface s2 = ptr->convert_to(cen::pixel_format::rgba32);
          surfaces.emplace(id, std::move(s2));
        } else {
          RGMLOAD(width, int);
          RGMLOAD(height, int);

          cen::log_debug(
                         "[Palette] id = %lld, is created with area %d x %d",
                         id, width, height);

          cen::surface s(cen::iarea{width, height}, cen::pixel_format::rgba32);
          surfaces.emplace(id, std::move(s));
        }
        return Qnil;
      }

      static VALUE dispose(VALUE, VALUE id_) {
        RGMLOAD(id, uint64_t);
        base::surfaces& surfaces = RGMDATA(base::surfaces);

        cen::log_debug(
                       "[Palette] id = %lld, is disposed", id);

        surfaces.erase(id);
        return Qnil;
      }

      static VALUE get_pixel(VALUE, VALUE id_, VALUE x_, VALUE y_) {
        RGMLOAD(id, uint64_t);
        RGMLOAD(x, int);
        RGMLOAD(y, int);

        base::surfaces& surfaces = RGMDATA(base::surfaces);

        cen::surface& s = surfaces.at(id);
        uint32_t* ptr = reinterpret_cast<uint32_t*>(s.pixel_data());
        if (x < 0 || x >= s.width()) return Qnil;
        if (y < 0 || y >= s.height()) return Qnil;
        size_t index = x + y * s.width();
        uint32_t color = ptr[index];
        return UINT2NUM(color);
      }

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

      static VALUE save_png(VALUE, VALUE id_, VALUE path_) {
        RGMLOAD(path, const char*);
        RGMLOAD(id, uint64_t);
        base::surfaces& surfaces = RGMDATA(base::surfaces);

        cen::log_info(
                      "[Palette] id = %lld, is saved to %s", id, path);

        cen::surface& s = surfaces.at(id);
        s.save_as_png(path);
        return Qnil;
      }

      static VALUE convert_to_bitmap(VALUE, VALUE id_, VALUE bitmap_id_,
                                     VALUE rect_) {
        RGMLOAD(id, uint64_t);
        RGMLOAD(bitmap_id, uint64_t);

        rect r;
        r << rect_;

        base::surfaces& surfaces = RGMDATA(base::surfaces);
        cen::surface& s = surfaces.at(id);

        auto ptr = std::make_unique<cen::surface>(cen::iarea{r.width, r.height},
                                                  cen::pixel_format::rgba32);
        SDL_Rect src{r.x, r.y, r.width, r.height};
        SDL_Rect dst{0, 0, r.width, r.height};
        SDL_BlitSurface(s.get(), &src, ptr->get(), &dst);

        worker >> bitmap_capture_palette{bitmap_id, std::move(ptr)};
        // RGMWAIT(1);
        return Qnil;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_BASE = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_BASE, "palette_create", wrapper::create,
                              3);
    rb_define_module_function(rb_mRGM_BASE, "palette_dispose", wrapper::dispose,
                              1);
    rb_define_module_function(rb_mRGM_BASE, "palette_get_pixel",
                              wrapper::get_pixel, 3);
    rb_define_module_function(rb_mRGM_BASE, "palette_set_pixel",
                              wrapper::set_pixel, 4);
    rb_define_module_function(rb_mRGM_BASE, "palette_save_png",
                              wrapper::save_png, 2);
    rb_define_module_function(rb_mRGM_BASE, "palette_convert_to_bitmap",
                              wrapper::convert_to_bitmap, 3);
  }
};
}  // namespace rgm::rmxp