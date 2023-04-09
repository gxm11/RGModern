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

  module Driver
    Software = 0
    OpenGL = 1
    Direct3D9 = 2
    Direct3D11 = 3
  end

  # 折叠过长的 tileset，使其高度不超过此值。此值不可修改。
  # 参见 rpgcache.rb 中的 RPG::Cache.tileset 方法
  Tileset_Texture_Height = 8192
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

if RGM::Build_Mode >= 3
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

if RGM::Build_Mode >= 2
  def load_file(fn)
    RGM::Base.load_embeded_file(fn)
  end
end

def msgbox(text)
  RGM::Base.message_show(text.to_s)
end
