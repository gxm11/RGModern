# zlib License

# copyright (C) 2023 Xiaomi Guo and Krimiston

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

class Bitmap
  # ---------------------------------------------------------------------------
  # Bitmap < Object
  # ---------------------------------------------------------------------------
  # The bitmap class. Bitmaps are expressions of so-called graphics.
  #
  # Sprites (Sprite) and other objects must be used to display bitmaps on the screen.
  # ---------------------------------------------------------------------------

  # ---------------------------------------------------------------------------
  # Properties
  # ---------------------------------------------------------------------------
  # width (read only)
  # Gets the bitmap width.
  #
  # height (read only)
  # Gets the bitmap height.
  #
  # font
  # The font (Font) used to draw a string with the draw_text method.
  # ---------------------------------------------------------------------------

  attr_reader :id, :width, :height
  attr_accessor :font

  def self.create_finalizer
    proc { |object_id|
      RGM::Base.bitmap_dispose(object_id)
    }
  end

  # ---------------------------------------------------------------------------
  # Bitmap.new(filename)
  # Loads the graphic file specified in filename and creates a bitmap object.
  #
  # Also automatically searches files included in RGSS-RTP and encrypted archives. File extensions may be omitted.
  #
  # Bitmap.new(width, height)
  # Creates a bitmap object with the specified size.
  # ---------------------------------------------------------------------------
  def initialize(width_or_path, height = nil)
    @id = object_id
    @disposed = false

    if height
      @width = width_or_path.to_i
      @height = height.to_i
      RGM::Base.bitmap_create(@id, @width, @height)
    else
      path = Finder.find(width_or_path, :image)
      # 保存图片位置的字符串 path（内蕴含的 char*）会被传递给渲染线程，
      # 需要保证在渲染线程使用之前 path 不被 GC。
      # 但由于 Finder 会缓存寻址结果，path 永远不被 GC。故不需要调用 RGM::Base.keep_alive。
      # RGM::Base.keep_alive(path)
      RGM::Base.bitmap_create(@id, path, nil)
      # 以路径指定的图片素材，通常不会在游戏过程中改变尺寸，所以缓存尺寸提升性能。
      @width, @height = Finder.get_picture_shape(path)
    end
    @font = Font.new

    ObjectSpace.define_finalizer(self, self.class.create_finalizer)
  end

  # Frees the bitmap. If the bitmap has already been freed, does nothing.
  def dispose
    RGM::Base.bitmap_dispose(@id) unless @disposed
    @disposed = true
  end

  # Returns TRUE if the bitmap has been freed.
  def disposed?
    @disposed
  end

  # Gets the bitmap rectangle (Rect).
  def rect
    Rect.new(0, 0, @width, @height)
  end

  # Gets the box (Rect) used when drawing a string str with the draw_text method. Does not include the angled portions of italicized text.
  def text_size(str)
    if str.empty?
      Rect.new(0, 0, 0, 0)
    else
      value = RGM::Base.bitmap_text_size(@font, str.to_s)
      Rect.new(0, 0, value & 0xffff, value >> 16)
    end
  end

  # Performs a block transfer from the src_bitmap box src_rect (Rect) to the specified bitmap coordinates (x, y).
  # opacity can be set from 0 to 255.
  def blt(x, y, src_bitmap, rect, opacity = 255)
    RGM::Base.bitmap_blt(@id, x.to_i, y.to_i, src_bitmap.id, rect, opacity.to_i)
  end

  # Performs a block transfer from the src_bitmap box src_rect (Rect) to the specified bitmap box dest_rect (Rect).
  # opacity can be set from 0 to 255.
  def stretch_blt(dest_rect, src_bitmap, src_rect, opacity = 255)
    RGM::Base.bitmap_stretch_blt(@id, dest_rect, src_bitmap.id, src_rect, opacity.to_i)
  end

  # fill_rect(x, y, width, height, color)
  # fill_rect(rect, color)
  # Fills the bitmap box (x, y, width, height) or rect (Rect) with color (Color).
  def fill_rect(*args)
    if args.first.is_a?(Rect)
      r = args[0]
      c = args[1]
    else
      r = Rect.new(args[0], args[1], args[2], args[3])
      c = args[4]
    end
    RGM::Base.bitmap_fill_rect(@id, r, c)
  end

  # Clears the entire bitmap.
  def clear
    RGM::Base.bitmap_fill_rect(@id, Rect.new(0, 0, @width, @height), Color.new(0, 0, 0, 0))
  end

  # draw_text(x, y, width, height, str[, align])
  # draw_text(rect, str[, align])
  # Draws a string str in the bitmap box (x, y, width, height) or rect (Rect).
  # If the text length exceeds the box's width, the text width will automatically be reduced by up to 60 percent.
  # Horizontal text is left-aligned by default; set align to 1 to center the text and to 2 to right-align it. Vertical text is always centered.
  # As this process is time-consuming, redrawing the text with every frame is not recommended.
  def draw_text(*args)
    if args.first.is_a?(Rect)
      rect, text, align = args

    else
      x, y, w, h, text, align = args
      rect = Rect.new(x, y, w, h)
    end
    text = text.to_s
    RGM::Base.keep_alive(text)
    RGM::Base.bitmap_draw_text(@id, @font, rect, text, align || 0)
  end

  # Changes the bitmap's hue within 360 degrees of displacement.
  # This process is time-consuming. Furthermore, due to conversion errors, repeated hue changes may result in color loss.
  def hue_change(hue)
    RGM::Base.bitmap_hue_change(@id, hue.to_i)
  end

  # get_pixel(x, y)
  # Gets the color (Color) at the specified pixel (x, y).
  # get_pixel 运行较慢的原因如下：
  # 1. 该 Bitmap 可能有绘制任务等待异步执行，必须等待绘制完毕才能获得正确的颜色；
  # 2. 获取 texture 中的颜色需要从 GPU 中读取数据，效率极低；
  # 3. RGSS 里没有调用过该方法。
  # 故这个方法不建议经常使用。
  # RGM 实现了 Palette 类来方便像素操作，Palette 的数据操作都是同步的。
  def get_pixel(x, y)
    puts '[Warning] Bitmap#get_pixel is a very slow operation, and should not be used frequently.'
    c = RGM::Base.bitmap_get_pixel(@id, x.to_i, y.to_i)
    Color.new(c & 255, (c >> 8) & 255, (c >> 16) & 255, c >> 24)
  end

  if RGM::Render_Driver == RGM::Driver::Direct3D11
    if RGM::Build_Mode >= 2
      def get_pixel(_x, _y)
        Color.new(0, 0, 0, 0)
      end
    else
      def get_pixel(_x, _y)
        puts '[Warning] Bitmap#get_pixel may return invalid result with Direct3d11 driver.'
        puts 'Bitmap#get_pixel is deprecated in this situation, Use Palette#get_pixel instead.'
        raise
      end
    end
  end

  # set_pixel(x, y, color)
  # Sets the specified pixel (x, y) to color (Color).
  def set_pixel(x, y, color)
    fill_rect(x.to_i, y.to_i, 1, 1, color)
  end

  def clone
    b = Bitmap.new(@width, @height)
    b.blt(0, 0, self, rect)
    b
  end

  def save_png(path)
    path = path.to_s
    RGM::Base.keep_alive(path)
    RGM::Base.bitmap_save_png(@id, path)
  end

  def capture_screen
    RGM::Base.bitmap_capture_screen(@id)
  end
end
