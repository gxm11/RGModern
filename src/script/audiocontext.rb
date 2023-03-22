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
    class Music
      attr_accessor :volume, :position

      def self.create_finalizer(id)
        proc { RGM::Ext.music_dispose(id) }
      end

      def initialize(path)
        @id = RGM::Base.new_id
        @volume = 100
        @position = 0
        @path = path

        RGM::Ext.music_create(self, @id, @path)
        ObjectSpace.define_finalizer(self, self.class.create_finalizer(@id))
      end

      def play(iteration)
        RGM::Ext.music_play(@id, iteration)
      end

      def fade_in(iteration, duration)
        RGM::Ext.music_fade_in(@id, iteration, duration)
      end

      class << self
        def volume
          @@volume
        end

        def volume=(value)
          @@volume = value
          RGM::Ext.music_set_volume(value)
        end

        def position
          @@position
        end

        def position=(value)
          @@position = value
          RGM::Ext.music_set_position(value)
        end

        def pause
          RGM::Ext.music_pause
        end

        def halt
          RGM::Ext.music_halt
        end

        def fade_out(duration)
          RGM::Ext.music_fade_out(duration)
        end

        def is_playing
          RGM::Ext.music_is_playing
        end
      end
    end
  end
end
