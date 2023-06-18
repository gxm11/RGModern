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

module Audio
  # 由于SDL的特殊设计，music不存在多通道混合，一次只能有 1 个music在播放。
  # 这看上去是更底层的限制，当然这也符合 RGSS 的设定。

  # 最终的表现为：
  # 关于BGM
  # 1. BGM正在播放时，插播的BGM如果跟原BGM相同，则会修改volume和position
  # 2. BGM正在播放时，插播的BGM如果跟原BGM不同，会切换到新的BGM
  # 3. BGM正在播放时，快速插播多个BGM，只会切换到最后一个BGM
  # 4. 无论当前是否在播放BGM，stop和fade都会停止当前的BGM，之后也不会播放任何新的BGM
  # 5. BGM会循环播放
  # 关于ME
  # 1. 当前ME播放时，播放ME会打断当前的ME
  # 2. 当前ME播放时，快速插播多个ME，只会切换到最后一个ME
  # 3. 无论当前是否在播放ME，stop和fade会停止当前的ME，之后也不会播放任何新的ME
  # 4. ME不会循环播放，播放一次后就会停止
  # 关于BGM插播ME
  # 1. BGM正在播放时，插播ME会暂停当前的BGM
  # 2. ME播放结束后，会还原当前的BGM
  # 3. ME播放时，插播其他的ME，会播放新的ME，不会影响需要还原的BGM
  # 4. ME播放时，快速插播多个ME，只会播放最后一个ME
  # 5. ME播放时，插播其他的BGM，不影响正在播放的ME，在ME结束后会切换为那个BGM
  # 6. ME播放时，插播与被暂停的BGM相同的BGM，会修改被暂停BGM的volume和position
  # 7. ME播放时，快速插播多个BGM，只要其中有一个与被暂停的BGM不同，在结束后只会切换到最后一个BGM

  # 默认 fade 的时间，单位 ms
  Default_Fade_Time = 500

  module Music_Manager
    Flag_Stop = 0
    Flag_BGM_Playing = 1
    Flag_BGM_Fading = 2
    Flag_ME_Playing = 3
    Flag_ME_Fading = 4

    BGM_Queue = []
    ME_Queue = []

    @@state = Flag_Stop

    module_function

    def play_bgm(music)
      case @@state
      when Flag_Stop
        music.play(-1)
        BGM_Queue << music
        @@state = Flag_BGM_Playing
      when Flag_BGM_Playing
        if music == BGM_Queue.first
          RGM::Base.music_set_volume(music.volume.to_i)
          RGM::Base.music_set_position(music.position.to_f) if music.position != -1
        else
          RGM::Base.music_fade_out(Default_Fade_Time)
          BGM_Queue << music
          @@state = Flag_BGM_Fading
        end
      when Flag_BGM_Fading, Flag_ME_Playing, Flag_ME_Fading
        BGM_Queue << music
      end
    end

    def play_me(music)
      ME_Queue << music

      case @@state
      when Flag_Stop
        music.play(0)
        @@state = Flag_ME_Playing
      when Flag_BGM_Playing
        current = BGM_Queue.first
        current.update
        RGM::Base.music_fade_out(Default_Fade_Time)
        BGM_Queue << RGM::Music.new(current.path, current.volume, current.position)
        @@state = Flag_BGM_Fading
      when Flag_ME_Playing
        RGM::Base.music_halt
        @@state = Flag_ME_Fading
      end
    end

    def fade_bgm(time = 0)
      case @@state
      when Flag_BGM_Playing
        if time > 0
          RGM::Base.music_fade_out(time.to_i)
        else
          RGM::Base.music_halt
        end
        @@state = Flag_BGM_Fading
      when Flag_BGM_Fading
        BGM_Queue.pop(BGM_Queue.size - 1)
      when Flag_ME_Playing, Flag_ME_Fading
        BGM_Queue.clear
      end
    end

    def fade_me(time = 0)
      case @@state
      when Flag_BGM_Playing, Flag_BGM_Fading
        ME_Queue.clear
      when Flag_ME_Playing
        if time > 0
          RGM::Base.music_fade_out(time.to_i)
        else
          RGM::Base.music_halt
        end
        @@state = Flag_ME_Fading
      when Flag_ME_Fading
        ME_Queue.pop(ME_Queue.size - 1)
      end
    end

    def on_finish
      case @@state
      when Flag_Stop
        BGM_Queue.clear
        ME_Queue.clear
      when Flag_BGM_Playing, Flag_BGM_Fading
        BGM_Queue.shift
      when Flag_ME_Playing, Flag_ME_Fading
        ME_Queue.shift
      end

      @@state = Flag_Stop

      unless ME_Queue.empty?
        ME_Queue.shift(ME_Queue.size - 1)
        music = ME_Queue.first
        music.play(0)
        @@state = Flag_ME_Playing
        return
      end

      unless BGM_Queue.empty?
        BGM_Queue.shift(BGM_Queue.size - 1)
        music = BGM_Queue.first
        music.play(-1)
        @@state = Flag_BGM_Playing
      end

      nil
    end

    def set_soundfonts(path)
      paths = []
      path.split(';').each do |sf_path|
        paths << Finder.find(sf_path, :soundfont)
      end
      RGM::Base.music_set_soundfonts(paths.compact.join(';'))
    end
  end

  # sound中，BGS是循环播放的。SE在播放前，需要clean一下当前的SE数量
  # 由于SDL channel的数量 = 8，所以SE
  module Sound_Manager
    Sound_List = []

    Type_BGS = 0
    Type_SE = 1

    Max_Channel = 8

    module_function

    def play_bgs(sound)
      clean if Sound_List.size >= Max_Channel

      # 播放 BGS 需要先 fade out 其他的 BGS
      Sound_List.each do |type, sound|
        sound.fade_out(Default_Fade_Time) if type == Type_BGS
      end
      Sound_List << [Type_BGS, sound]
      sound.play(-1)
    end

    def play_se(sound)
      clean if Sound_List.size >= Max_Channel
      # 播放 SE 直接操作即可
      Sound_List << [Type_SE, sound]

      sound.play(0)
    end

    def fade_bgs(time = 0)
      Sound_List.each do |type, sound|
        sound.fade_out(time) if type == Type_BGS
      end
    end

    def fade_se(_time = 0)
      Sound_List.each do |type, sound|
        sound.stop if type == Type_SE
      end
    end

    def clean
      Sound_List.select! { |_type, sound| sound.is_playing }
    end
  end
