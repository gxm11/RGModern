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

unless File.exist?(RGM::Default_Config)
  File.open(RGM::Default_Config, 'wb') do |f|
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
