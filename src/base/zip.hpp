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

#ifdef RGM_EMBEDED_ZIP
INCBIN(zip, "embeded.zip");
#endif

namespace rgm::base {
/**
 * @brief 内嵌在 exe 中的资源，打包成加密的 zip 格式，只在 release 模式生效。
 */
struct zip_data_embeded {
  zip_t* archive;

  explicit zip_data_embeded() : archive(nullptr) {
#ifdef RGM_EMBEDED_ZIP
    zip_error_t error;
    zip_source_t* zs;

    zs = zip_source_buffer_create(rgm_zip_data, rgm_zip_size, 0, &error);
    archive = zip_open_from_source(zs, ZIP_RDONLY, &error);
    zip_set_default_password(archive, xorstr_(PASSWORD));
#endif
  }

  ~zip_data_embeded() {
    if (archive) zip_close(archive);
  }

  // TODO(guoxiaomi): 这里的string，在后面变成ruby
  // string时又发生了一次复制。可能需要优化？
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
};

/**
 * @brief 创建读取内嵌 zip 的 ruby 方法
 */

struct init_zip {
  using data = std::tuple<zip_data_embeded>;

  static void before(auto& this_worker) {
    static decltype(auto) worker = this_worker;

    struct wrapper {
      static VALUE empty(VALUE, VALUE) { return Qnil; }

      static VALUE load_scirpt(VALUE, VALUE path_) {
        RGMLOAD(path, const char*);
        zip_data_embeded& z = RGMDATA(zip_data_embeded);

        const std::string buf = z.load_string(path);
        if (buf.empty()) {
          rb_raise(rb_eArgError, "Cannot find embeded script `%s'.\n", path);
          return Qnil;
        }

        int ruby_state;
        VALUE object = rb_eval_string_protect(buf.data(), &ruby_state);
        if (ruby_state) {
          VALUE rbError = rb_funcall(rb_errinfo(), rb_intern("message"), 0);
          cen::log_error(rb_string_value_ptr(&rbError));

          rb_raise(rb_eLoadError,
                   "ERROR: Failed to load embeded script `%s'.\n", path);
          return Qnil;
        }

        return object;
      }

      static VALUE load_file(VALUE, VALUE path_) {
        RGMLOAD(path, const char*);
        zip_data_embeded& z = RGMDATA(zip_data_embeded);

        const std::string buf = z.load_string(path);
        if (buf.empty()) return Qnil;

        VALUE object = rb_str_new(buf.data(), buf.size() - 1);

        return object;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
#ifdef RGM_EMBEDED_ZIP
    rb_define_module_function(rb_mRGM_Base, "load_script", wrapper::load_scirpt,
                              1);
    rb_define_module_function(rb_mRGM_Base, "load_embeded_file",
                              wrapper::load_file, 1);
#else
    rb_define_module_function(rb_mRGM_Base, "load_script", wrapper::empty, 1);
    rb_define_module_function(rb_mRGM_Base, "load_embeded_file", wrapper::empty,
                              1);
#endif
  }
};
}  // namespace rgm::base