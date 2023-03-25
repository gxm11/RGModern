# Copyright (c) 2022 Xiaomi Guo
# Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

module Audio2
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
  #  - 并且清空队列里除当前ME外的全部ME。
  #  - 此后在回调事件中将当前的ME出队。
  #   - 同时移除{flag_fading_out}
  #   - 队列里的BGM没有任何改动，此时会继续播放队列里最后一个BGM（见下）
  # - 如果需要fade或者stop BGM，则直接清除队列中的BGM。
  #  - 此后在回调事件中将当前的ME出队。因为此时队列里没有BGM，就不会再播放BGM。
  # 当队列的首项是BGM时，说明现在正在播放BGM。
  #  - 如果现在要修改当前BGM的position和volume，直接操作即可
  #  - 如果现在要切换BGM，则将新的BGM入队，fading_out当前的BGM
  #   - 向队列中添加一个dummy任务{flag_fading_out}。
  #  - 如果队列中有{flag_fading_out}，说明当前的BGM正在退出，不需要重复fade。
  #   - 此后在回调事件中将当前的BGM出队，播放队列中最后一个BGM
  #   - 同时移除{flag_fading_out}，和其他BGM。
  #  - 如果现在要切换ME，则将ME入队。
  #   - 当前的BGM的position和volume保存下来，再次入队。
  #   - fade out当前的BGM，等待回调。
  #   - 向队列中添加一个dummy任务{flag_fading_out}。
  #   - 如果队列中有{flag_fading_out}，说明当前的BGM正在退出，不需要重复fade。
  #    - 也不需要再次添加当前的BGM。
  #   - 此后在回调事件中将当前的BGM出队
  #    - 因为队列中有ME，所以会播放队列中最后一个ME。
  #    - 同时移除{flag_fading_out}，和其他ME。
  #    - BGM不受影响，所以旧的BGM还在，ME结束后会继续播放此BGM。
  Music_Queue = []

  T_FADEOUT = 0
  T_BGM = 1
  T_ME = 2

  Music_Task = Struct.new(:music, :type, :volume, :position)
  # 默认 fade 的时间，单位 ms
  Default_Fade_Time = 500

  module Music_State
    Fading = 1
    FadingIn = 2
    FadingOut = 4
    Paused = 8
    Playing = 16
  end

  module_function

  def bgm_play(filename, volume = 80, pitch = 100, pos = -1)
    return if @@disable_music

    puts 'ignoring the pitch setting for bgm.' if pitch != 100
    path = Finder.find(filename, :music)

    bgm = Music_Task.new(RGM::Ext::Music.new(path), T_BGM, volume, pos)

    # 获取音乐的 State
    state = RGM::Ext.music_get_state

    if (state & Music_State::FadingOut) != 0
      # 当前的音乐正在 fading 的场合
      # 加入到队列里，在fading结束后的回调里，会播放此BGM
      Music_Queue << bgm
    elsif (state & Music_State::Playing) != 0
      # 当前的音乐正在播放的场合
      current = Music_Queue.first
      if current.type == T_ME
        # 当前的音乐是ME，加入到队列里
        Music_Queue << bgm
      elsif current.type == T_BGM
        # 当前音乐是BGM
        if current.music == bgm.music
          # 与自己相同则修改volume和position
          RGM::Ext.music_set_volume(bgm.volume)
          RGM::Ext.music_set_position(bgm.position) if pos != -1
        else
          # 与自己不同，则fadeout当前music，然后把自己加入队列
          RGM::Ext.music_fade_out(Default_Fade_Time)
          Music_Queue << bgm
        end
      end
    else
      # 当前没有音乐在播放的场合
      if Music_Queue.empty?
        # 当前队列里没有任何音乐，则播放当前音乐
        bgm.play(-1)
        RGM::Ext.music_set_volume(bgm.volume)
        RGM::Ext.music_set_position(bgm.position)
      end
      # 添加到队列里
      Music_Queue << bgm
    end
  end

  def bgm_stop
    return if @@disable_music

    # 获取音乐的 State
    state = RGM::Ext.music_get_state
  end
end
