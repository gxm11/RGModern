# Copyright (c) 2022 Xiaomi Guo
# Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

module Finder
  module_function

  Load_Path = {
    font: [nil, './Fonts', 'C:/Windows/Fonts'],
    image: [nil],
    music: [nil],
    sound: [nil],
    data: [nil],
    none: [nil]
  }

  if RGM::BuildMode <= 1
    Load_Path.each_key do |type|
      next if type == :none

      Load_Path[type] += ['./resource/', './Project1/']
    end
  end

  Suffix = {
    font: ['', '.ttf', '.ttc'],
    image: ['', '.png', '.jpg', '.bmp', '.webp', '.gif', '.tiff'],
    music: ['', '.mp3', '.wav', '.ogg', '.mid'],
    sound: ['', '.wav', '.ogg'],
    data: ['', '.rxdata'],
    none: ['']
  }

  Cache = {}

  FontPaths = {}

  PictureShapes = {}

  def find(filename, key = :none)
    return Cache[filename] if Cache[filename]

    if key == :image
      Suffix[key].each do |extname|
        path = filename + extname
        next unless RGM::Ext.external_check(path)

        Cache[filename] = RGM::Resource_Prefix + path
        # puts "#{filename} -> #{Cache[filename]}"
        return Cache[filename]
      end
    end

    filename = FontPaths[filename] || filename if key == :font

    Load_Path[key].each do |directory|
      Suffix[key].each do |extname|
        path = File.expand_path(filename + extname, directory)
        if File.exist?(path)
          Cache[filename] = path
          return Cache[filename]
        end
      end
    end

    case key
    when :font
      puts "[警告] 找不到字体: #{filename}"
    else
      raise "Finder cannot find valid path for #{filename}"
    end
    Cache[filename] = nil
  end

  def regist(path, password)
    RGM::Ext.external_regist(path, password)
  end

  def get_picture_shape(path)
    unless PictureShapes[path]
      if path.start_with?(RGM::Resource_Prefix)
        content = RGM::Ext.external_load(path)
        PictureShapes[path] = Imagesize.load_raw(content)
      else
        PictureShapes[path] = Imagesize.load(path)
      end
    end
    PictureShapes[path]
  end
end
