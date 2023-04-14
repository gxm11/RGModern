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

namespace rgm::ext {
struct zip_data_external {
  zip_t* archive;

  explicit zip_data_external() : archive(nullptr) {}

  ~zip_data_external() {
    if (archive) zip_close(archive);
  }

  void regist(const char* path, const char* password) {
    zip_error_t error;
    zip_source_t* zs;

    if (archive) zip_close(archive);

    if (strlen(path) != 0) {
      zs = zip_source_file_create(path, 0, 0, &error);
      archive = zip_open_from_source(zs, ZIP_RDONLY, &error);
    }
    if (strlen(password) != 0) {
      zip_set_default_password(archive, password);
    }
  }

  bool check(const char* path) {
    zip_stat_t sb;
    int ret = zip_stat(archive, path, ZIP_FL_ENC_STRICT, &sb);
    return ret == 0;
  }

  const std::string load_string(const char* path) {
    std::string buf;

    zip_stat_t sb;
    int ret = zip_stat(archive, path, ZIP_FL_ENC_STRICT, &sb);
    if (ret != 0) return buf;

    zip_file_t* file = zip_fopen(archive, path, ZIP_FL_ENC_STRICT);
    if (!file) return buf;

    buf.resize(sb.size + 1, '\0');
    zip_fread(file, buf.data(), sb.size);

    zip_fclose(file);
    return buf;
  }

  SDL_Texture* load_texture(const char* path, cen::renderer& renderer) {
    if (!archive) return nullptr;

    const std::string buf = load_string(path);
    if (buf.empty()) return nullptr;

    SDL_RWops* src = SDL_RWFromConstMem(buf.data(), buf.size() - 1);

    // Load an image from an SDL data source into a GPU texture.
    return IMG_LoadTexture_RW(renderer.get(), src, 1);
  }

  SDL_Surface* load_surface(const char* path) {
    if (!archive) return nullptr;

    const std::string buf = load_string(path);
    if (buf.empty()) return nullptr;

    SDL_RWops* src = SDL_RWFromConstMem(buf.data(), buf.size() - 1);

    // Load an image from an SDL data source into a software surface.
    return IMG_Load_RW(src, 1);
  }
};

template <size_t>
struct regist_external_data {
  using data = std::tuple<zip_data_external>;

  const char* path;
  const char* password;

  void run(auto& worker) {
    zip_data_external& z = RGMDATA(zip_data_external);

    z.regist(path, password);
  }
};

struct init_external {
  using data = std::tuple<zip_data_external>;

  static void before(auto& this_worker) {
    using detail = base::detail;

    static decltype(auto) worker = this_worker;

    struct wrapper {
      static VALUE external_regist(VALUE, VALUE path_, VALUE password_) {
        RGMLOAD(path, const char*);
        RGMLOAD(password, const char*);

        // 等待渲染线程结束任务
        worker >> regist_external_data<1>{path, password};
        RGMWAIT(1);

        zip_data_external& z = RGMDATA(zip_data_external);

        z.regist(path, password);
        return Qnil;
      }

      static VALUE external_check(VALUE, VALUE path_) {
        RGMLOAD(path, const char*);
        zip_data_external& z = RGMDATA(zip_data_external);

        if (!z.archive) return Qfalse;

        bool valid = z.check(path);
        return valid ? Qtrue : Qfalse;
      }

      static VALUE external_load(VALUE, VALUE path_) {
        RGMLOAD(path, const char*);
        zip_data_external& z = RGMDATA(zip_data_external);

        const char* path2 = path + config::resource_prefix.size();
        const std::string buf = z.load_string(path2);
        if (buf.empty()) return Qnil;

        VALUE object = rb_str_new(buf.data(), buf.size() - 1);

        return object;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Ext = rb_define_module_under(rb_mRGM, "Ext");

    rb_define_module_function(rb_mRGM_Ext, "external_check",
                              wrapper::external_check, 1);
    rb_define_module_function(rb_mRGM_Ext, "external_regist",
                              wrapper::external_regist, 2);
    rb_define_module_function(rb_mRGM_Ext, "external_load",
                              wrapper::external_load, 1);
  }
};
}  // namespace rgm::ext