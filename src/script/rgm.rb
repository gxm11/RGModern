# Copyright (c) 2022 Xiaomi Guo
# Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

module RGM
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
    def self.music_finish_callback; end
  end

  module Word
  end

  module SDL
  end

  class Music
  end

  class Sound
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

if RGM::BuildMode >= 3
  def load_data(fn)
    data = nil
    if fn.start_with?('Data/')
      data = RGM::Base.load_data(fn)
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

if RGM::BuildMode >= 2
  def load_file(fn)
    RGM::Base.load_file(fn)
  end
end

def msgbox(text)
  RGM::Base.message_show(text.to_s)
end
