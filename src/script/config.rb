# Copyright (c) 2022 Xiaomi Guo
# Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

unless File.exist?(RGM::Default_Config)
  File.open(RGM::Default_Config, 'w') do |f|
    f << load_file('config.ini')
  end
end

File.open(RGM::Default_Config, 'r') do |f|
  flag = nil
  f.each_line do |line|
    line = line.strip
    line.force_encoding('UTF-8')

    # flags
    if line.start_with?('[')
      flag = :Game if line == '[Game]'
      flag = :System if line == '[System]'
      flag = :Keymap if line == '[Keymap]'
      flag = :Font if line == '[Font]'
      next
    end

    if flag == :Game
      if line =~ /^Title=(.+)$/
        title = Regexp.last_match(1)
        Graphics.set_title(title)
      end
      if line =~ /^RTP\d+=(.+)$/
        rtp = Regexp.last_match(1)
        next unless File.exist?(rtp)

        %i[image music sound].each do |type|
          Finder::Load_Path[type] << rtp
        end
      end
    end

    if flag == :System
      Audio.disable_music if line == 'MUSIC=OFF'
      Audio.disable_sound if line == 'SOUND=OFF'
      Graphics.set_fullscreen(Regexp.last_match(1).to_i) if line =~ /^FULLSCREEN=(\d+)/
      if line == 'MESSAGEBOX=ON'
        def p(*args)
          msgbox(args.collect(&:to_s).join("\n"))
        end
      end
    end

    if flag == :Keymap
      if line =~ /^(K_\w+)=(\w+)$/
        Input.bind Regexp.last_match(1).to_sym, Regexp.last_match(2).to_sym
      elsif line =~ /^(K_\w+)=$/
        Input.bind Regexp.last_match(1).to_sym, nil
      end
    end

    next unless flag == :Font && (line =~ /^(.+)=(.+)$/)

    fontname = Regexp.last_match(1)
    path = Regexp.last_match(2)
    Finder::FontPaths[fontname] = path
  end
end
