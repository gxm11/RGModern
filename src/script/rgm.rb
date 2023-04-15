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

module RGM
  module Config
    # Config_Path = "./config.ini"
    # Build_Mode = 1

    # 控制器的 Axis 超过此阈值才会触发特定的按键效果
    # Controller_Axis_Threshold = 8000

    # 包含 ruby 线程外的最多线程数，如果超过此值需要修改 base/signal.hpp 源码
    # Max_Workers = 8

    # 折叠过长的 tileset，使其高度不超过此值。此值不可修改。
    # 参见 rpgcache.rb 中的 RPG::Cache.tileset 方法
    # Tileset_Texture_Height = 8192

    # Battle_Test
    # Debug
    # Synchronized
    # Game_Title = "RGModern"
    # Resource_Prefix = "resource://"
    # Window_Width
    # Window_Height
    # Screen_Width
    # Screen_Height
    # Render_Driver = 0
    # Render_Driver_Name = "software"
  end

  module Base
    Temp = []

    module_function

    # keep_alive 将其他线程异步使用的 ruby 对象保存在 Temp 数组中，暂时阻止该对象的 GC。
    # 主要给 Bitmap 的 draw_text 使用。每次 Graphics.update 结束后，Temp 数组会清空。
    def keep_alive(object)
      Temp << object
      object
    end

    # 音乐播放结束后的自动回调，已在 Audio 模块中重新定义
    # def music_finish_callback; end

    # 其他音乐相关函数
    # def music_get_volume; end
    # def music_get_position(id); end
    # def music_create(id, path); end
    # def music_dispose(id); end
    # def music_play(id, iteration); end
    # def music_fade_in(id, iteration, duration); end
    # def music_set_volume(volumn); end
    # def music_set_position(position); end
    # def music_resume; end
    # def music_pause; end
    # def music_hale; end
    # def music_rewind; end
    # def music_fade_out(duration); end

    # 音效相关函数
    # def sound_get_state(id); end
    # def sound_get_channel(id); end
    # def sound_create(id, path); end
    # def sound_dispose(id); end
    # def sound_play(id, iteration); end
    # def sound_stop(id); end
    # def sound_fade_in(id, duration); end
    # def sound_fade_out(id, duration); end
    # def sound_set_volume(id, volume); end
    # def sound_set_pitch(id, speed, loop); end

    # 控制器相关函数
    # def controller_rumble(joy_index, low, high, duration); end
    # def controller_rumble_trigger(joy_index, left, right, duration); end

    # def new_id; end
    # def load_script(path); end
    # def load_embeded_file(path); end

    # def synchronize(worker_id); end
    # def check_delay(frame_rate); end

    # def set_title(title); end
    # def set_fullscreen(mode); end
    # def present_window; end
    # def resize_screen(width, height); end
    # def resize_window(width, height, scale_mode); end
  end

  module Ext
    module_function

    # 外部资源包相关函数
    # def external_check(path); end
    # def external_regist(path, password); end
    # def external_load(path); end

    # 输入法相关函数
    # def textinput_edit_text; end
    # def textinput_edit_pos; end
    # def textinput_edit_clear; end
    # def textinput_start(x, y, width, height); end
    # def textinput_stop; end

    # RGM::Ext.send(:async_ping, 100) { |ret| p ret.size, ret }
    def send(method_name, *args, &block)
      method(method_name).call(*args, async_bind(&block))
    end

    Callback_Blocks = {}
    @@callback_index = 0

    def async_bind(&block)
      @@callback_index += 1
      Callback_Blocks[@@callback_index] = block
      @@callback_index
    end

    def async_callback(id, buffer)
      proc = Callback_Blocks.delete(id)
      proc.call(buffer) if proc
      @@callback_index = 0 if Callback_Blocks.empty?
    end
  end

  module Word
  end

  module SDL
  end

  class Music
  end

  class Sound
  end

  module Driver
    Software = 0
    OpenGL = 1
    Direct3D9 = 2
    Direct3D11 = 3
  end
end

def force_utf8_encode(obj)
  case obj
  when Array
    obj.each { |item| force_utf8_encode(item) }
  when Hash
    obj.each { |_key, item| force_utf8_encode(item) }
  when String
    obj.force_encoding('utf-8')
  else
    if obj.class.name.start_with?('RPG::')
      obj.instance_variables.each do |name|
        item = obj.instance_variable_get(name)
        force_utf8_encode(item)
      end
    end
  end
end

def load_data(fn)
  fn = Finder.find(fn, :data)
  data = nil
  File.open(fn, 'rb') do |f|
    data = Marshal.load(f)
  end
  force_utf8_encode(data)
  data
end

def save_data(fn, obj)
  File.open(fn, 'wb') do |f|
    Marshal.dump(obj, f)
  end
end

def load_file(fn)
  File.open('./src/' + fn, 'rb') do |f|
    return f.read
  end
end

if RGM::Config::Build_Mode >= 3
  def load_data(fn)
    data = nil
    if fn.start_with?('Data/')
      bin = RGM::Base.load_embeded_file(fn)
      data = Marshal.load(bin)
    else
      fn = Finder.find(fn, :data)
      File.open(fn, 'rb') do |f|
        data = Marshal.load(f)
      end
    end
    force_utf8_encode(data)
    data
  end
end

if RGM::Config::Build_Mode >= 2
  def load_file(fn)
    RGM::Base.load_embeded_file(fn)
  end
end

def msgbox(text)
  RGM::Base.message_show(text.to_s)
end
