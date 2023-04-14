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

class Font
  # 与 RGSS 不同，Font 的 name 属性被当作 path 使用
  @@default_name = 'simhei'
  @@default_size = 22
  @@default_bold = false
  @@default_italic = false
  @@default_color = Color.new(255, 255, 255, 255)
  @@default_underlined = false
  @@default_strikethrough = false
  @@default_solid = false

  class << self
    def exist?(name)
      !!Finder.find(name, :font)
    end

    Code_Default = <<~END
      def default_key=(key)
        @@default_key = key
      end

      def default_key
        @@default_key
      end
    END

    %w[name size bold italic color].each do |attribute|
      eval(Code_Default.gsub('key', attribute))
    end
  end

  attr_reader :id, :name
  attr_accessor :size, :bold, :italic, :color, :underlined, :strikethrough, :solid

  def initialize(name = @@default_name, size = @@default_size)
    self.name = name
    @size = size
    @bold = @@default_bold
    @italic = @@default_italic
    @color = @@default_color
    @underlined = @@default_underlined
    @strikethrough = @@default_strikethrough
    @solid = @@default_solid
  end

  def name=(font_name)
    font_name = [font_name] unless font_name.is_a?(Array)
    font_name.each do |n|
      path = Finder.find(n, :font)
      next unless path

      @name = n
      @id = RGM::Base.font_create(path)
      return n
    end

    raise 'Default font not found.' if font_name == @@default_name

    puts 'Font not found, use default font instead.'
    puts @@default_name
    self.name = @@default_name
  end
end
