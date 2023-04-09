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

if RGM::Build_Mode >= 2
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
  puts "resource prefix = #{RGM::Resource_Prefix}"
  driver_names = %w[Software OpenGL Direct3D9 Direct3D11]
  puts "render driver = #{RGM::Render_Driver} (#{driver_names[RGM::Render_Driver]})"
  puts "build mode = #{RGM::Build_Mode}"
}

at_exit do
  RGM::Max_Threads.times do |i|
    RGM::Base.synchronize(i)
  end
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
load_script 'sound.rb'
load_script 'audio.rb'
load_script 'rpg.rb'
load_script 'rpgcache.rb'
load_script 'config.rb'
# entry
load_script 'main.rb'
