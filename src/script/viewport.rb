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

class Viewport
  # ---------------------------------------------------------------------------
  # The viewport class. Used when displaying sprites in one portion of the screen,
  # with no overflow into other regions.
  # ---------------------------------------------------------------------------

  # Viewport 也是 Drawable 类型，但是这里不跟 Sprite 等类一起实现
  # 在 C++ 层的构造也有区别，因为绑定到此 Viewport 的 Drawable 要
  # 用容器管理，该容器在 C++ 层为 viewport 类的成员变量 m_data。
  # Viewport 类没有实例变量 @viewport ，从而有独特的 API。
  attr_reader :visible, :z, :ox, :oy, :rect
  attr_accessor :color, :tone

  def self.create_finalizer(id)
    proc { RGM::Base.viewport_dispose(id) }
  end

  def initialize(x_or_rect, y = nil, width = nil, height = nil)
    # builtin members
    @id = RGM::Base.new_id
    @z = 0
    @disposed = false
    @visible = true

    # value type members
    @ox = 0
    @oy = 0

    # object type members
    @rect = y ? Rect.new(x_or_rect, y, width, height) : x_or_rect
    @color = Color.new(0, 0, 0, 0)
    @tone = Tone.new(0, 0, 0, 0)

    # flash 相关
    @flash_count = 0
    @flash_type = 0
    @flash_hidden = false
    @flash_color = Color.new(0, 0, 0, 0)

    # constructor / destructor
    RGM::Base.viewport_create(self)
    ObjectSpace.define_finalizer(self, self.class.create_finalizer(@id))
  end

  def disposed?
    @disposed
  end

  # 注意，dispose 方法会将 Viewport 内的 Drawable 全部释放。
  # 释放后仍然可以试图操作 Drawable 的属性，但是没有效果。
  def dispose
    @visible = false
    @disposed = true
    RGM::Base.viewport_dispose(@id)
  end

  def visible=(visible)
    @visible = visible & (!@disposed)
  end

  # value type members setters
  def z=(z)
    return @z if @z == z

    @z = RGM::Base.viewport_set_z(self, z.to_i) unless @disposed
  end

  def ox=(ox)
    ox = ox.to_i
    if @ox != ox
      @ox = ox
      RGM::Base.viewport_refresh_value(self, RGM::Word::Attribute_ox) unless @disposed
    end
    @ox
  end

  def oy=(oy)
    oy = oy.to_i
    if @oy != oy
      @oy = oy
      RGM::Base.viewport_refresh_value(self, RGM::Word::Attribute_oy) unless @disposed
    end
    @oy
  end

  def flash(color, duration)
    if color
      @flash_type = 0
      @flash_count = duration
      @flash_color = color
      @flash_hidden = false
    else
      @flash_type = 1
      @flash_count = duration
      @flash_color.set(0, 0, 0, 0)
      @flash_hidden = true
    end
    RGM::Base.viewport_refresh_value(self, RGM::Word::Attribute_flash_hidden) unless @disposed
  end

  def update
    # 更新 flash 的状态
    if @flash_count > 0
      @flash_color.alpha -= @flash_color.alpha / (1 + @flash_count) if @flash_type == 0
      @flash_count -= 1
      if @flash_count == 0
        @flash_type = 0
        @flash_color.set(0, 0, 0, 0)
        @flash_hidden = false
        RGM::Base.viewport_refresh_value(self, RGM::Word::Attribute_flash_hidden) unless @disposed
      end
    end
  end
end
