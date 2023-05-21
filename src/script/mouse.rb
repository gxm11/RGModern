# zlib License
#
# copyright (C) 2023 Guoxiaomi and Krimiston
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

module RGM
  module Ext
    module Mouse
      LEFT = 1
      MIDDLE = 2
      RIGHT = 3
      X1 = 4
      X2 = 5

      module_function

      def position
        x, y = raw_position
        Ext::Window.cast_to_screen(x, y)
      end

      def raw_position
        [Ext.mouse_x, Ext.mouse_y]
      end

      def press?(key)
        key = const_get(key) if key.is_a?(Symbol)

        Ext.mouse_press(key)
      end

      def trigger?(key)
        key = const_get(key) if key.is_a?(Symbol)

        Ext.mouse_trigger(key)
      end

      def double_click?(key)
        Ext.mouse_double_click(key, @@interval)
      end

      def double_click_interval=(interval)
        raise 'Invalid double click interval must be in range [1, 63]' if interval < 1 || interval > 63

        @@interval = interval
      end

      def wheel_down?
        Ext.mouse_wheel < 0
      end

      def wheel_up?
        Ext.mouse_wheel > 0
      end

      @@interval = Graphics.frame_rate / 3
    end
  end
end
