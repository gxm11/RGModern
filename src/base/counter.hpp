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
#include "core/core.hpp"

namespace rgm::base {
/// @brief 计数器，用于生成递增的 ID
/// @name data
/// 检查某个对象的 ID 属性，如果不是 increament 的倍数，则其通常与
/// 比之略小的最接近的那个 increament 的倍数 ID 的对象有关联。
struct counter {
  /// @brief 调用时 ID 递增的量，必须是偶数
  static constexpr uint64_t increament = 10;

  /// @brief 返回一个不重复的 ID，并且此 ID 一定是 increament 的倍数
  /// @return uint64_t 返回的 ID 值
  uint64_t fetch_and_add() {
    /* ID 的初始值并不是 0，这些 ID 将保留起来备用 */
    static std::atomic<uint64_t> id = increament * 100;

    static_assert(increament % 2 == 0,
                  "Increment of counter must be an even number!");

    return id.fetch_add(increament);
  }
};

/// @brief 数据类 counter 相关的初始化类
/// @name task
struct init_counter {
  using data = std::tuple<counter>;

  static void before(auto& this_worker) {
    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /* ruby method: Base#new_id -> counter::fetch_and_add */
      static VALUE new_id(VALUE) {
        return ULL2NUM(RGMDATA(counter).fetch_and_add());
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "new_id", wrapper::new_id, 0);
  }
};
}  // namespace rgm::base