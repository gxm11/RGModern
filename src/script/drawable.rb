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

class Sprite
  # ---------------------------------------------------------------------------
  # The sprite class. Sprites are the basic concept used to display characters,
  # etc. on the game screen.
  # ---------------------------------------------------------------------------
  attr_reader :bitmap,
              # value
              :x, :y, :ox, :oy, :zoom_x, :zoom_y, :angle,
              :mirror, :bush_depth, :opacity, :blend_type,
              :scale_mode, :flash_hidden
  # object
  attr_accessor :color, :tone, :src_rect

  def initialize
    @bitmap = nil
    # value
    @x = @y = @ox = @oy = 0
    @zoom_x = @zoom_y = 1.0
    @angle = 0.0
    @mirror = false
    @bush_depth = 0
    @opacity = 255
    @blend_type = 0
    @scale_mode = 0
    # object
    @color = Color.new(0, 0, 0, 0)
    @tone = Tone.new(0, 0, 0, 0)
    @src_rect = Rect.new(0, 0, 0, 0)
    # flash 相关
    @flash_count = 0
    @flash_type = 0
    @flash_hidden = false
    @flash_color = Color.new(0, 0, 0, 0)
    # C++ 层对象的内存地址
    @data = nil
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
    RGM::Base.sprite_refresh_value(@data, RGM::Word::Attribute_flash_hidden)
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
        RGM::Base.sprite_refresh_value(@data, RGM::Word::Attribute_flash_hidden)
      end
    end
  end
end

class Plane
  # ---------------------------------------------------------------------------
  # The Plane class. Planes are special sprites that tile bitmap patterns across
  # the entire screen, and are used to display panoramas and fog.
  # ---------------------------------------------------------------------------
  attr_reader :bitmap,
              # value
              :ox, :oy, :zoom_x, :zoom_y, :opacity, :blend_type, :scale_mode
  # object
  attr_accessor :color, :tone

  def initialize
    # value
    @bitmap = nil
    @ox = @oy = 0
    @zoom_x = @zoom_y = 1.0
    @opacity = 255
    @blend_type = 0
    @scale_mode = 0
    # object
    @color = Color.new(0, 0, 0, 0)
    @tone = Tone.new(0, 0, 0, 0)
    # C++ 层对象的内存地址
    @data = nil
  end
end

class Window
  # ---------------------------------------------------------------------------
  # The game window class. Created internally from multiple sprites.
  # ---------------------------------------------------------------------------
  attr_reader :windowskin, :contents,
              # value
              :stretch, :active, :pause, :x, :y, :width, :height,
              :ox, :oy, :opacity, :back_opacity, :contents_opacity
  # object
  attr_accessor :cursor_rect

  def initialize
    # value
    @windowskin = nil
    @contents = nil
    @stretch = true
    @active = true
    @pause = false
    @x = @y = @width = @height = @ox = @oy = 0
    @opacity = @back_opacity = @contents_opacity = 255
    @update_count = 0
    @cursor_count = 0
    # object
    @cursor_rect = Rect.new(0, 0, 0, 0)
    # C++ 层对象的内存地址
    @data = nil
  end

  def update
    @update_count = (@update_count + 1) % 32
    @cursor_count = @active ? (@cursor_count + 1) % 32 : 15
    RGM::Base.window_refresh_value(@data, RGM::Word::Attribute_update_count)
    RGM::Base.window_refresh_value(@data, RGM::Word::Attribute_cursor_count)
  end
end

class Tilemap
  # ---------------------------------------------------------------------------
  # The class governing tilemaps. Tilemaps are a specialized concept used in 2D
  # game map displays, created internally from multiple sprites.
  # ---------------------------------------------------------------------------
  attr_reader :tileset,
              # value
              :map_data, :flash_data, :priorities, :ox, :oy, :repeat_x, :repeat_y
  # object
  attr_accessor :autotiles

  def initialize
    @autotiles = []

    @tileset = nil
    @map_data = nil
    @flash_data = nil
    @priorities = nil
    @ox = @oy = 0
    @repeat_x = true
    @repeat_y = true
    @update_count = 0
    # C++ 层对象的内存地址
    @data = nil
  end

  def update
    @update_count = (@update_count + 1) % 1_073_741_824
    RGM::Base.tilemap_refresh_value(@data, RGM::Word::Attribute_update_count)
  end
end

# apply decorator
RGM::Base.decorate_drawable(Sprite)
RGM::Base.decorate_drawable(Plane)
RGM::Base.decorate_drawable(Window)
RGM::Base.decorate_drawable(Tilemap)

class Sprite
  def bitmap=(bitmap)
    raise ArgumentError, 'Argument 1 should be nil or Bitmap.' if bitmap && !bitmap.is_a?(Bitmap)

    if @bitmap != bitmap
      @bitmap = bitmap
      RGM::Base.sprite_refresh_value(@data, RGM::Word::Attribute_bitmap) unless @disposed
      # 在更新 bitmap 后，重设 src_rect
      @src_rect = @bitmap.rect if @bitmap
    end
    @bitmap
  end
end
