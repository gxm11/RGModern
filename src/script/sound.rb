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

module RGM
  # RGM::Base 中相关的 module functions
  # 所有的 duration 单位都是 ms，但是 position 单位是 s。

  # 1. module functions using in Sound_Effect instance methods
  # - sound_create(id, path : String)
  # - sound_dispose(id)
  # - sound_play(id, iteration : Integer)
  # - sound_stop(id)
  # - sound_fade_in(id, duration : Integer)
  # - sound_fade_out(id, duration : Integer)
  # - sound_get_channel(id)
  # - sound_set_volume(id, volume : Integer)
  # - sound_set_pitch(id, pitch : Integer, loop : Boolean)

  # 2. module functions that show current states.
  # - sound_get_state can get results for following states:
  #  - sound_is_playing(id) -> 1
  #  - sound_is_fading(id) -> 2

  # 3. module functions can only use as Music class functions
  # - sound_is_any_playing -> bool

  class Sound
    attr_reader :id, :path
    attr_accessor :volume, :pitch

    def self.create_finalizer(id)
      proc { RGM::Base.sound_dispose(id) }
    end

    def initialize(path, volume = 100, pitch = 100)
      @id = RGM::Base.new_id
      @path = path
      @volume = volume
      @pitch = pitch

      RGM::Base.sound_create(@id, @path)
      ObjectSpace.define_finalizer(self, self.class.create_finalizer(@id))
    end

    def play(iteration)
      RGM::Base.sound_play(@id, iteration)
      RGM::Base.sound_set_volume(@id, @volume.to_i)
      RGM::Base.sound_set_pitch(@id, @pitch.to_i, (iteration == -1))
    end

    def stop
      RGM::Base.sound_stop(@id)
    end

    def fade_in(duration)
      RGM::Base.sound_fade_in(@id, duration.to_i)
    end

    def fade_out(duration)
      if duration > 0
        RGM::Base.sound_fade_out(@id, duration.to_i)
      else
        RGM::Base.sound_stop(@id)
      end
    end

    def is_playing
      state = RGM::Base.sound_get_state(@id)
      (state & 1) != 0
    end

    def is_fading
      state = RGM::Base.sound_get_state(@id)
      (state & 2) != 0
    end

    def channel
      RGM::Base.sound_get_channel(@id)
    end

    def ==(other)
      other.is_a?(Sound) && other.id == @id
    end
  end
end
