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
            cen::log_info(cen::log_category::system,
                          "[Palette] id = %lld, is created from external://%s",
                          id, path2);
          } else {
            ptr = std::make_unique<cen::surface>(path);
            cen::log_info(cen::log_category::system,
                          "[Palette] id = %lld, is created from %s", id, path);
          }
          cen::surface s2 = ptr->convert_to(cen::pixel_format::rgba32);
          surfaces.emplace(id, std::move(s2));
        } else {
          RGMLOAD(width, int);
          RGMLOAD(height, int);

          cen::log_debug(cen::log_category::system,
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

        cen::log_debug(cen::log_category::system,
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

        cen::log_info(cen::log_category::system,
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