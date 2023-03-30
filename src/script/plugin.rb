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

# 载入数据库
$data_actors        = load_data('Data/Actors.rxdata')
$data_classes       = load_data('Data/Classes.rxdata')
$data_skills        = load_data('Data/Skills.rxdata')
$data_items         = load_data('Data/Items.rxdata')
$data_weapons       = load_data('Data/Weapons.rxdata')
$data_armors        = load_data('Data/Armors.rxdata')
$data_enemies       = load_data('Data/Enemies.rxdata')
$data_troops        = load_data('Data/Troops.rxdata')
$data_states        = load_data('Data/States.rxdata')
$data_animations    = load_data('Data/Animations.rxdata')
$data_tilesets      = load_data('Data/Tilesets.rxdata')
$data_common_events = load_data('Data/CommonEvents.rxdata')
$data_system        = load_data('Data/System.rxdata')
# 生成系统对像
$game_system = Game_System.new
# 重置测量游戏时间用的画面计数器
Graphics.frame_count = 0
# 生成各种游戏对像
$game_temp          = Game_Temp.new
$game_system        = Game_System.new
$game_switches      = Game_Switches.new
$game_variables     = Game_Variables.new
$game_self_switches = Game_SelfSwitches.new
$game_screen        = Game_Screen.new
$game_actors        = Game_Actors.new
$game_party         = Game_Party.new
$game_troop         = Game_Troop.new
$game_map           = Game_Map.new
$game_player        = Game_Player.new
# 设置初期同伴位置
$game_party.setup_starting_members
# 设置初期位置的地图
$game_map.setup($data_system.start_map_id)
# 主角向初期位置移动
$game_player.moveto($data_system.start_x, $data_system.start_y)
# 刷新主角
$game_player.refresh
# 执行地图设置的 BGM 与 BGS 的自动切换
$game_map.autoplay
# 刷新地图 (执行并行事件)
$game_map.update
# 切换地图画面
$scene = Scene_Map.new

$scene.main
exit
