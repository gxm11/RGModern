# Copyright (c) 2005 RPG MAKER XP
# Various RPG MAKER XP functions, from character parameters to the battle system, are created via
# programs called scripts. RPG MAKER XP is equipped with the Ruby Game Scripting System (RGSS),
# based on the Ruby language and customized especially for this program. By editing these RGSS
# scripts, you are free to change or add to any of RPG MAKER XP's functions. Think of it as taking
# a peek into the engine of RPG MAKER XP and tinkering around with what's inside. Since RGSS is
# very similar to a programming language, it may take some time to master. Once you've mastered
# these scripts however, you'll not only be able to edit menu screens and the battle system but
# you'll also be able to create your very own event commands. Though this feature is geared toward
# expert users, becoming familiar with it will allow RPG creation with more complex and engaging
# games.

# -------------------------------------------------------------------------------------------------
# RPG::Cache
# -------------------------------------------------------------------------------------------------
module RPG
  module Cache
    @cache = {}
    def self.load_bitmap(folder_name, filename, hue = 0)
      path = folder_name + filename
      if !@cache.include?(path) || @cache[path].disposed?
        @cache[path] = if filename != ''
                         Bitmap.new(path)
                       else
                         Bitmap.new(32, 32)
                       end
      end
      if hue == 0
        @cache[path]
      else
        key = [path, hue]
        if !@cache.include?(key) || @cache[key].disposed?
          @cache[key] = @cache[path].clone
          @cache[key].hue_change(hue)
        end
        @cache[key]
      end
    end

    def self.animation(filename, hue)
      load_bitmap('Graphics/Animations/', filename, hue)
    end

    def self.autotile(filename)
      load_bitmap('Graphics/Autotiles/', filename)
    end

    def self.battleback(filename)
      load_bitmap('Graphics/Battlebacks/', filename)
    end

    def self.battler(filename, hue)
      load_bitmap('Graphics/Battlers/', filename, hue)
    end

    def self.character(filename, hue)
      load_bitmap('Graphics/Characters/', filename, hue)
    end

    def self.fog(filename, hue)
      load_bitmap('Graphics/Fogs/', filename, hue)
    end

    def self.gameover(filename)
      load_bitmap('Graphics/Gameovers/', filename)
    end

    def self.icon(filename)
      load_bitmap('Graphics/Icons/', filename)
    end

    def self.panorama(filename, hue)
      load_bitmap('Graphics/Panoramas/', filename, hue)
    end

    def self.picture(filename)
      load_bitmap('Graphics/Pictures/', filename)
    end

    def self.tileset(filename)
      load_bitmap('Graphics/Tilesets/', filename)
    end

    def self.title(filename)
      load_bitmap('Graphics/Titles/', filename)
    end

    def self.windowskin(filename)
      load_bitmap('Graphics/Windowskins/', filename)
    end

    def self.tile(filename, tile_id, hue)
      key = [filename, tile_id, hue]
      if !@cache.include?(key) || @cache[key].disposed?
        @cache[key] = Bitmap.new(32, 32)
        x = (tile_id - 384) % 8 * 32
        y = (tile_id - 384) / 8 * 32
        rect = Rect.new(x, y, 32, 32)
        @cache[key].blt(0, 0, tileset(filename), rect)
        @cache[key].hue_change(hue)
      end
      @cache[key]
    end

    def self.clear
      @cache = {}
      GC.start
    end
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Sprite
# -------------------------------------------------------------------------------------------------
module RPG
  class Sprite < ::Sprite
    @@_animations = []
    @@_reference_count = {}
    def initialize(viewport = nil)
      super(viewport)
      @_whiten_duration = 0
      @_appear_duration = 0
      @_escape_duration = 0
      @_collapse_duration = 0
      @_damage_duration = 0
      @_animation_duration = 0
      @_blink = false
    end

    def dispose
      dispose_damage
      dispose_animation
      dispose_loop_animation
      super
    end

    def whiten
      self.blend_type = 0
      color.set(255, 255, 255, 128)
      self.opacity = 255
      @_whiten_duration = 16
      @_appear_duration = 0
      @_escape_duration = 0
      @_collapse_duration = 0
    end

    def appear
      self.blend_type = 0
      color.set(0, 0, 0, 0)
      self.opacity = 0
      @_appear_duration = 16
      @_whiten_duration = 0
      @_escape_duration = 0
      @_collapse_duration = 0
    end

    def escape
      self.blend_type = 0
      color.set(0, 0, 0, 0)
      self.opacity = 255
      @_escape_duration = 32
      @_whiten_duration = 0
      @_appear_duration = 0
      @_collapse_duration = 0
    end

    def collapse
      self.blend_type = 1
      color.set(255, 64, 64, 255)
      self.opacity = 255
      @_collapse_duration = 48
      @_whiten_duration = 0
      @_appear_duration = 0
      @_escape_duration = 0
    end

    def damage(value, critical)
      dispose_damage
      damage_string = if value.is_a?(Numeric)
                        value.abs.to_s
                      else
                        value.to_s
                      end
      bitmap = Bitmap.new(160, 48)
      bitmap.font.name = 'Ariblk'
      bitmap.font.size = 24
      bitmap.font.color.set(0, 0, 0)
      bitmap.draw_text(-1, 12 - 1, 160, 36, damage_string, 1)
      bitmap.draw_text(+1, 12 - 1, 160, 36, damage_string, 1)
      bitmap.draw_text(-1, 12 + 1, 160, 36, damage_string, 1)
      bitmap.draw_text(+1, 12 + 1, 160, 36, damage_string, 1)
      if value.is_a?(Numeric) && (value < 0)
        bitmap.font.color.set(176, 255, 144)
      else
        bitmap.font.color.set(255, 255, 255)
      end
      bitmap.draw_text(0, 12, 160, 36, damage_string, 1)
      if critical
        bitmap.font.size = 16
        bitmap.font.color.set(0, 0, 0)
        bitmap.draw_text(-1, -1, 160, 20, 'CRITICAL', 1)
        bitmap.draw_text(+1, -1, 160, 20, 'CRITICAL', 1)
        bitmap.draw_text(-1, +1, 160, 20, 'CRITICAL', 1)
        bitmap.draw_text(+1, +1, 160, 20, 'CRITICAL', 1)
        bitmap.font.color.set(255, 255, 255)
        bitmap.draw_text(0, 0, 160, 20, 'CRITICAL', 1)
      end
      @_damage_sprite = ::Sprite.new(viewport)
      @_damage_sprite.bitmap = bitmap
      @_damage_sprite.ox = 80
      @_damage_sprite.oy = 20
      @_damage_sprite.x = x
      @_damage_sprite.y = y - oy / 2
      @_damage_sprite.z = 3000
      @_damage_duration = 40
    end

    def animation(animation, hit)
      dispose_animation
      @_animation = animation
      return if @_animation.nil?

      @_animation_hit = hit
      @_animation_duration = @_animation.frame_max
      animation_name = @_animation.animation_name
      animation_hue = @_animation.animation_hue
      bitmap = RPG::Cache.animation(animation_name, animation_hue)
      if @@_reference_count.include?(bitmap)
        @@_reference_count[bitmap] += 1
      else
        @@_reference_count[bitmap] = 1
      end
      @_animation_sprites = []
      if (@_animation.position != 3) || !@@_animations.include?(animation)
        (0..15).each do |_i|
          sprite = ::Sprite.new(viewport)
          sprite.bitmap = bitmap
          sprite.visible = false
          @_animation_sprites.push(sprite)
        end
        @@_animations.push(animation) unless @@_animations.include?(animation)
      end
      update_animation
    end

    def loop_animation(animation)
      return if animation == @_loop_animation

      dispose_loop_animation
      @_loop_animation = animation
      return if @_loop_animation.nil?

      @_loop_animation_index = 0
      animation_name = @_loop_animation.animation_name
      animation_hue = @_loop_animation.animation_hue
      bitmap = RPG::Cache.animation(animation_name, animation_hue)
      if @@_reference_count.include?(bitmap)
        @@_reference_count[bitmap] += 1
      else
        @@_reference_count[bitmap] = 1
      end
      @_loop_animation_sprites = []
      (0..15).each do |_i|
        sprite = ::Sprite.new(viewport)
        sprite.bitmap = bitmap
        sprite.visible = false
        @_loop_animation_sprites.push(sprite)
      end
      update_loop_animation
    end

    def dispose_damage
      unless @_damage_sprite.nil?
        @_damage_sprite.bitmap.dispose
        @_damage_sprite.dispose
        @_damage_sprite = nil
        @_damage_duration = 0
      end
    end

    def dispose_animation
      unless @_animation_sprites.nil?
        sprite = @_animation_sprites[0]
        unless sprite.nil?
          @@_reference_count[sprite.bitmap] -= 1
          sprite.bitmap.dispose if @@_reference_count[sprite.bitmap] == 0
        end
        @_animation_sprites.each do |sprite|
          sprite.dispose
        end
        @_animation_sprites = nil
        @_animation = nil
      end
    end

    def dispose_loop_animation
      unless @_loop_animation_sprites.nil?
        sprite = @_loop_animation_sprites[0]
        unless sprite.nil?
          @@_reference_count[sprite.bitmap] -= 1
          sprite.bitmap.dispose if @@_reference_count[sprite.bitmap] == 0
        end
        @_loop_animation_sprites.each do |sprite|
          sprite.dispose
        end
        @_loop_animation_sprites = nil
        @_loop_animation = nil
      end
    end

    def blink_on
      unless @_blink
        @_blink = true
        @_blink_count = 0
      end
    end

    def blink_off
      if @_blink
        @_blink = false
        color.set(0, 0, 0, 0)
      end
    end

    def blink?
      @_blink
    end

    def effect?
      @_whiten_duration > 0 or
        @_appear_duration > 0 or
        @_escape_duration > 0 or
        @_collapse_duration > 0 or
        @_damage_duration > 0 or
        @_animation_duration > 0
    end

    def update
      super
      if @_whiten_duration > 0
        @_whiten_duration -= 1
        color.alpha = 128 - (16 - @_whiten_duration) * 10
      end
      if @_appear_duration > 0
        @_appear_duration -= 1
        self.opacity = (16 - @_appear_duration) * 16
      end
      if @_escape_duration > 0
        @_escape_duration -= 1
        self.opacity = 256 - (32 - @_escape_duration) * 10
      end
      if @_collapse_duration > 0
        @_collapse_duration -= 1
        self.opacity = 256 - (48 - @_collapse_duration) * 6
      end
      if @_damage_duration > 0
        @_damage_duration -= 1
        case @_damage_duration
        when 38..39
          @_damage_sprite.y -= 4
        when 36..37
          @_damage_sprite.y -= 2
        when 34..35
          @_damage_sprite.y += 2
        when 28..33
          @_damage_sprite.y += 4
        end
        @_damage_sprite.opacity = 256 - (12 - @_damage_duration) * 32
        dispose_damage if @_damage_duration == 0
      end
      if !@_animation.nil? && Graphics.frame_count.even?
        @_animation_duration -= 1
        update_animation
      end
      if !@_loop_animation.nil? && Graphics.frame_count.even?
        update_loop_animation
        @_loop_animation_index += 1
        @_loop_animation_index %= @_loop_animation.frame_max
      end
      if @_blink
        @_blink_count = (@_blink_count + 1) % 32
        alpha = if @_blink_count < 16
                  (16 - @_blink_count) * 6
                else
                  (@_blink_count - 16) * 6
                end
        color.set(255, 255, 255, alpha)
      end
      @@_animations.clear
    end

    def update_animation
      if @_animation_duration > 0
        frame_index = @_animation.frame_max - @_animation_duration
        cell_data = @_animation.frames[frame_index].cell_data
        position = @_animation.position
        animation_set_sprites(@_animation_sprites, cell_data, position)
        @_animation.timings.each do |timing|
          animation_process_timing(timing, @_animation_hit) if timing.frame == frame_index
        end
      else
        dispose_animation
      end
    end

    def update_loop_animation
      frame_index = @_loop_animation_index
      cell_data = @_loop_animation.frames[frame_index].cell_data
      position = @_loop_animation.position
      animation_set_sprites(@_loop_animation_sprites, cell_data, position)
      @_loop_animation.timings.each do |timing|
        animation_process_timing(timing, true) if timing.frame == frame_index
      end
    end

    def animation_set_sprites(sprites, cell_data, position)
      (0..15).each do |i|
        sprite = sprites[i]
        pattern = cell_data[i, 0]
        if sprite.nil? || pattern.nil? || (pattern == -1)
          sprite.visible = false unless sprite.nil?
          next
        end
        sprite.visible = true
        sprite.src_rect.set(pattern % 5 * 192, pattern / 5 * 192, 192, 192)
        if position == 3
          if !viewport.nil?
            sprite.x = viewport.rect.width / 2
            sprite.y = viewport.rect.height - 160
          else
            sprite.x = 320
            sprite.y = 240
          end
        else
          sprite.x = x - ox + src_rect.width / 2
          sprite.y = y - oy + src_rect.height / 2
          sprite.y -= src_rect.height / 4 if position == 0
          sprite.y += src_rect.height / 4 if position == 2
        end
        sprite.x += cell_data[i, 1]
        sprite.y += cell_data[i, 2]
        sprite.z = 2000
        sprite.ox = 96
        sprite.oy = 96
        sprite.zoom_x = cell_data[i, 3] / 100.0
        sprite.zoom_y = cell_data[i, 3] / 100.0
        sprite.angle = cell_data[i, 4]
        sprite.mirror = (cell_data[i, 5] == 1)
        sprite.opacity = cell_data[i, 6] * opacity / 255.0
        sprite.blend_type = cell_data[i, 7]
      end
    end

    def animation_process_timing(timing, hit)
      if (timing.condition == 0) ||
         ((timing.condition == 1) && (hit == true)) ||
         ((timing.condition == 2) && (hit == false))
        if timing.se.name != ''
          se = timing.se
          Audio.se_play('Audio/SE/' + se.name, se.volume, se.pitch)
        end
        case timing.flash_scope
        when 1
          flash(timing.flash_color, timing.flash_duration * 2)
        when 2
          viewport.flash(timing.flash_color, timing.flash_duration * 2) unless viewport.nil?
        when 3
          flash(nil, timing.flash_duration * 2)
        end
      end
    end

    def x=(x)
      sx = x - self.x
      if sx != 0
        unless @_animation_sprites.nil?
          (0..15).each do |i|
            @_animation_sprites[i].x += sx
          end
        end
        unless @_loop_animation_sprites.nil?
          (0..15).each do |i|
            @_loop_animation_sprites[i].x += sx
          end
        end
      end
      super
    end

    def y=(y)
      sy = y - self.y
      if sy != 0
        unless @_animation_sprites.nil?
          (0..15).each do |i|
            @_animation_sprites[i].y += sy
          end
        end
        unless @_loop_animation_sprites.nil?
          (0..15).each do |i|
            @_loop_animation_sprites[i].y += sy
          end
        end
      end
      super
    end
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Weather
# -------------------------------------------------------------------------------------------------
module RPG
  class Weather
    def initialize(viewport = nil)
      @type = 0
      @max = 0
      @ox = 0
      @oy = 0
      color1 = Color.new(255, 255, 255, 255)
      color2 = Color.new(255, 255, 255, 128)
      @rain_bitmap = Bitmap.new(7, 56)
      (0..6).each do |i|
        @rain_bitmap.fill_rect(6 - i, i * 8, 1, 8, color1)
      end
      @storm_bitmap = Bitmap.new(34, 64)
      (0..31).each do |i|
        @storm_bitmap.fill_rect(33 - i, i * 2, 1, 2, color2)
        @storm_bitmap.fill_rect(32 - i, i * 2, 1, 2, color1)
        @storm_bitmap.fill_rect(31 - i, i * 2, 1, 2, color2)
      end
      @snow_bitmap = Bitmap.new(6, 6)
      @snow_bitmap.fill_rect(0, 1, 6, 4, color2)
      @snow_bitmap.fill_rect(1, 0, 4, 6, color2)
      @snow_bitmap.fill_rect(1, 2, 4, 2, color1)
      @snow_bitmap.fill_rect(2, 1, 2, 4, color1)
      @sprites = []
      (1..40).each do |_i|
        sprite = Sprite.new(viewport)
        sprite.z = 1000
        sprite.visible = false
        sprite.opacity = 0
        @sprites.push(sprite)
      end
    end

    def dispose
      @sprites.each do |sprite|
        sprite.dispose
      end
      @rain_bitmap.dispose
      @storm_bitmap.dispose
      @snow_bitmap.dispose
    end

    def type=(type)
      return if @type == type

      @type = type
      bitmap = case @type
               when 1
                 @rain_bitmap
               when 2
                 @storm_bitmap
               when 3
                 @snow_bitmap
               end
      (1..40).each do |i|
        sprite = @sprites[i]
        unless sprite.nil?
          sprite.visible = (i <= @max)
          sprite.bitmap = bitmap
        end
      end
    end

    def ox=(ox)
      return if @ox == ox

      @ox = ox
      @sprites.each do |sprite|
        sprite.ox = @ox
      end
    end

    def oy=(oy)
      return if @oy == oy

      @oy = oy
      @sprites.each do |sprite|
        sprite.oy = @oy
      end
    end

    def max=(max)
      return if @max == max

      @max = [[max, 0].max, 40].min
      (1..40).each do |i|
        sprite = @sprites[i]
        sprite.visible = (i <= @max) unless sprite.nil?
      end
    end

    def update
      return if @type == 0

      (1..@max).each do |i|
        sprite = @sprites[i]
        break if sprite.nil?

        if @type == 1
          sprite.x -= 2
          sprite.y += 16
          sprite.opacity -= 8
        end
        if @type == 2
          sprite.x -= 8
          sprite.y += 16
          sprite.opacity -= 12
        end
        if @type == 3
          sprite.x -= 2
          sprite.y += 8
          sprite.opacity -= 8
        end
        x = sprite.x - @ox
        y = sprite.y - @oy
        next unless (sprite.opacity < 64) || (x < -50) || (x > 750) || (y < -300) || (y > 500)

        sprite.x = rand(-50..749) + @ox
        sprite.y = rand(-200..599) + @oy
        sprite.opacity = 255
      end
    end
    attr_reader :type, :max, :ox, :oy
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Map
# -------------------------------------------------------------------------------------------------
module RPG
  class Map
    def initialize(width, height)
      @tileset_id = 1
      @width = width
      @height = height
      @autoplay_bgm = false
      @bgm = RPG::AudioFile.new
      @autoplay_bgs = false
      @bgs = RPG::AudioFile.new('', 80)
      @encounter_list = []
      @encounter_step = 30
      @data = Table.new(width, height, 3)
      @events = {}
    end
    attr_accessor :tileset_id, :width, :height, :autoplay_bgm, :bgm, :autoplay_bgs, :bgs,
                  :encounter_list, :encounter_step, :data, :events
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::MapInfo
# -------------------------------------------------------------------------------------------------
module RPG
  class MapInfo
    def initialize
      @name = ''
      @parent_id = 0
      @order = 0
      @expanded = false
      @scroll_x = 0
      @scroll_y = 0
    end
    attr_accessor :name, :parent_id, :order, :expanded, :scroll_x, :scroll_y
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Event
# -------------------------------------------------------------------------------------------------
module RPG
  class Event
    def initialize(x, y)
      @id = 0
      @name = ''
      @x = x
      @y = y
      @pages = [RPG::Event::Page.new]
    end
    attr_accessor :id, :name, :x, :y, :pages
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Event::Page
# -------------------------------------------------------------------------------------------------
module RPG
  class Event
    class Page
      def initialize
        @condition = RPG::Event::Page::Condition.new
        @graphic = RPG::Event::Page::Graphic.new
        @move_type = 0
        @move_speed = 3
        @move_frequency = 3
        @move_route = RPG::MoveRoute.new
        @walk_anime = true
        @step_anime = false
        @direction_fix = false
        @through = false
        @always_on_top = false
        @trigger = 0
        @list = [RPG::EventCommand.new]
      end
      attr_accessor :condition, :graphic, :move_type, :move_speed, :move_frequency, :move_route,
                    :walk_anime, :step_anime, :direction_fix, :through, :always_on_top, :trigger,
                    :list
    end
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Event::Page::Condition
# -------------------------------------------------------------------------------------------------
module RPG
  class Event
    class Page
      class Condition
        def initialize
          @switch1_valid = false
          @switch2_valid = false
          @variable_valid = false
          @self_switch_valid = false
          @switch1_id = 1
          @switch2_id = 1
          @variable_id = 1
          @variable_value = 0
          @self_switch_ch = 'A'
        end
        attr_accessor :switch1_valid, :switch2_valid, :variable_valid, :self_switch_valid,
                      :switch1_id, :switch2_id, :variable_id, :variable_value, :self_switch_ch
      end
    end
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Event::Page::Graphic
# -------------------------------------------------------------------------------------------------
module RPG
  class Event
    class Page
      class Graphic
        def initialize
          @tile_id = 0
          @character_name = ''
          @character_hue = 0
          @direction = 2
          @pattern = 0
          @opacity = 255
          @blend_type = 0
        end
        attr_accessor :tile_id, :character_name, :character_hue, :direction, :pattern, :opacity,
                      :blend_type
      end
    end
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::EventCommand
# -------------------------------------------------------------------------------------------------
module RPG
  class EventCommand
    def initialize(code = 0, indent = 0, parameters = [])
      @code = code
      @indent = indent
      @parameters = parameters
    end
    attr_accessor :code, :indent, :parameters
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::MoveRoute
# -------------------------------------------------------------------------------------------------
module RPG
  class MoveRoute
    def initialize
      @repeat = true
      @skippable = false
      @list = [RPG::MoveCommand.new]
    end
    attr_accessor :repeat, :skippable, :list
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::MoveCommand
# -------------------------------------------------------------------------------------------------
module RPG
  class MoveCommand
    def initialize(code = 0, parameters = [])
      @code = code
      @parameters = parameters
    end
    attr_accessor :code, :parameters
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Actor
# -------------------------------------------------------------------------------------------------
module RPG
  class Actor
    def initialize
      @id = 0
      @name = ''
      @class_id = 1
      @initial_level = 1
      @final_level = 99
      @exp_basis = 30
      @exp_inflation = 30
      @character_name = ''
      @character_hue = 0
      @battler_name = ''
      @battler_hue = 0
      @parameters = Table.new(6, 100)
      (1..99).each do |i|
        @parameters[0, i] = 500 + i * 50
        @parameters[1, i] = 500 + i * 50
        @parameters[2, i] = 50 + i * 5
        @parameters[3, i] = 50 + i * 5
        @parameters[4, i] = 50 + i * 5
        @parameters[5, i] = 50 + i * 5
      end
      @weapon_id = 0
      @armor1_id = 0
      @armor2_id = 0
      @armor3_id = 0
      @armor4_id = 0
      @weapon_fix = false
      @armor1_fix = false
      @armor2_fix = false
      @armor3_fix = false
      @armor4_fix = false
    end
    attr_accessor :id, :name, :class_id, :initial_level, :final_level, :exp_basis, :exp_inflation,
                  :character_name, :character_hue, :battler_name, :battler_hue, :parameters,
                  :weapon_id, :armor1_id, :armor2_id, :armor3_id, :armor4_id, :weapon_fix,
                  :armor1_fix, :armor2_fix, :armor3_fix, :armor4_fix
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Class
# -------------------------------------------------------------------------------------------------
module RPG
  class Class
    def initialize
      @id = 0
      @name = ''
      @position = 0
      @weapon_set = []
      @armor_set = []
      @element_ranks = Table.new(1)
      @state_ranks = Table.new(1)
      @learnings = []
    end
    attr_accessor :id, :name, :position, :weapon_set, :armor_set, :element_ranks, :state_ranks,
                  :learnings
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Class::Learning
# -------------------------------------------------------------------------------------------------
module RPG
  class Class
    class Learning
      def initialize
        @level = 1
        @skill_id = 1
      end
      attr_accessor :level, :skill_id
    end
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Skill
# -------------------------------------------------------------------------------------------------
module RPG
  class Skill
    def initialize
      @id = 0
      @name = ''
      @icon_name = ''
      @description = ''
      @scope = 0
      @occasion = 1
      @animation1_id = 0
      @animation2_id = 0
      @menu_se = RPG::AudioFile.new('', 80)
      @common_event_id = 0
      @sp_cost = 0
      @power = 0
      @atk_f = 0
      @eva_f = 0
      @str_f = 0
      @dex_f = 0
      @agi_f = 0
      @int_f = 100
      @hit = 100
      @pdef_f = 0
      @mdef_f = 100
      @variance = 15
      @element_set = []
      @plus_state_set = []
      @minus_state_set = []
    end
    attr_accessor :id, :name, :icon_name, :description, :scope, :occasion, :animation1_id,
                  :animation2_id, :menu_se, :common_event_id, :sp_cost, :power, :atk_f, :eva_f,
                  :str_f, :dex_f, :agi_f, :int_f, :hit, :pdef_f, :mdef_f, :variance, :element_set,
                  :plus_state_set, :minus_state_set
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Item
# -------------------------------------------------------------------------------------------------
module RPG
  class Item
    def initialize
      @id = 0
      @name = ''
      @icon_name = ''
      @description = ''
      @scope = 0
      @occasion = 0
      @animation1_id = 0
      @animation2_id = 0
      @menu_se = RPG::AudioFile.new('', 80)
      @common_event_id = 0
      @price = 0
      @consumable = true
      @parameter_type = 0
      @parameter_points = 0
      @recover_hp_rate = 0
      @recover_hp = 0
      @recover_sp_rate = 0
      @recover_sp = 0
      @hit = 100
      @pdef_f = 0
      @mdef_f = 0
      @variance = 0
      @element_set = []
      @plus_state_set = []
      @minus_state_set = []
    end
    attr_accessor :id, :name, :icon_name, :description, :scope, :occasion, :animation1_id,
                  :animation2_id, :menu_se, :common_event_id, :price, :consumable,
                  :parameter_type, :parameter_points, :recover_hp_rate, :recover_hp,
                  :recover_sp_rate, :recover_sp, :hit, :pdef_f, :mdef_f, :variance, :element_set,
                  :plus_state_set, :minus_state_set
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Weapon
# -------------------------------------------------------------------------------------------------
module RPG
  class Weapon
    def initialize
      @id = 0
      @name = ''
      @icon_name = ''
      @description = ''
      @animation1_id = 0
      @animation2_id = 0
      @price = 0
      @atk = 0
      @pdef = 0
      @mdef = 0
      @str_plus = 0
      @dex_plus = 0
      @agi_plus = 0
      @int_plus = 0
      @element_set = []
      @plus_state_set = []
      @minus_state_set = []
    end
    attr_accessor :id, :name, :icon_name, :description, :animation1_id, :animation2_id, :price,
                  :atk, :pdef, :mdef, :str_plus, :dex_plus, :agi_plus, :int_plus, :element_set,
                  :plus_state_set, :minus_state_set
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Armor
# -------------------------------------------------------------------------------------------------
module RPG
  class Armor
    def initialize
      @id = 0
      @name = ''
      @icon_name = ''
      @description = ''
      @kind = 0
      @auto_state_id = 0
      @price = 0
      @pdef = 0
      @mdef = 0
      @eva = 0
      @str_plus = 0
      @dex_plus = 0
      @agi_plus = 0
      @int_plus = 0
      @guard_element_set = []
      @guard_state_set = []
    end
    attr_accessor :id, :name, :icon_name, :description, :kind, :auto_state_id, :price, :pdef,
                  :mdef, :eva, :str_plus, :dex_plus, :agi_plus, :int_plus, :guard_element_set,
                  :guard_state_set
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Enemy
# -------------------------------------------------------------------------------------------------
module RPG
  class Enemy
    def initialize
      @id = 0
      @name = ''
      @battler_name = ''
      @battler_hue = 0
      @maxhp = 500
      @maxsp = 500
      @str = 50
      @dex = 50
      @agi = 50
      @int = 50
      @atk = 100
      @pdef = 100
      @mdef = 100
      @eva = 0
      @animation1_id = 0
      @animation2_id = 0
      @element_ranks = Table.new(1)
      @state_ranks = Table.new(1)
      @actions = [RPG::Enemy::Action.new]
      @exp = 0
      @gold = 0
      @item_id = 0
      @weapon_id = 0
      @armor_id = 0
      @treasure_prob = 100
    end
    attr_accessor :id, :name, :battler_name, :battler_hue, :maxhp, :maxsp, :str, :dex, :agi, :int,
                  :atk, :pdef, :mdef, :eva, :animation1_id, :animation2_id, :element_ranks,
                  :state_ranks, :actions, :exp, :gold, :item_id, :weapon_id, :armor_id,
                  :treasure_prob
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Enemy::Action
# -------------------------------------------------------------------------------------------------
module RPG
  class Enemy
    class Action
      def initialize
        @kind = 0
        @basic = 0
        @skill_id = 1
        @condition_turn_a = 0
        @condition_turn_b = 1
        @condition_hp = 100
        @condition_level = 1
        @condition_switch_id = 0
        @rating = 5
      end
      attr_accessor :kind, :basic, :skill_id, :condition_turn_a, :condition_turn_b, :condition_hp,
                    :condition_level, :condition_switch_id, :rating
    end
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Troop
# -------------------------------------------------------------------------------------------------
module RPG
  class Troop
    def initialize
      @id = 0
      @name = ''
      @members = []
      @pages = [RPG::BattleEventPage.new]
    end
    attr_accessor :id, :name, :members, :pages
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Troop::Member
# -------------------------------------------------------------------------------------------------
module RPG
  class Troop
    class Member
      def initialize
        @enemy_id = 1
        @x = 0
        @y = 0
        @hidden = false
        @immortal = false
      end
      attr_accessor :enemy_id, :x, :y, :hidden, :immortal
    end
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Troop::Page
# -------------------------------------------------------------------------------------------------
module RPG
  class Troop
    class Page
      def initialize
        @condition = RPG::Troop::Page::Condition.new
        @span = 0
        @list = [RPG::EventCommand.new]
      end
      attr_accessor :condition, :span, :list
    end
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Troop::Page::Condition
# -------------------------------------------------------------------------------------------------
module RPG
  class Troop
    class Page
      class Condition
        def initialize
          @turn_valid = false
          @enemy_valid = false
          @actor_valid = false
          @switch_valid = false
          @turn_a = 0
          @turn_b = 0
          @enemy_index = 0
          @enemy_hp = 50
          @actor_id = 1
          @actor_hp = 50
          @switch_id = 1
        end
        attr_accessor :turn_valid, :enemy_valid, :actor_valid, :switch_valid, :turn_a, :turn_b,
                      :enemy_index, :enemy_hp, :actor_id, :actor_hp, :switch_id
      end
    end
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::State
# -------------------------------------------------------------------------------------------------
module RPG
  class State
    def initialize
      @id = 0
      @name = ''
      @animation_id = 0
      @restriction = 0
      @nonresistance = false
      @zero_hp = false
      @cant_get_exp = false
      @cant_evade = false
      @slip_damage = false
      @rating = 5
      @hit_rate = 100
      @maxhp_rate = 100
      @maxsp_rate = 100
      @str_rate = 100
      @dex_rate = 100
      @agi_rate = 100
      @int_rate = 100
      @atk_rate = 100
      @pdef_rate = 100
      @mdef_rate = 100
      @eva = 0
      @battle_only = true
      @hold_turn = 0
      @auto_release_prob = 0
      @shock_release_prob = 0
      @guard_element_set = []
      @plus_state_set = []
      @minus_state_set = []
    end
    attr_accessor :id, :name, :animation_id, :restriction, :nonresistance, :zero_hp, :cant_get_exp,
                  :cant_evade, :slip_damage, :rating, :hit_rate, :maxhp_rate, :maxsp_rate, :str_rate,
                  :dex_rate, :agi_rate, :int_rate, :atk_rate, :pdef_rate, :mdef_rate, :eva,
                  :battle_only, :hold_turn, :auto_release_prob, :shock_release_prob,
                  :guard_element_set, :plus_state_set, :minus_state_set
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Animation
# -------------------------------------------------------------------------------------------------
module RPG
  class Animation
    def initialize
      @id = 0
      @name = ''
      @animation_name = ''
      @animation_hue = 0
      @position = 1
      @frame_max = 1
      @frames = [RPG::Animation::Frame.new]
      @timings = []
    end
    attr_accessor :id, :name, :animation_name, :animation_hue, :position, :frame_max, :frames,
                  :timings
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Animation::Frame
# -------------------------------------------------------------------------------------------------
module RPG
  class Animation
    class Frame
      def initialize
        @cell_max = 0
        @cell_data = Table.new(0, 0)
      end
      attr_accessor :cell_max, :cell_data
    end
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Animation::Timing
# -------------------------------------------------------------------------------------------------
module RPG
  class Animation
    class Timing
      def initialize
        @frame = 0
        @se = RPG::AudioFile.new('', 80)
        @flash_scope = 0
        @flash_color = Color.new(255, 255, 255, 255)
        @flash_duration = 5
        @condition = 0
      end
      attr_accessor :frame, :se, :flash_scope, :flash_color, :flash_duration, :condition
    end
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::Tileset
# -------------------------------------------------------------------------------------------------
module RPG
  class Tileset
    def initialize
      @id = 0
      @name = ''
      @tileset_name = ''
      @autotile_names = [''] * 7
      @panorama_name = ''
      @panorama_hue = 0
      @fog_name = ''
      @fog_hue = 0
      @fog_opacity = 64
      @fog_blend_type = 0
      @fog_zoom = 200
      @fog_sx = 0
      @fog_sy = 0
      @battleback_name = ''
      @passages = Table.new(384)
      @priorities = Table.new(384)
      @priorities[0] = 5
      @terrain_tags = Table.new(384)
    end
    attr_accessor :id, :name, :tileset_name, :autotile_names, :panorama_name, :panorama_hue,
                  :fog_name, :fog_hue, :fog_opacity, :fog_blend_type, :fog_zoom, :fog_sx, :fog_sy,
                  :battleback_name, :passages, :priorities, :terrain_tags
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::CommonEvent
# -------------------------------------------------------------------------------------------------
module RPG
  class CommonEvent
    def initialize
      @id = 0
      @name = ''
      @trigger = 0
      @switch_id = 1
      @list = [RPG::EventCommand.new]
    end
    attr_accessor :id, :name, :trigger, :switch_id, :list
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::System
# -------------------------------------------------------------------------------------------------
module RPG
  class System
    def initialize
      @magic_number = 0
      @party_members = [1]
      @elements = [nil, '']
      @switches = [nil, '']
      @variables = [nil, '']
      @windowskin_name = ''
      @title_name = ''
      @gameover_name = ''
      @battle_transition = ''
      @title_bgm = RPG::AudioFile.new
      @battle_bgm = RPG::AudioFile.new
      @battle_end_me = RPG::AudioFile.new
      @gameover_me = RPG::AudioFile.new
      @cursor_se = RPG::AudioFile.new('', 80)
      @decision_se = RPG::AudioFile.new('', 80)
      @cancel_se = RPG::AudioFile.new('', 80)
      @buzzer_se = RPG::AudioFile.new('', 80)
      @equip_se = RPG::AudioFile.new('', 80)
      @shop_se = RPG::AudioFile.new('', 80)
      @save_se = RPG::AudioFile.new('', 80)
      @load_se = RPG::AudioFile.new('', 80)
      @battle_start_se = RPG::AudioFile.new('', 80)
      @escape_se = RPG::AudioFile.new('', 80)
      @actor_collapse_se = RPG::AudioFile.new('', 80)
      @enemy_collapse_se = RPG::AudioFile.new('', 80)
      @words = RPG::System::Words.new
      @test_battlers = []
      @test_troop_id = 1
      @start_map_id = 1
      @start_x = 0
      @start_y = 0
      @battleback_name = ''
      @battler_name = ''
      @battler_hue = 0
      @edit_map_id = 1
    end
    attr_accessor :magic_number, :party_members, :elements, :switches, :variables, :windowskin_name,
                  :title_name, :gameover_name, :battle_transition, :title_bgm, :battle_bgm,
                  :battle_end_me, :gameover_me, :cursor_se, :decision_se, :cancel_se, :buzzer_se,
                  :equip_se, :shop_se, :save_se, :load_se, :battle_start_se, :escape_se,
                  :actor_collapse_se, :enemy_collapse_se, :words, :test_battlers, :test_troop_id,
                  :start_map_id, :start_x, :start_y, :battleback_name, :battler_name,
                  :battler_hue, :edit_map_id
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::System::Words
# -------------------------------------------------------------------------------------------------
module RPG
  class System
    class Words
      def initialize
        @gold = ''
        @hp = ''
        @sp = ''
        @str = ''
        @dex = ''
        @agi = ''
        @int = ''
        @atk = ''
        @pdef = ''
        @mdef = ''
        @weapon = ''
        @armor1 = ''
        @armor2 = ''
        @armor3 = ''
        @armor4 = ''
        @attack = ''
        @skill = ''
        @guard = ''
        @item = ''
        @equip = ''
      end
      attr_accessor :gold, :hp, :sp, :str, :dex, :agi, :int, :atk, :pdef, :mdef, :weapon, :armor1,
                    :armor2, :armor3, :armor4, :attack, :skill, :guard, :item, :equip
    end
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::System::TestBattler
# -------------------------------------------------------------------------------------------------
module RPG
  class System
    class TestBattler
      def initialize
        @actor_id = 1
        @level = 1
        @weapon_id = 0
        @armor1_id = 0
        @armor2_id = 0
        @armor3_id = 0
        @armor4_id = 0
      end
      attr_accessor :actor_id, :level, :weapon_id, :armor1_id, :armor2_id, :armor3_id, :armor4_id
    end
  end
end

# -------------------------------------------------------------------------------------------------
# RPG::AudioFile
# -------------------------------------------------------------------------------------------------
module RPG
  class AudioFile
    def initialize(name = '', volume = 100, pitch = 100)
      @name = name
      @volume = volume
      @pitch = pitch
    end
    attr_accessor :name, :volume, :pitch
  end
end
