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

class Color
  # ---------------------------------------------------------------------------
  # Color < Object
  # ---------------------------------------------------------------------------
  # The RGBA color class. Each component is handled with a floating point value (Float).

  # ---------------------------------------------------------------------------
  # Properties
  # ---------------------------------------------------------------------------
  # red
  # The red value (0-255). Values out of range are automatically corrected.
  #
  # green
  # The green value (0-255). Values out of range are automatically corrected.
  #
  # blue
  # The blue value (0-255). Values out of range are automatically corrected.
  #
  # alpha
  # The alpha value (0-255). Values out of range are automatically corrected.
  # ---------------------------------------------------------------------------
  attr_reader :red, :green, :blue, :alpha

  # -------------------------------------------------------------------------
  # Creates a Color object. If alpha is omitted, it is assumed at 255.
  # -------------------------------------------------------------------------
  def initialize(red, green, blue, alpha = 255)
    set(red, green, blue, alpha)
  end

  # Sets all components at once.
  def set(red, green, blue, alpha = 255)
    self.red = red
    self.green = green
    self.blue = blue
    self.alpha = alpha
  end

  def _dump(_depth = 1)
    [@red, @green, @blue, @alpha].pack('E4')
  end

  def self._load(s)
    obj = Color.new(0, 0, 0, 0)
    obj.set(*s.unpack('E4').collect(&:to_i))
    obj
  end

  def inspect
    format('#<Color:%d> [%d, %d, %d, %d]', object_id, @red, @green, @blue, @alpha)
  end

  def ==(other)
    other.is_a?(Color) && @red == other.red && @green == other.green && @blue == other.blue && @alpha == other.alpha
  end
end

RGM::Base.decorate_builtin_setter(Color, :red, 0, 255)
RGM::Base.decorate_builtin_setter(Color, :green, 0, 255)
RGM::Base.decorate_builtin_setter(Color, :blue, 0, 255)
RGM::Base.decorate_builtin_setter(Color, :alpha, 0, 255)

class Tone
  # ---------------------------------------------------------------------------
  # Tone < Object
  # ---------------------------------------------------------------------------
  # The color tone class. Each component is handled with a floating point value (Float).
  # ---------------------------------------------------------------------------

  # ---------------------------------------------------------------------------
  # Properties
  # ---------------------------------------------------------------------------
  # red
  # The red balance adjustment value (-255 to 255). Values out of range are automatically corrected.
  #
  # green
  # The green balance adjustment value (-255 to 255). Values out of range are automatically corrected.
  #
  # blue
  # The blue balance adjustment value (-255 to 255). Values out of range are automatically corrected.
  #
  # gray
  # The grayscale filter strength (0 to 255). Values out of range are automatically corrected.
  #
  # When this value is not 0, processing time is significantly longer than when using tone balance adjustment values alone.
  # ---------------------------------------------------------------------------
  attr_reader :red, :green, :blue, :gray

  # ---------------------------------------------------------------------------
  # Creates a Tone object. If gray is omitted, it is assumed at 0.
  # ---------------------------------------------------------------------------
  def initialize(red, green, blue, gray)
    set(red, green, blue, gray)
  end

  # Sets all components at once.
  def set(red, green, blue, gray)
    self.red = red
    self.green = green
    self.blue = blue
    self.gray = gray
  end

  def _dump(_depth = 1)
    [@red, @green, @blue, @gray].pack('E4')
  end

  def self._load(s)
    obj = Tone.new(0, 0, 0, 0)
    obj.set(*s.unpack('E4').collect(&:to_i))
    obj
  end

  def inspect
    format('#<Tone:%d> [%d, %d, %d, %d]', object_id, @red, @green, @blue, @gray)
  end

  def ==(other)
    other.is_a?(Tone) && @red == other.red && @green == other.green && @blue == other.blue && @gray == other.gray
  end
end

RGM::Base.decorate_builtin_setter(Tone, :red, -255, 255)
RGM::Base.decorate_builtin_setter(Tone, :green, -255, 255)
RGM::Base.decorate_builtin_setter(Tone, :blue, -255, 255)
RGM::Base.decorate_builtin_setter(Tone, :gray, 0, 255)

class Rect
  # ---------------------------------------------------------------------------
  # Rect < Object
  # ---------------------------------------------------------------------------
  # The rectangle class.
  # ---------------------------------------------------------------------------

  # ---------------------------------------------------------------------------
  # Properties
  # ---------------------------------------------------------------------------
  # x
  # The X-coordinate of the rectangle's upper left corner.
  #
  # y
  # The Y-coordinate of the rectange's upper left corner.
  #
  # width
  # The rectangle's width.
  #
  # height
  # The rectangle's height.
  # ---------------------------------------------------------------------------
  attr_accessor :x, :y, :width, :height

  # ---------------------------------------------------------------------------
  # Creates a new Rect object.
  # ---------------------------------------------------------------------------
  def initialize(x, y, width, height)
    set(x, y, width, height)
  end

  # Sets all parameters at once.
  def set(x, y, width, height)
    raise ArgumentError if width < 0 || height < 0

    @x = x
    @y = y
    @width = width
    @height = height
  end

  def inspect
    format('#<Rect:%d> [%d, %d, %d, %d]', object_id, @x, @y, @width, @height)
  end

  def empty
    set(0, 0, 0, 0)
  end

  def _dump(_depth = 1)
    [@x, @y, @width, @height].pack('l4')
  end

  def self._load(s)
    obj = Rect.new(0, 0, 0, 0)
    obj.set(*s.unpack('l4'))
    obj
  end

  def ==(other)
    other.is_a?(Rect) && @x == other.x && @y == other.y && @width == other.width && @height == other.height
  end
end
