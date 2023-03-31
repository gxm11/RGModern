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

class Palette
  # Palette 即调色盘，是对 SDL_Surface 的封装。
  # 与 Bitmap 不同，该类的操作都是实时的

  attr_reader :id, :width, :height

  def self.create_finalizer
    proc { |object_id|
      RGM::Base.palette_dispose(object_id)
    }
  end

  def initialize(width_or_path, height = nil)
    @id = object_id
    @disposed = false

    if height
      @width = width_or_path.to_i
      @height = height.to_i
      RGM::Base.palette_create(@id, @width, @height)
    else
      path = Finder.find(width_or_path, :image)
      RGM::Base.palette_create(@id, path, nil)
      @width, @height = Finder.get_picture_shape(path)
    end
    ObjectSpace.define_finalizer(self, self.class.create_finalizer)
  end

  def dispose
    RGM::Base.palette_dispose(@id) unless @disposed
    @disposed = true
  end

  def disposed?
    @disposed
  end

  def get_pixel(x, y)
    c = RGM::Base.palette_get_pixel(@id, x.to_i, y.to_i)
    Color.new(c & 255, (c >> 8) & 255, (c >> 16) & 255, c >> 24)
  end

  def set_pixel(x, y, color)
    RGM::Base.palette_set_pixel(@id, x.to_i, y.to_i, color)
  end

  def save_png(path)
    RGM::Base.palette_save_png(@id, path.to_s)
  end

  def convert_to_bitmap(rect = nil)
    rect ||= Rect.new(0, 0, @width, @height)
    b = Bitmap.new(rect.width, rect.height)
    RGM::Base.palette_convert_to_bitmap(@id, b.id, rect)
    b
  end
end
