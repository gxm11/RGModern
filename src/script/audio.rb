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
  module_function

  def bgm_play(filename, volume = 80, pitch = 100, pos = -1)
    return if @@disable_music

    puts 'ignoring the pitch setting for bgm.' if pitch != 100
    path = Finder.find(filename, :music)
    RGM::Base.audio_bgm_play(path, volume, pos)
  end

  def bgm_stop
    RGM::Base.audio_bgm_stop(0)
  end

  def bgm_fade(time)
    RGM::Base.audio_bgm_stop(time)
  end

  def bgm_pos
    RGM::Base.audio_bgm_pos
  end

  def me_play(filename, volume = 80, pitch = 100)
    return if @@disable_music

    puts 'ignoring the pitch setting for me.' if pitch != 100

    path = Finder.find(filename, :music)
    RGM::Base.audio_me_play(path, volume)
  end

  def me_stop
    RGM::Base.audio_me_stop(0)
  end

  def me_fade(time)
    RGM::Base.audio_me_stop(time)
  end

  def bgs_play(filename, volume = 80, pitch = 100)
    return if @@disable_sound

    path = Finder.find(filename, :sound)
    RGM::Base.audio_bgs_play(path, volume, pitch)
  end

  def bgs_stop
    RGM::Base.audio_bgs_stop(0)
  end

  def bgs_fade(time)
    RGM::Base.audio_bgs_stop(time)
  end

  def se_play(filename, volume = 80, pitch = 100)
    return if @@disable_sound
    return if volume <= 0

    path = Finder.find(filename, :sound)
    RGM::Base.audio_se_play(path, volume, pitch)
  end

  def se_stop
    RGM::Base.audio_se_stop
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
