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
    module Window
      module_function

      def set_title(title)
        @@title = title
        RGM::Base.set_title(@@title)
      end

      def set_fps(fps)
        if fps.nil?
          RGM::Base.set_title(@@title)
          return
        end

        if fps < 0 || fps > 2 * Graphics.frame_rate
          RGM::Base.set_title(format('%s - Sampling...', @@title))
        else
          RGM::Base.set_title(format('%s - %.1f FPS', @@title, fps))
        end
      end

      def set_fullscreen(mode)
        @@fullscreen = mode.to_i
        RGM::Base.set_fullscreen(@@fullscreen)

        refresh_size
      end

      def resize(width, height, scale_mode = 0)
        @@width = width
        @@height = height
        # scale_mode 设置成 0,1,2 对应不同的缩放模式：nearest/linear/best
        # 设置成其它的值将不会缩放，而是居中显示
        @@scale_mode = scale_mode

        RGM::Base.resize_window(width.to_i, height.to_i, scale_mode.to_i)

        refresh_size
      end

      def refresh_size
        value = RGM::Base.display_bounds
        raise 'Failed to get display bounds' if value == 0

        @@width = value & 0xffff
        @@height = value >> 16
      end

      def cast_to_screen(x, y)
        screen_width = Graphics.width
        screen_height = Graphics.height

        if @@scale_mode >= 0 && @@scale_mode <= 2
          x = x * screen_width / @@width
          y = y * screen_height / @@height
        else
          x += (screen_width - @@width) / 2
          y += (screen_height - @@height) / 2
        end

        [x, y]
      end

      @@title = RGM::Config::Game_Title
      @@width = 640
      @@height = 480
      @@scale_mode = 0
      @@fullscreen = 0
    end
  end
end
