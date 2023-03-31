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
$data_tilesets = load_data('Data/Tilesets')
File.open('table.log', 'w') do |f|
  map = load_data('Data/Map014')
  tileset = $data_tilesets[map.tileset_id]
  f.puts tileset.tileset_name
  f.puts tileset.autotile_names

  f.puts tileset.priorities.inspect
  f.puts tileset.passages.inspect
  f.puts tileset.terrain_tags.inspect
  t = map.data
  for z in 0...t.zsize
    f.print "z = %d\n" % z
    for y in 0...t.ysize
      for x in 0...t.xsize
        f.print '%6d' % t[x, y, z]
      end
      f.print "\n"
    end
  end
end