end

module RGM
  module Base
    module_function

    def music_finish_callback
      Audio::Music_Manager.on_finish
    end
  end
end

module Audio
  module_function

  def bgm_play(filename, volume = 80, pitch = 100, pos = -1)
    return if @@disable_music

    puts 'ignoring the pitch setting for bgm.' if pitch != 100
    path = Finder.find(filename, :music)
    music = RGM::Music.new(path, volume, pos)
    Music_Manager.play_bgm(music)
  end

  def bgm_fade(time)
    return if @@disable_music

    Music_Manager.fade_bgm(time)
  end

  def bgm_stop
    return if @@disable_music

    bgm_fade(0)
  end

  def bgm_pos
    bgm = Music_Manager::BGM_Queue.first
    return -1 if bgm.nil?

    bgm.update
    bgm.position
  end

  def me_play(filename, volume = 80, pitch = 100)
    return if @@disable_music

    puts 'ignoring the pitch setting for bgm.' if pitch != 100
    path = Finder.find(filename, :music)
    music = RGM::Music.new(path, volume)
    Music_Manager.play_me(music)
  end

  def me_fade(time)
    return if @@disable_music

    Music_Manager.fade_me(time)
  end

  def me_stop
    return if @@disable_music

    me_fade(0)
  end

  def bgs_play(filename, volume = 80, pitch = 100)
    return if @@disable_sound

    path = Finder.find(filename, :sound)
    sound = RGM::Sound.new(path, volume, pitch)
    Sound_Manager.play_bgs(sound)
  end

  def bgs_stop
    return if @@disable_sound

    bgs_fade(0)
  end

  def bgs_fade(time)
    return if @@disable_sound

    Sound_Manager.fade_bgs(time)
  end

  def se_play(filename, volume = 80, pitch = 100)
    return if @@disable_sound

    path = Finder.find(filename, :sound)
    sound = RGM::Sound.new(path, volume, pitch)
    Sound_Manager.play_se(sound)
  end

  def se_stop
    return if @@disable_sound

    Sound_Manager.fade_se(0)
  end

  def disable_music
    @@disable_music = true
    bgm_stop
    me_stop
  end

  def disable_sound
    @@disable_sound = true
    bgs_stop
    se_stop
  end

  @@disable_music = false
  @@disable_sound = false

  # 下面的代码为了解决 Build_Mode = 2 时 btest 没声音。
  # 可能是这里需要唤醒一下音频所在的线程，实际上只需要执行 se_stop 就可以了
  bgm_stop
  me_stop
  bgs_stop
  se_stop
end
