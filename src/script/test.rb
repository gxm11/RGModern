# zlib License

# copyright (C) [2023] [Xiaomi Guo]

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

def test_loop
  t = Time.now
  n = 0
  3000.times do
    n += 1
    Graphics.update
    Input.update
    break if Input.trigger?(Input::C)

    yield if block_given?
  end
  puts format('fps: %.2f', n / (Time.now - t))
end

Graphics.frame_reset

$a = []

$game_map = Game_Map.new
$game_map.setup(1)

# 生成元件地图
@tilemap = Tilemap.new(@viewport1)
@tilemap.tileset = RPG::Cache.tileset($game_map.tileset_name)
for i in 0..6
  autotile_name = $game_map.autotile_names[i]
  @tilemap.autotiles[i] = RPG::Cache.autotile(autotile_name)
end
@tilemap.map_data = $game_map.data
@tilemap.priorities = $game_map.priorities

Graphics.update
Graphics.snap_to_bitmap.save_png('current.png')

test_loop
