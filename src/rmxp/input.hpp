// zlib License

// copyright (C) 2023 Xiaomi Guo and Krimiston

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
#include <set>

#include "base/base.hpp"

namespace rgm::rmxp {
/**
 * @brief 按键映射组合，其中的元素是 { SDL_Key, RMXP Input Key }
 */
struct keymap : std::set<std::pair<int32_t, uint32_t>> {
  explicit keymap() : std::set<std::pair<int32_t, uint32_t>>() {}

  /**
   * @brief 遍历所有的 { SDL_Key, RMXP Input Key } 组合，执行特定函数
   * @param sdl_key 组合的第一项必须等于该值
   * @param callback 只接受一个参数，即 RMXP Input Key
   */
  void iterate(int32_t sdl_key, std::function<void(uint32_t)> callback) {
    auto it = lower_bound({sdl_key, 0});
    while (it != end()) {
      if (it->first != sdl_key) break;
      callback(it->second);
      ++it;
    }
  }
};

/**
 * @brief keystate 是一个数组，索引代表相应的按键，值代表按键的状态
 * @note 值的每一位代表每一帧该按键是否按下，0 表示松开，1 表示按下。
 * 最后一位（最低位）代表当前帧的情况，倒数第 N 位代表向前推算 N - 1 帧的情况。
 */
struct keystate {
  std::array<uint32_t, 256> data;

  int32_t last_press;
  int32_t last_release;

  explicit keystate() : data() {}

  /**
   * @brief 更新按键当前帧状态，默认延续上一帧的状态
   * @note S0 -> S00, S1 -> S11。在此之后需要读取 press 和 release
   * 的信息进一步更新。
   */
  void update() {
    for (size_t i = 0; i < data.size(); ++i) {
      const uint32_t value = data[i];
      data[i] = (value & 1) | (value << 1);
    }
  }

  /**
   * @brief 将按键当前帧的状态改为按下，S? -> S1
   * @param key 目标按键
   */
  void press(uint32_t key) { data[key % data.size()] |= 0b01; }

  /**
   * @brief 将按键当前帧的状态改为松开，S? -> S0
   * @param key 目标按键
   */
  void release(uint32_t key) {
    data[key % data.size()] ^= (data[key % data.size()] & 1);
  }

  /**
   * @brief 判断按键是否刚刚按下，只需检测最低 2 位是否为 01
   * @param key 目标按键
   */
  bool is_trigger(uint32_t key) {
    return (data[key % data.size()] & 0b11) == 0b01;
  }

  /**
   * @brief 判断按键是否正在按下，只需检测最低 1 位是否为 1
   * @param key 目标按键
   */
  bool is_press(uint32_t key) {
    return (data[key % data.size()] & 0b01) == 0b01;
  }

  /**
   * @brief 判断按键是否正在按下，但是每 interval 帧只会有 1 帧返回 true
   * @param key 目标按键
   * @note 实现方案：
   * RMXP 内置的 repeat 方案，是从 trigger 的那一帧算起（设为1），第 16 + 4x
   * 帧触发。 并且 trigger 那一帧也算。从而满足以下pattern时触发：
   * 1. 后两位是 0b01，即 trigger 触发时
   * 2. 后16位是 0b1111'1111'1111'1111
   * 3. 后16位是 0b1110'1111'1111'1111
   * 从而在满足 0b1111'1111'1111'1111 时，修改按键的状态为
   * 0b1110'1111'1111'1111， 4 帧后又会再次触发。而前两次触发则出现在 trigger
   * 的当前帧，和那之后的第 15 帧。 interval_a 和 interval_b 可以修改。
   */

