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
#include "word.hpp"

namespace rgm::rmxp {
/// @brief 记录键盘按键与 RGSS 中虚拟按键的映射关系
/// 同一个键盘按键可以对应多个虚拟按键，当此键盘按键按下或抬起时，
/// 会触发此按键对应的全部虚拟按键的按下或抬起。
/// RGSS 中可以调用 Input.trigger? 等方法检测相应的虚拟按键。
struct keymap {
  /// @brief 储存映射关系的 set，有序排放
  /// pair 中第一个元素是 SDL_Key，
  /// 第二个元素是 RGSS 虚拟按键，即 RMXP Input Key。
  std::set<std::pair<int32_t, int>> m_data;

  /// @brief 遍历所有的 { SDL_Key, RMXP Input Key } 组合，执行特定函数
  /// @param 组合的第一项必须等于该值
  /// @param callback 只接受一个 int 类型的参数，即 RMXP Input Key
  /// callback 通常是修改 RMXP Input Key 按下或抬起的状态。
  void iterate(int32_t sdl_key, std::function<void(int)> callback) const {
    /*
     * 查询不小于 {sdl_key, 0} 的那个元素
     * 此元素显然是 {sdl_key, i}，i 对应绑定的值最小的那个虚拟按键值
     * 若 sdl_key 未有任何绑定，则返回 end()
     */
    auto it = m_data.lower_bound({sdl_key, 0});

    while (it != m_data.end()) {
      /* 当键盘按键不再是 sdl_key 时，结束循环 */
      if (it->first != sdl_key) break;

      /* 使用绑定的虚拟按键触发回调 */
      callback(it->second);

      ++it;
    }
  }

  /// @brief 重置特定键盘按键的全部映射，使其不再对应任何虚拟按键
  /// 即从 set 中移除 {sdl_key, i}，i 是任意的 int
  void erase(int32_t sdl_key) {
    /*
     * 查询不小于 {sdl_key, 0} 的那个元素
     * 此元素显然是 {sdl_key, i}，i 对应绑定的值最小的那个虚拟按键值
     * 若 sdl_key 未有任何绑定，则返回 end()
     */
    auto begin = m_data.lower_bound({sdl_key, 0});

    if (begin == m_data.end()) return;

    /*
     * 查询不小于 {sdl_key + 1, 0} 的那个元素
     * 此元素显然是 {sdl_key_next, ?}，其中 sdl_key_next > sdl_key。
     */
    auto end = m_data.lower_bound({sdl_key + 1, 0});

    /* STL 中算法遵循前闭后开原则，begin 会被移除而 end 不会被移除 */
    m_data.erase(begin, end);
  }

  /// @brief 添加一组映射关系
  /// @param sdl_key 键盘按键
  /// @param key RGSS 虚拟按键
  void insert(int32_t sdl_key, int key) { m_data.insert({sdl_key, key}); }
};

/// @brief keystate 存储了虚拟按键按下或抬起的状态。
struct keystate {
  /// @brief 最多支持的虚拟按键数量，设置为 256 应该足够。
  static constexpr size_t max = 256;

  /// @brief 存储按键状态的数组，索引代表相应的按键，值代表按键的状态。
  /// 值的每一位代表每一帧该按键是否按下，0 表示松开，1 表示按下。最后一位
  /// （最低位）代表当前帧的情况，倒数第 N 位代表向前推算 N - 1 帧的情况。
  /// 比如 0b00000000_00000000_00000000_00000001，代表此帧是刚刚按下对应按键。
  std::array<uint32_t, max> m_data;

  /// @brief 记录最后一次按键按下事件对应的 SDL_Key
  int32_t last_press;

  /// @brief 记录最后一次按键抬起事件对应的 SDL_Key
  int32_t last_release;

  /// @brief 更新按键当前帧状态，默认延续上一帧的状态。
  /// S0 -> S00，S1 -> S11
  /// 在此之后才会执行 key_press 和 key_release 的事件继续更新。
  void update() {
    for (size_t i = 0; i < max; ++i) {
      const uint32_t value = m_data[i];

      m_data[i] = (value & 1) | (value << 1);
    }
  }

