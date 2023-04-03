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

        bitmap = Bitmap.new(key)
        value.blt(0, 0, bitmap, value.rect, 255)
        # RGM::Base.bitmap_dispose(value.id + 1)
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
  end
end
