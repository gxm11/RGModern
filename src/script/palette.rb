# Copyright (c) 2022 Xiaomi Guo
# Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

class Palette
  # Palette 即调色盘，是对 SDL_Surface 的封装。
  # 与 Bitmap 不同，该类的操作都是实时的

  attr_reader :id, :width, :height

  def self.create_finalizer
    proc { |object_id|
      RGM::Base.palette_dispose(object_id)
    }
  end

  def initialize(width_or_path, height = nil)
    @id = object_id
    @disposed = false

    if height
      @width = width_or_path
      @height = height
      RGM::Base.palette_create(@id, @width, @height)
    else
      path = Finder.find(width_or_path, :image)
      RGM::Base.palette_create(@id, path, nil)
      if path.start_with?(RGM::Resource_Prefix)
        content = RGM::Ext.external_load(path)
        @width, @height = Imagesize.load_raw(content)
      else
        @width, @height = Imagesize.load(path)
      end
    end
    ObjectSpace.define_finalizer(self, self.class.create_finalizer)
  end

  def dispose
    RGM::Base.palette_dispose(@id) unless @disposed
    @disposed = true
  end

  def disposed?
    @disposed
  end

  def get_pixel(x, y)
    c = RGM::Base.palette_get_pixel(@id, x, y)
    Color.new(c & 255, (c >> 8) & 255, (c >> 16) & 255, c >> 24)
  end

  def set_pixel(x, y, color)
    RGM::Base.palette_set_pixel(@id, x, y, color)
  end

  def save_png(path)
    RGM::Base.palette_save_png(@id, path)
  end
end