  /// @brief 将按键当前帧的状态改为按下
  /// @param key 目标按键
  /// S? -> S1
  void press(int key) { m_data[key % max] |= 0b01; }

  /// @brief 将按键当前帧的状态改为松开
  /// @param key 目标按键
  /// S? -> S0
  void release(int key) { m_data[key % max] ^= (m_data[key % max] & 1); }

  /// @brief 判断按键是否刚刚按下，只需检测最低 2 位是否为 01
  /// @param key 目标按键
  /// @return 返回 true 表示此按键刚刚按下
  [[nodiscard]] bool is_trigger(int key) const {
    return (m_data[key % max] & 0b11) == 0b01;
  }

  /// @brief 判断按键是否正在按下，只需检测最低 1 位是否为 1
  /// @param key 目标按键
  /// @return 返回 true 表示此按键正在按下
  [[nodiscard]] bool is_press(int key) const {
    return (m_data[key % max] & 0b01) == 0b01;
  }

  /// @brief 判断按键是否正在重复按下
  /// @param key 目标按键
  /// 和按下的判断条件相同，但是连续的若干帧里只会有 1 帧返回 true。
  /// RMXP 内置的 repeat 方案，是从 trigger 的那一帧算起（设为1），
  /// 第 16 + 4x 帧触发。并且 trigger 那一帧也算。
  /// 考虑以下 pattern 时触发：
  /// 1. 后两位是 0b01，即 trigger 触发时
  /// 2. 后16位是 0b1111'1111'1111'1111
  /// 3. 后16位是 0b1110'1111'1111'1111
  /// 且在满足(2)时，修改按键的状态为 0b1110'1111'1111'1111，持续 4
  /// 帧后又会再次触发。
  [[nodiscard]] bool is_repeat(int key) {
    /* 按键按下时，重复触发的周期为 a + bx，单位：帧 */
    constexpr int interval_a = 16;
    constexpr int interval_b = 4;

    /* 根据重复触发的周期，设置键值匹配的模式 */
    constexpr uint32_t pattern_1 = (1u << interval_a) - 1;
    constexpr uint32_t pattern_2 =
        pattern_1 ^ (1u << (interval_a - interval_b));

    static_assert(interval_a < 30,
                  "Value of interval_a must be less than 30.\n");
    static_assert(interval_b < interval_a - 4,
                  "Value of interval_b must be less than interval_a - 4.\n");

    uint32_t& value = m_data[key % max];

    /* 对应上面第 1 种模式 */
    if ((value & 0b11) == 0b01) return true;
    /* 对应上面第 3 种模式 */
    if ((value & pattern_1) == pattern_2) return true;
    /* 对应上面第 2 种模式 */
    if ((value & pattern_1) == pattern_1) {
      /* 修改按键的状态为满足第 3 种模式 */
      value = value & pattern_2;
      return true;
    }
    return false;
  }

  /// @brief 重置所有的按键状态为抬起，清空上次按键的记忆
  void reset() {
    m_data.fill(0);
    last_press = 0;
    last_release = 0;
  }
};

/// @brief 按键按下的事件
struct key_press {
  /// @brief 键盘的按键
  int32_t sdl_key;

  void run(auto& worker) {
    keymap& map = RGMDATA(keymap);
    keystate& state = RGMDATA(keystate);

    /* 记录为 last_press 值 */
    state.last_press = sdl_key;

    /* 修改 key_state 中对应的值 */
    map.iterate(sdl_key, [&state](int key) { state.press(key); });
  }
};

/// @brief 按键抬起的事件
struct key_release {
  /// @brief 键盘的按键
  int32_t sdl_key;

