# zlib License

# copyright (C) [2023] [Xiaomi Guo]

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
