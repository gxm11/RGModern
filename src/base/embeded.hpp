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
#include "detail.hpp"

#ifdef RGM_EMBEDED_ZIP
INCBIN(zip, RGM_EMBEDED_ZIP);

namespace rgm::base {
/// @brief 管理内嵌在 exe 中的资源文件类
/// @name data
/// 对于 build_mode <= 1，此类没有任何作用；
/// 对于 build_mode = 2，用加密 zip 格式打包脚本文件夹 src/scripts；
/// 对于 build_mode = 3，在 2 的基础上额外打包数据文件夹（宏）Data。
struct zip_data_embeded {
  /// @brief 管理内嵌资源文件的指针
  zip_t* archive;

  /// @brief 在构造函数中读取内嵌的资源
  explicit zip_data_embeded() : archive(nullptr) {
    zip_error_t error;
    zip_source_t* zs;

    zs = zip_source_buffer_create(rgm_zip_data, rgm_zip_size, 0, &error);
    archive = zip_open_from_source(zs, ZIP_RDONLY, &error);
    zip_set_default_password(archive, xorstr_(PASSWORD));
  }

  /// @brief 在析构函数中释放保存的资源
  ~zip_data_embeded() {
    if (archive) zip_close(archive);
  }

  /// @brief 读取内嵌资源包中指定的文件的内容
  /// @param path 内嵌资源包中的文件路径
  /// @return 成功则 std::string 中存储了文件的内容，失败返回 std::nullopt
  std::optional<std::string> load_string(std::string_view path) const {
    std::string buf;

    zip_stat_t sb;
    int ret = zip_stat(archive, path.data(), ZIP_FL_ENC_STRICT, &sb);
    if (ret != 0) return std::nullopt;

    zip_file_t* file = zip_fopen(archive, path.data(), ZIP_FL_ENC_STRICT);
    if (!file) return std::nullopt;

    buf.resize(sb.size);
    zip_fread(file, buf.data(), sb.size);

    zip_fclose(file);
    return buf;
  }
};

/// @brief 数据类 zip_data_embeded 相关的初始化类
/// @name task
struct init_embeded {
  using data = std::tuple<zip_data_embeded>;

  static void before(auto& this_worker) {
    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /* ruby method: Base#load_scirpt -> zip_data_embeded::load_string */
      static VALUE load_scirpt(VALUE, VALUE path_) {
        RGMLOAD(path, std::string_view);
        zip_data_embeded& z = RGMDATA(zip_data_embeded);

        auto buf = z.load_string(path);
        if (!buf) {
          rb_raise(rb_eArgError, "Cannot find embeded script `%s'.\n", path.data());
          return Qnil;
        }

        int ruby_state;
        /* 安全地执行 ruby 脚本，捕获其中发生的错误 */
        VALUE object = rb_eval_string_protect(buf->data(), &ruby_state);
        if (ruby_state) {
          VALUE rbError = rb_funcall(rb_errinfo(), rb_intern("message"), 0);
          cen::log_error(rb_string_value_ptr(&rbError));

          rb_raise(rb_eLoadError,
                   "ERROR: Failed to load embeded script `%s'.\n", path.data());
          return Qnil;
        }

        return object;
      }

      /* ruby method: Base#load_file -> zip_data_embeded::load_string */
      static VALUE load_file(VALUE, VALUE path_) {
        RGMLOAD(path, std::string_view);
        zip_data_embeded& z = RGMDATA(zip_data_embeded);

        auto buf = z.load_string(path);
        if (!buf) return Qnil;

        VALUE object = rb_str_new(buf->data(), buf->size());

        return object;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "load_script", wrapper::load_scirpt,
                              1);
    rb_define_module_function(rb_mRGM_Base, "embeded_load", wrapper::load_file,
                              1);
  }
};
}  // namespace rgm::base
#else
namespace rgm::base {
/// @brief 在未定义宏 RGM_EMBEDED_ZIP 的场合，替代的 init_embeded 类
/// @name task
/// 定义了 ruby 中的函数 load_script 和 embeded_load，都始终返回 nil。
struct init_embeded {
  static void before(auto&) {
    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /* ruby method: Base#load_script -> empty */
      /* ruby method: Base#embeded_load -> empty */
      static VALUE empty(VALUE, VALUE) { return Qnil; }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "load_script", wrapper::empty, 1);
    rb_define_module_function(rb_mRGM_Base, "embeded_load", wrapper::empty, 1);
  }
};
}  // namespace rgm::base
#endif