  void run(auto& worker) {
    keymap& map = RGMDATA(keymap);
    keystate& state = RGMDATA(keystate);

    /* 记录为 last_press 值 */
    state.last_release = sdl_key;

    /* 修改 key_state 中对应的值 */
    map.iterate(sdl_key, [&state](int key) { state.release(key); });
  }
};

/// @brief 按键相关操作的初始化类
struct init_input {
  /* 引入数据对象 keymap 和 keystate */
  using data = std::tuple<keymap, keystate>;

  static void before(auto& this_worker) noexcept {
    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /* ruby method: Base#input_bind -> keymap::insert */
      static VALUE bind(VALUE, VALUE sdl_key_, VALUE input_key_) {
        keymap& map = RGMDATA(keymap);
        /* sdl_key 的值超过了 0x4000_0000，不能使用 FIX2INT */
        int32_t sdl_key = NUM2LONG(sdl_key_);

        if (input_key_ == Qnil) {
          map.erase(sdl_key);
        } else {
          RGMLOAD(input_key, int);

          map.insert(sdl_key, input_key);
        }
        return Qnil;
      }

      /* ruby method: Base#input_trigger -> keystate::is_trigger */
      static VALUE is_trigger(VALUE, VALUE input_key_) {
        RGMLOAD(input_key, int);

        return RGMDATA(keystate).is_trigger(input_key) ? Qtrue : Qfalse;
      }

      /* ruby method: Base#input_press -> keystate::is_press */
      static VALUE is_press(VALUE, VALUE input_key_) {
        RGMLOAD(input_key, int);

        return RGMDATA(keystate).is_press(input_key) ? Qtrue : Qfalse;
      }

      /* ruby method: Base#input_repeat -> keystate::is_repeat */
      static VALUE is_repeat(VALUE, VALUE input_key_) {
        RGMLOAD(input_key, int);

        return RGMDATA(keystate).is_repeat(input_key) ? Qtrue : Qfalse;
      }

      /* ruby method: Base#input_update -> keystate::update */
      static VALUE update(VALUE) {
        /* 更新 keystate */
        RGMDATA(keystate).update();

        /*
         * 处理当前积压的事件
         * 在 Graphics.update 里也有一次 poll_event
         */
        worker >> base::poll_event{};

        /*
         * 执行了 flush() 函数清空任务队列
         * 若队列里有按键事件（包括控制器按键事件），就会在此处刷新 keystate。
         * 由于主动 worker 必须定期调用 flush()，相应的，ruby 中必须定期调用
         * Input.update
         */
        worker.flush();

        return Qnil;
      }

      /* ruby method: Base#input_reset -> keystate::reset */
      static VALUE reset(VALUE) {
        RGMDATA(keystate).reset();
        return Qnil;
      }

      /* ruby method: Base#input_last_press -> keystate::last_press */
      static VALUE last_press(VALUE) {
        int key = RGMDATA(keystate).last_press;
        return INT2FIX(key);
      }

      /* ruby method: Base#input_last_release -> keystate::last_release */
      static VALUE last_release(VALUE) {
        int key = RGMDATA(keystate).last_release;
        return INT2FIX(key);
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    rb_define_const(rb_mRGM, "Max_Keycode", INT2FIX(keystate::max - 1));
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "input_update", wrapper::update, 0);
    rb_define_module_function(rb_mRGM_Base, "input_reset", wrapper::reset, 0);
    rb_define_module_function(rb_mRGM_Base, "input_trigger",
                              wrapper::is_trigger, 1);
    rb_define_module_function(rb_mRGM_Base, "input_press", wrapper::is_press,
                              1);
    rb_define_module_function(rb_mRGM_Base, "input_repeat", wrapper::is_repeat,
                              1);
    rb_define_module_function(rb_mRGM_Base, "input_bind", wrapper::bind, 2);
    rb_define_module_function(rb_mRGM_Base, "input_last_press",
                              wrapper::last_press, 0);
    rb_define_module_function(rb_mRGM_Base, "input_last_release",
                              wrapper::last_release, 0);
  }
};
}  // namespace rgm::rmxp