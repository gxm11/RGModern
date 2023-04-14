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

# -------------------------------------------------------------------------------------------------
# RPG::Cache
# -------------------------------------------------------------------------------------------------
module RPG
  module Cache
    def self.reload
      @cache.each_pair do |key, value|
        next unless key.is_a?(String)
        next if value.disposed?

        next unless Finder::Cache.include?(key)

        bitmap = if key.start_with?('Graphics/Tilesets/')
                   make_tileset(key)
                 else
                   Bitmap.new(key)
                 end
        value.blt(0, 0, bitmap, value.rect, 255)
        RGM::Base.bitmap_reload_autotile(value.id, false)
      end

      @cache.each_pair do |key, value|
        next unless key.is_a?(Array)
        next if value.disposed?

        path = key[0]
        hue = key[1]

        bitmap = @cache[path].clone
        bitmap.hue_change(hue)
        value.blt(0, 0, bitmap, value.rect, 255)
      end
    end

    def self.tileset(filename)
      raise 'Invalid file name for tileset resource.' if filename.empty?

      path = 'Graphics/Tilesets/' + filename
      unless @cache.include?(path) && !@cache[path].disposed?
        #  make tileset
        @cache[path] = make_tileset(path)
      end
      # return cache
      @cache[path]
    end

    # 针对非常长的 tileset 进行优化，使其支持到 262,144 的长度
    def self.make_tileset(path)
      figure = Palette.new(path)
      # 实际上由于 Table 里的值最大为 32767，则 tileid <= 32767
      # 那么 tileset 最高也不过 (32767 - 384) / 8 * 32  = 129,532
      raise 'Height of tileset must less than 129,532' if figure.height > 129_532

      # keep palette alive before the tileset texture is created.
      RGM::Base.keep_alive(figure)

      h = RGM::Config::Tileset_Texture_Height
      n = (figure.height - 1) / h + 1
      if n > 1
        tileset = Bitmap.new(n * 256, h)
        n.times.each do |i|
          r = Rect.new(0, i * h, 256, h)
          b = figure.convert_to_bitmap(r)
          tileset.blt(i * 256, 0, b, b.rect)
        end
      else
        tileset = figure.convert_to_bitmap
      end
      tileset
    end
  end
end
