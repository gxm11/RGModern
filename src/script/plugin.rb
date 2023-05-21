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

# 辅助修改地图的插件。
# 鼠标左键：将当前图块的第 3 层设置为 384
# 鼠标右键：将当前图块的第 3 层设置为 0
# Ctrl+S ：保存地图
# 注释掉此脚本即可移除此功能
class Scene_Map
  alias origin_update update
  def update
    origin_update
    update_map_editor
  end

  def update_map_editor
    x, y = RGM::Ext::Mouse.position
    x = (x + $game_map.display_x / 4) / 32
    y = (y + $game_map.display_y / 4) / 32
    if RGM::Ext::Mouse.trigger?(:LEFT)
      $game_map.data[x, y, 2] = 384
    elsif RGM::Ext::Mouse.trigger?(:RIGHT)
      $game_map.data[x, y, 2] = 0
    elsif Input.press?(:CTRL) && Input.trigger?(:Y)
      save_map_data
    end
  end

  def save_map_data
    fn = format('Data/Map%03d.rxdata', $game_map.map_id)
    fn = Finder.find(fn, :data)
    map_data = load_data(fn)
    save_data(map_data, fn + '.bak')
    map_data.data = $game_map.data.clone
    save_data(map_data, fn)
    # 注释掉下面这一行取消弹窗
    msgbox '地图数据已覆盖。'
  end
end
