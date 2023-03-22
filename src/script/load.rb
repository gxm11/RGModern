# Copyright (c) 2022 Xiaomi Guo
# Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

if RGM::BuildMode >= 2
  def load_script(fn)
    RGM::Base.load_script('script/' + fn)
  end
else
  def load_script(fn)
    Kernel.load(fn)
  end
end

BEGIN {
  puts 'start ruby.'
  puts "build mode = #{RGM::BuildMode}"
}

at_exit do
  RGM::Base.synchronize(3)
  puts 'exit ruby.'
end

load_script 'rgm.rb'
load_script 'win32api.rb'
load_script 'imagesize.rb'
load_script 'sdl.rb'
load_script 'finder.rb'
load_script 'decorator.rb'
load_script 'builtin.rb'
load_script 'table.rb'
load_script 'font.rb'
load_script 'graphics.rb'
load_script 'bitmap.rb'
load_script 'palette.rb'
load_script 'viewport.rb'
load_script 'drawable.rb'
load_script 'textbox.rb'
load_script 'input.rb'
load_script 'debug.rb'
load_script 'music.rb'
load_script 'audio.rb'
load_script 'rpg.rb'
load_script 'config.rb'
# entry
load_script 'main.rb'
