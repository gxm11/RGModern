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

    # 1. module functions using in Music instance methods
    # - music_create(id, path : String)
    # - music_dispose(id)
    # - music_play(id, iteration)
    # - music_fade_in(id, iteration, duration)
    # - music_get_position(id)

    # 2. module functions for volumn and position
    # - music_get_volumn
    # - music_set_volumn(volumn : Integer)
    # - music_set_position(position : Float)

    # 3. module functions can only use as Music class functions
    # - music_resume
    # - music_pause
    # - music_halt
    # - music_rewind
    # - music_fade_out(duration)

    # 4. module functions that show current states.
    # - music_get_state can get results for following states:
    #  - music_is_playing -> Bool
    #  - music_is_paused -> Bool
    #  - music_is_fading -> Bool
    #  - music_is_fading_in -> Bool
    #  - music_is_fading_out -> Bool

    # 5. module functions that will automatically called
    #    after current music finishes. However, it will be
    #    called in next Input.update asynchronously.
    # - music_finish_callback
    def self.music_finish_callback; end
  end
end

module RGM
  module Ext
    class Music
      attr_reader :id, :path
      attr_accessor :volume, :position

      def self.create_finalizer(id)
        proc { RGM::Ext.music_dispose(id) }
      end

      def initialize(path, volume = 100, position = -1)
        @id = RGM::Base.new_id
        @path = path
        @volume = volume
        @position = position

        RGM::Ext.music_create(@id, @path)
        ObjectSpace.define_finalizer(self, self.class.create_finalizer(@id))
      end

      def play(iteration)
        RGM::Ext.music_play(@id, iteration)
        RGM::Ext.music_set_volume(@volume)
        RGM::Ext.music_set_position(@position) if @position != -1
      end

      # duration 单位是 ms
      def fade_in(iteration, duration)
        RGM::Ext.music_fade_in(@id, iteration, duration)
      end

      # position 单位是 s
      def update
        @volume = RGM::Ext.music_get_volume
        @position = RGM::Ext.music_get_position(@id)
      end

      def ==(other)
        other.is_a?(Music) && other.path == @path
      end
    end
  end
end
