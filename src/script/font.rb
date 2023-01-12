# Copyright (c) 2022 Xiaomi Guo
# Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

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
