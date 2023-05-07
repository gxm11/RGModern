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
  module Base
    Temp = []

    module_function

    # keep_alive 将其他线程异步使用的 ruby 对象保存在 Temp 数组中，暂时阻止该对象的 GC。
    # 主要给 Bitmap 的 draw_text 使用。每次 Graphics.update 结束后，Temp 数组会清空。
    def keep_alive(object)
      Temp << object
      object
    end

    # 音乐播放结束后的自动回调，已在 Audio 模块中重新定义
    def music_finish_callback; end
  end

  module Ext
    module_function

    # RGM::Ext.send(:async_ping, 100) { |ret| p ret.size, ret }
    def send(method_name, *args, &block)
      method(method_name).call(*args, async_bind(&block))
    end

    Callback_Blocks = {}
    @@callback_index = 0

    def async_bind(&block)
      @@callback_index += 1
      Callback_Blocks[@@callback_index] = block
      @@callback_index
    end

    def async_callback(id, buffer)
      proc = Callback_Blocks.delete(id)
      proc.call(buffer) if proc
      @@callback_index = 0 if Callback_Blocks.empty?
    end
  end

  module Word
  end

  module SDL
  end

  class Music
  end

  class Sound
  end

  module Driver
    Software = 0
    OpenGL = 1
    Direct3D9 = 2
    Direct3D11 = 3
  end
end
