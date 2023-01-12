# Copyright (c) 2022 Xiaomi Guo
# Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

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
