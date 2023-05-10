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
/// @brief 管理外部 zip 资源包的类
/// 可以直接从外部资源包读取 texture 和 surface，对应为 ruby 中的
/// Bitmap 和 Palette。此方法用于实现图像素材的加密。
struct zip_data_external {
  /// @brief 管理外部资源文件的指针
  zip_t* archive;

  /// @brief 在构造函数中什么也不做
  explicit zip_data_external() : archive(nullptr) {}

  /// @brief 在析构函数中释放保存的资源
  ~zip_data_external() {
    if (archive) zip_close(archive);
  }

  /// @brief 注册某个 zip 包为外部资源包
  /// @param path 外部资源包的路径
  /// @param password 外部资源包的密码
  void regist(std::string_view path, std::string_view password) {
    zip_error_t error;
    zip_source_t* zs;

    if (archive) zip_close(archive);

    if (path.size() != 0) {
      zs = zip_source_file_create(path.data(), 0, 0, &error);
      archive = zip_open_from_source(zs, ZIP_RDONLY, &error);
    }
    if (password.size() != 0) {
      zip_set_default_password(archive, password.data());
    }
  }

  /// @brief 检查某个路径是否位于外部资源包中
  /// @param path 要检查的文件名称
  /// @return 如果该文件存在，则 true，否则 false
  [[nodiscard]] bool check(std::string_view path) const {
    zip_stat_t sb;
    int ret = zip_stat(archive, path.data(), ZIP_FL_ENC_STRICT, &sb);
    return ret == 0;
  }

  /// @brief 读取外部资源包中指定的文件的内容
  /// @param path 外部资源包中的文件路径
  /// @return 成功则 std::string 中存储了文件的内容，失败返回 std::nullopt
  [[nodiscard]] std::optional<std::string> load_string(
      std::string_view path) const {
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

  /// @brief 直接读取外部资源包中的图像文件为 cen::texture
  /// @param path 外部资源包中的图像文件路径
  /// @param renderer SDL 的渲染器
  /// @return 成功则返回新创建的 cen::texture，失败则返回 std::nullopt。
  [[nodiscard]] std::optional<cen::texture> load_texture(
      std::string_view path, cen::renderer& renderer) const {
    if (!archive) return std::nullopt;

    auto buf = load_string(path);
    if (!buf) return std::nullopt;

    SDL_RWops* src = SDL_RWFromConstMem(buf->data(), buf->size());

    // Load an image from an SDL data source into a GPU texture.
    SDL_Texture* ptr = IMG_LoadTexture_RW(renderer.get(), src, 1);
    if (!ptr) return std::nullopt;
    
    return cen::texture(ptr);
  }

  /// @brief 直接读取外部资源包中的图像文件为 cen::surface
  /// @param path 外部资源包中的图像文件路径
  /// @param renderer SDL 的渲染器
  /// @return 成功则返回新创建的 cen::surface，失败则返回 std::nullopt。
  [[nodiscard]] std::optional<cen::surface> load_surface(std::string_view path) const {
    if (!archive) return std::nullopt;

    auto buf = load_string(path);
    if (!buf) return std::nullopt;

    SDL_RWops* src = SDL_RWFromConstMem(buf->data(), buf->size());

    // Load an image from an SDL data source into a software surface.
    SDL_Surface* ptr = IMG_Load_RW(src, 1);
    if (!ptr) return std::nullopt;

    return cen::surface(ptr);
  }
};

/// @brief 注册外部资源包
/// 自动附带了数据 zip_data_external。
template <size_t worker_id = 0>
struct regist_external_data {
  using data = std::tuple<zip_data_external>;

  /// @brief path 外部资源包的路径
  std::string path;

  /// @brief password 外部资源包的密码
  std::string password;

  void run(auto& worker) {
    zip_data_external& z = RGMDATA(zip_data_external);

    z.regist(path, password);

    /* 递归广播此任务给下一个 worker */
    if constexpr (worker_id < config::max_workers) {
      worker >> regist_external_data<worker_id + 1>{std::move(path),
                                                    std::move(password)};
    }
  }
};

/// @brief 数据类 zip_data_external 相关的初始化类
struct init_external {
  static void before(auto& this_worker) {
    /* 需要使用 base::detail 完成 ruby 到 C++ 类型的转换 */
    using detail = base::detail;

    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    struct wrapper {
      /* ruby method: Ext#external_regist -> regist_external_data */
      static VALUE external_regist(VALUE, VALUE path_, VALUE password_) {
        RGMLOAD(path, std::string);
        RGMLOAD(password, std::string);

        /* 这里不能用 worker >> 送到队列中，需要立刻执行 */
        regist_external_data<0>{path, password}.run(worker);

        return Qnil;
      }

      /* ruby method: Ext#external_check -> zip_data_external::check */
      static VALUE external_check(VALUE, VALUE path_) {
        RGMLOAD(path, std::string_view);

        zip_data_external& z = RGMDATA(zip_data_external);

        if (!z.archive) return Qfalse;

        bool valid = z.check(path);
        return valid ? Qtrue : Qfalse;
      }

      /* ruby method: Ext#external_load -> zip_data_external::load_string */
      static VALUE external_load(VALUE, VALUE path_) {
        RGMLOAD(path, std::string_view);

        zip_data_external& z = RGMDATA(zip_data_external);

        std::string_view path2 = path.substr(config::resource_prefix.size());
        auto buf = z.load_string(path2);
        if (!buf) return Qnil;

        VALUE object = rb_str_new(buf->data(), buf->size());

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