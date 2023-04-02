# zlib License

# copyright (C) 2023 Guoxiaomi and Krimiston

# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.

# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:

# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

module Input
  # ---------------------------------------------------------------------------
  # A module that handles input data from a gamepad or keyboard.
  # ---------------------------------------------------------------------------

  # 虚拟按键，含义是一种特定的输入。多个真实按键都可以绑定到对应的虚拟按键上。
  # 举例，RGSS 中的 Input::C 就是虚拟按键，其含义是确认的意思。
  # 并且有 Space/Enter/C 这三个真实按键绑定，任何一个按键的触发都会导致 Input::C 的触发。
  # 在 RGM 中，虚拟按键最多可以有 256 个，值从 0-255 不等。
  # 其中部分虚拟按键的值，和绑定的真实按键已经被预先设定。

  DOWN = 2
  LEFT = 4
  RIGHT = 6
  UP = 8
  A = 11
  B = 12
  C = 13
  X = 14
  Y = 15
  Z = 16
  L = 17
  R = 18
  SHIFT = 21
  CTRL = 22
  ALT = 23
  F5 = 25
  F6 = 26
  F7 = 27
  F8 = 28
  F9 = 29

  # 保留的按键，从后面用起
  DEBUG = -1
  FPS_TOGGLE = -2
  TEXT_TOGGLE = -3
  TEXT_CONFIRM = -4
  TEXT_BACKSPACE = -5
  TEXT_CLEAR = -6

  module_function

  def update
    RGM::Ext.textinput_edit_clear
    # Updates input data. As a rule, this method is called once per frame.
    RGM::Base.input_update
  end

  def reset
    # reset input states
    RGM::Base.input_reset
  end

  def press?(key)
    # Determines whether the button num is currently being pressed.
    # If the button is being pressed, returns TRUE. If not, returns FALSE.

    # RGM 允许使用 symbol 作为 key，会将其转换成对应的常量
    # RGM 会在按键按下后，抬起前判定为 press
    key = const_get(key) if key.is_a?(Symbol)
    RGM::Base.input_press(key)
  end

  def trigger?(key)
    # Determines whether the button num is being pressed again.
    # "Pressed again" is seen as time having passed between the button being not pressed and being pressed.
    # If the button is being pressed, returns TRUE. If not, returns FALSE.

    # RGM 允许使用 symbol 作为 key，会将其转换成对应的常量
    # RGM 会在按键抬起的那一帧判定为 trigger
    key = const_get(key) if key.is_a?(Symbol)
    RGM::Base.input_trigger(key)
  end

  def repeat?(key)
    # Determines whether the button num is being pressed again.
    # Unlike trigger?, takes into account the repeat input of a button being held down continuously.
    # If the button is being pressed, returns TRUE. If not, returns FALSE.

    # RGM 允许使用 symbol 作为 key，会将其转换成对应的常量
    # RGM 中同 press，但是只会在按下后的第 2 + 8x 帧判定为 repeat
    key = const_get(key) if key.is_a?(Symbol)
    RGM::Base.input_repeat(key)
  end

  def dir4
    # Checks the status of the directional buttons,
    # translates the data into a specialized 4-direction input format,
    # and returns the number pad equivalent (2, 4, 6, 8).
    # If no directional buttons are being pressed (or the equivalent), returns 0.

    return 2 if RGM::Base.input_press(DOWN)
    return 4 if RGM::Base.input_press(LEFT)
    return 6 if RGM::Base.input_press(RIGHT)
    return 8 if RGM::Base.input_press(UP)

    0
  end

  def dir8
    # Checks the status of the directional buttons,
    # translates the data into a specialized 8-direction input format,
    # and returns the number pad equivalent (1, 2, 3, 4, 6, 7, 8, 9).
    # If no directional buttons are being pressed (or the equivalent), returns 0.

    if RGM::Base.input_press(DOWN)
      return 1 if RGM::Base.input_press(LEFT)
      return 3 if RGM::Base.input_press(RIGHT)

      return 2
    end
    if RGM::Base.input_press(UP)
      return 7 if RGM::Base.input_press(LEFT)
      return 9 if RGM::Base.input_press(RIGHT)

      return 8
    end
    return 4 if RGM::Base.input_press(LEFT)
    return 6 if RGM::Base.input_press(RIGHT)

    0
  end

  def bind(sdl_key_name, key_name)
    # 将某个真实的键值（SDL Key）绑定到虚拟键上（即 Input 中定义的常量）
    sdl_key = RGM::SDL.const_get(sdl_key_name)

    # 1. 相同的 sdl_key 可以绑定到多个虚拟键上
    # 2. 如果 key = nil，则取消该 sdl_key 的全部绑定。
    if key_name.nil?
      RGM::Base.input_bind(sdl_key, nil)
      return
    end

    # 虚拟按键的状态由 std:array 管理，故其编号不能超过数组的大小。
    # 默认为 256，最大编号的虚拟按键是 Input::DEBUG = 255
    key = const_get(key_name)
    key += 256 if key < 0
    raise "Key code #{key} must be 0 - #{RGM::Max_Keycode}" if key && (key < 0 || key > RGM::Max_Keycode)

    RGM::Base.input_bind(sdl_key, key)
  end

  def last_sdl_key
    sdl_key = RGM::Base.input_last_press
    RGM::SDL.constants.each do |sym|
      next if RGM::SDL.const_get(sym) != sdl_key

      return [sdl_key, sym]
    end
    [0, nil]
  end

  def controller_bind(button_name, key_name, joy_index = 0)
    # 将某个真实的键值（SDL Key）绑定到虚拟键上（即 Input 中定义的常量）
    button = RGM::SDL.const_get(button_name)

    # 1. 相同的 button 可以绑定到多个虚拟键上
    # 2. 如果 key = nil，则取消该 button 的全部绑定。
    if key_name.nil?
      RGM::Base.controller_bind(button, nil, joy_index)
      return
    end

    # 虚拟按键的状态由 std:array 管理，故其编号不能超过数组的大小。
    # 默认为 256，最大编号的虚拟按键是 Input::DEBUG = 255
    key = const_get(key_name)
    key += 256 if key < 0
    raise "Key code #{key} must be 0 - #{RGM::Max_Keycode}" if key && (key < 0 || key > RGM::Max_Keycode)

    RGM::Base.controller_bind(button, key, joy_index)
  end

  def controller_axis_value(axis, joy_index = 0)
    axis = RGM::SDL.const_get(axis) if axis.is_a?(Symbol)
    # compare with RGM::Base::Controller_Axis_Threshold
    RGM::Base.controller_axis_value(axis, joy_index)
  end
end