  bool is_repeat(uint32_t key) {
    /** 按键按下时，重复触发的周期为 a + bx，单位：帧 */
    constexpr int interval_a = 16;
    constexpr int interval_b = 4;
    constexpr uint32_t pattern_1 = (1 << interval_a) - 1;
    constexpr uint32_t pattern_2 = pattern_1 ^ (1 << (interval_a - interval_b));

    static_assert(interval_a < 30,
                  "Value of interval_a must be less than 30.\n");
    static_assert(interval_b < interval_a - 4,
                  "Value of interval_b must be less than interval_a - 4.\n");

    uint32_t& value = data[key % data.size()];

    if ((value & 0b11) == 0b01) return true;
    if ((value & pattern_1) == pattern_2) return true;
    if ((value & pattern_1) == pattern_1) {
      value = value & pattern_2;
      return true;
    }
    return false;
  }

  void reset() {
    data.fill(0);
    last_press = 0;
    last_release = 0;
  }
};

/**
 * @brief 任务：按键按下
 */
struct key_press {
  int32_t sdl_key;

  void run(auto& worker) {
    keymap& map = RGMDATA(keymap);
    keystate& state = RGMDATA(keystate);

    state.last_press = sdl_key;
    map.iterate(sdl_key, [&state](int32_t key) { state.press(key); });
  }
};

/**
 * @brief 任务：按键抬起
 */
struct key_release {
  int32_t sdl_key;

  void run(auto& worker) {
    keymap& map = RGMDATA(keymap);
    keystate& state = RGMDATA(keystate);

    state.last_release = sdl_key;
    map.iterate(sdl_key, [&state](int32_t key) { state.release(key); });
  }
};

/**
 * @brief 添加 keymap 和 keystate 到 worker 的 datalist 中，并创建 Input 相关的
 * ruby 方法。
 */
struct init_input {
  using data = std::tuple<keymap, keystate>;

  static void before(auto& this_worker) {
    static decltype(auto) worker = this_worker;

    struct wrapper {
      static VALUE bind(VALUE, VALUE sdl_key_, VALUE input_key_) {
        keymap& map = RGMDATA(keymap);
        int32_t sdl_key = NUM2LONG(sdl_key_);

        if (input_key_ == Qnil) {
          auto begin = map.lower_bound({sdl_key, 0});
          auto end = map.lower_bound({sdl_key + 1, 0})++;
          map.erase(begin, end);
        } else {
          Check_Type(input_key_, T_FIXNUM);
          map.insert({sdl_key, FIX2UINT(input_key_)});
        }
        return Qnil;
      }

      static VALUE is_trigger(VALUE, VALUE input_key_) {
        Check_Type(input_key_, T_FIXNUM);
        return RGMDATA(keystate).is_trigger(FIX2UINT(input_key_)) ? Qtrue
                                                                  : Qfalse;
      }

      static VALUE is_press(VALUE, VALUE input_key_) {
        Check_Type(input_key_, T_FIXNUM);
        return RGMDATA(keystate).is_press(FIX2UINT(input_key_)) ? Qtrue
                                                                : Qfalse;
      }

      static VALUE is_repeat(VALUE, VALUE input_key_) {
        Check_Type(input_key_, T_FIXNUM);
        return RGMDATA(keystate).is_repeat(FIX2UINT(input_key_)) ? Qtrue
                                                                 : Qfalse;
      }

      static VALUE update(VALUE) {
        RGMDATA(keystate).update();
        // 在 Graphics.update 里也有一次 poll_event，处理当前积压的事件
        // 由于执行了 flush() 函数清空任务队列，Input 数据总会适时刷新。
        worker >> base::poll_event{};
        worker.flush();
        return Qnil;
      }

      static VALUE reset(VALUE) {
        RGMDATA(keystate).reset();
        return Qnil;
      }

      static VALUE last_press(VALUE) {
        int32_t key = RGMDATA(keystate).last_press;
        return INT2NUM(key);
      }

      static VALUE last_release(VALUE) {
        int32_t key = RGMDATA(keystate).last_release;
        return INT2NUM(key);
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    rb_define_const(rb_mRGM, "Max_Keycode",
                    INT2FIX(RGMDATA(keystate).data.size() - 1));
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