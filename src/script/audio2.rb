# Copyright (c) 2022 Xiaomi Guo
# Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

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

  # 实现方案：
  # 准备一个 Music_Queue 用来存放当前播放和待播放的BGM和ME，以及控制任务（dummy类型）
  # Music_Queue 的首项永远是当前正在播放，或者fading_in的那个音乐。
  # 每次BGM或者ME停止时，会发出回调事件，此时首项就会被shift。
  # 当队列为空时，BGM和ME的播放会插入到队列中，并且立刻开始播放。
  # 当队列的首项是ME时，说明现在正在播放ME。
  # - 如果现在要插播ME，则将新的ME添加到队列里，并且halt当前的ME。
  #  - 向队列中添加一个dummy任务{flag_fading_out}。
  #  - 如果队列中有{flag_fading_out}，说明当前的ME正在退出，不需要重复halt。
  #  - 此后在回调事件中将当前的ME出队，播放队列中最后一个ME
  #   - 同时移除{flag_fading_out}和其他ME
  #   - 队列里的BGM没有任何改动
  # - 如果现在要播放新的BGM。则将新的BGM添加到队列里。
  #  - 此后ME自动结束，在回调事件中将当前的ME出队，播放队列中最后一个BGM
  # - 如果需要fade或者stop ME，则直接执行相应内容。
  #  - 向队列中添加一个dummy任务{flag_fading_out}。
  #  - 如果队列中有{flag_fading_out}，说明当前的ME正在退出，不需要重复fade或者stop。
  #  - 并且清空队列里除当前ME外的全部ME。
  #  - 此后在回调事件中将当前的ME出队。
  #   - 同时移除{flag_fading_out}
  #   - 队列里的BGM没有任何改动，此时会继续播放队列里最后一个BGM（见下）
  # - 如果需要fade或者stop BGM，则直接清除队列中的BGM。
  #  - 此后在回调事件中将当前的ME出队。因为此时队列里没有BGM，就不会再播放BGM。
  # 当队列的首项是BGM时，说明现在正在播放BGM。
  # - 如果现在要修改当前BGM的position和volume，直接操作即可
  # - 如果现在要切换BGM，则将新的BGM入队，fading_out当前的BGM
  #  - 向队列中添加一个dummy任务{flag_fading_out}。
  # - 如果队列中有{flag_fading_out}，说明当前的BGM正在退出，不需要重复fade。
  #  - 此后在回调事件中将当前的BGM出队，播放队列中最后一个BGM
  #  - 同时移除{flag_fading_out}，和其他BGM。
  # - 如果现在要切换ME，则将ME入队。
  #  - 当前的BGM的position和volume保存下来，再次入队。
  #  - fade out当前的BGM，等待回调。
  #  - 向队列中添加一个dummy任务{flag_fading_out}。
  #  - 如果队列中有{flag_fading_out}，说明当前的BGM正在退出，不需要重复fade。
  #   - 也不需要再次添加当前的BGM。
  #  - 此后在回调事件中将当前的BGM出队
  #   - 因为队列中有ME，所以会播放队列中最后一个ME。
  #   - 同时移除{flag_fading_out}，和其他ME。
  #   - BGM不受影响，所以旧的BGM还在，ME结束后会继续播放此BGM。
  # - 如果需要fade或者stop BGM，则直接执行相应内容。
  #  - 向队列中添加一个dummy任务{flag_fading_out}。
  #  - 如果队列中有{flag_fading_out}，说明当前的BGM正在退出，不需要重复fade或者stop。
  #  - 并且清空队列里除当前BGM外的全部BGM。
  #  - 此后在回调事件中将当前的BGM出队。
  #   - 同时移除{flag_fading_out}
  # - 如果需要fade或者stop ME，则直接清除队列中的ME。

  # 综上所述，总共有5*(1+2x2)种情况需要考虑。
  # 5 种调用的函数：bgm_play/bgm_stop/me_play/me_stop/on_finish
  #  - 其中fade和stop没有本质区别。
  # 5 种场合：Queue为空，Queue的首项是BGM或者ME x Queue中是否有{flag_fading_out}

  # 默认 fade 的时间，单位 ms
  Default_Fade_Time = 500

  module Music_Manager
    Flag_Stop = 0
    Flag_BGM_Playing = 1
    Flag_BGM_Fading = 2
    Flag_ME_Playing = 3
    Flag_ME_Fading = 4

    Type_Unknown = 0
    Type_BGM = 1
    Type_ME = 2

    BGM_Queue = []
    ME_Queue = []

    @@state = Flag_Stop

    module_function

    def play(type, music)
      if type == Type_BGM
        case @@state
        when Flag_Stop
          music.play(-1)
          BGM_Queue << music
          @@state = Flag_BGM_Playing
        when Flag_BGM_Playing
          if music == BGM_Queue.first
            RGM::Ext.music_set_volume(music.volume)
            RGM::Ext.music_set_position(music.position) if music.position != -1
          else
            RGM::Ext.music_fade_out(Default_Fade_Time)
            BGM_Queue << music
            @@state = Flag_BGM_Fading
          end
        when Flag_BGM_Fading, Flag_ME_Playing, Flag_ME_Fading
          BGM_Queue << music
        end
      end

      if type == Type_ME
        ME_Queue << music

        case @@state
        when Flag_Stop
          music.play(0)
          @@state = Flag_ME_Playing
        when Flag_BGM_Playing
          current = BGM_Queue.first
          current.update
          RGM::Ext.music_fade_out(Default_Fade_Time)
          BGM_Queue << RGM::Ext::Music.new(current.path, current.volume, current.position)
          @@state = Flag_BGM_Fading
        when Flag_ME_Playing
          RGM::Ext.music_halt
          @@state = Flag_ME_Fading
        end
      end
    end

    def fade(type, time = 0)
      if type == Type_BGM
        case @@state
        when Flag_Stop
        when Flag_BGM_Playing
          if time > 0
            RGM::Ext.music_fade_out(time)
          else
            RGM::Ext.music_halt
          end
          @@state = Flag_BGM_Fading
        when Flag_BGM_Fading
          BGM_Queue.pop(BGM_Queue.size - 1)
        when Flag_ME_Playing
          BGM_Queue.clear
        when Flag_ME_Fading
          BGM_Queue.clear
        end
      end

      if type == Type_ME
        case @@state
        when Flag_Stop
        when Flag_BGM_Playing
          ME_Queue.clear
        when Flag_BGM_Fading
          ME_Queue.clear
        when Flag_ME_Playing
          if time > 0
            RGM::Ext.music_fade_out(time)
          else
            RGM::Ext.music_halt
          end
          @@state = Flag_ME_Fading
        when Flag_ME_Fading
          ME_Queue.pop(ME_Queue.size - 1)
        end
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
    end
  end

  # sound中，BGS是循环播放的。SE在播放前，需要clean一下当前的SE数量
  # 由于SDL channel的数量 = 8，所以SE
  module Sound_Manager
    Sound_List = []

    Type_Unknown = 0
    Type_BGS = 1
    Type_SE = 2

    Max_Channel = 8

    module_function

    def play(type, sound)
      clean if Sound_List.size >= Max_Channel

      # 播放 BGS 需要先 fade out 其他的 BGS
      if type == Type_BGS
        Sound_List.each do |type2, sound|
          sound.fade_out(Default_Fade_Time) if type2 == Type_BGS
        end
        Sound_List << [type, sound]
        sound.play(-1)
      end

      if type == Type_SE
        # 播放 SE 直接操作即可
        Sound_List << [type, sound]

        sound.play(0)
      end
    end

    def fade(type, time = 0)
      if type == Type_BGS
        Sound_List.each do |type2, sound|
          sound.fade_out(time) if type2 == Type_BGS
        end
      end

      if type == Type_SE
        Sound_List.each do |type2, sound|
          sound.stop if type2 == Type_SE
        end
      end
    end

    def clean
      Sound_List.select! { |_type, sound| sound.is_playing }
    end
  end

  module_function

  def bgm_play(filename, volume = 80, pitch = 100, pos = -1)
    return if @@disable_music

    puts 'ignoring the pitch setting for bgm.' if pitch != 100
    path = Finder.find(filename, :music)
    music = RGM::Ext::Music.new(path, volume, pos)
    Music_Manager.play(Music_Manager::Type_BGM, music)
  end

  def bgm_fade(time)
    return if @@disable_music

    Music_Manager.fade(Music_Manager::Type_BGM, time)
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
    music = RGM::Ext::Music.new(path, volume)
    Music_Manager.play(Music_Manager::Type_ME, music)
  end

  def me_fade(time)
    return if @@disable_music

    Music_Manager.fade(Music_Manager::Type_ME, time)
  end

  def me_stop
    return if @@disable_music

    me_fade(0)
  end

  def bgs_play(filename, volume = 80, pitch = 100)
    return if @@disable_sound

    path = Finder.find(filename, :sound)
    sound = RGM::Ext::Sound.new(path, volume, pitch)
    Sound_Manager.play(Sound_Manager::Type_BGS, sound)
  end

  def bgs_stop
    return if @@disable_sound

    bgs_fade(0)
  end

  def bgs_fade(time)
    return if @@disable_sound

    Sound_Manager.fade(Sound_Manager::Type_BGS, time)
  end

  def se_play(filename, volume = 80, pitch = 100)
    return if @@disable_sound

    path = Finder.find(filename, :sound)
    sound = RGM::Ext::Sound.new(path, volume, pitch)
    Sound_Manager.play(Sound_Manager::Type_SE, sound)
  end

  def se_stop
    return if @@disable_sound

    Sound_Manager.fade(Sound_Manager::Type_SE, 0)
  end

  def disable_music
    @@disable_music = true
  end

  def disable_sound
    @@disable_sound = true
  end

  @@disable_music = false
  @@disable_sound = false
end

module RGM
  module Ext
    module_function

    def music_finish_callback
      Audio::Music_Manager.on_finish
    end
  end
end
