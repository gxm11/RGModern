# Copyright (c) 2022 Xiaomi Guo
# Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

module RGM
  module Ext
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
    #  - sound_is_playing(id) -> bool
    #  - sound_is_fading(id) -> bool

    # 3. module functions can only use as Music class functions
    # - sound_is_any_playing -> bool

    class Sound
      attr_reader :id, :path
      attr_accessor :volume, :pitch

      def self.create_finalizer(id)
        proc { RGM::Ext.sound_dispose(id) }
      end

      def initialize(path, volume = 100, pitch = 100)
        @id = RGM::Base.new_id
        @path = path
        @volume = volume
        @pitch = pitch

        RGM::Ext.sound_create(@id, @path)
        ObjectSpace.define_finalizer(self, self.class.create_finalizer(@id))
      end

      def play(iteration)
        RGM::Ext.sound_play(@id, iteration)
        RGM::Ext.sound_set_volume(@id, @volume)
        RGM::Ext.sound_set_pitch(@id, @pitch, (iteration == -1))
      end

      def stop
        RGM::Ext.sound_stop(@id)
      end

      def fade_in(duration)
        RGM::Ext.sound_fade_in(@id, duration)
      end

      def fade_out(duration)
        RGM::Ext.sound_fade_out(@id, duration)
      end

      def is_playing
        state = RGM::Ext.sound_get_state(@id)
        (state & 1) != 0
      end

      def is_fading
        state = RGM::Ext.sound_get_state(@id)
        (state & 2) != 0
      end

      def channel
        RGM::Ext.sound_get_channel(@id)
      end

      def ==(other)
        other.is_a?(Sound) && other.id == @id
      end
    end
  end
end
