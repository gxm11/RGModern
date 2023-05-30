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

module Graphics
  # ---------------------------------------------------------------------------
  # The module that carries out graphics processing.
  # ---------------------------------------------------------------------------

  module_function

  def update
    # Refreshes the game screen and advances time by 1 frame.
    # This method must be called at set intervals.
    update_fps
    update_synchronize

    if @@low_fps_mode
      @@low_fps_countdown -= 1
      @@low_fps_countdown += @@low_fps_ratio if @@low_fps_countdown <= 0
      return if @@low_fps_countdown < @@low_fps_ratio
    end

    unless @@freeze_bitmap
      RGM::Base.graphics_update
      RGM::Base::Temp.clear
    end
    present
  end

  def update_synchronize
    return unless @@flag_synchronize

    @@flag_synchronize = false
    t = Time.now

    RGM::Config::Max_Workers.times do |i|
      RGM::Base.synchronize(i) if i != 0
    end

    delta_t = Time.now - t
    puts format('Warn: wait %.3fs for synchronization', delta_t) if delta_t > 0.1
  end

  def update_fps
    if @@show_fps
      delta_count = @@frame_count - @@fps_last_frame_count
      delta_time = Time.now.to_f - @@fps_last_time

      # 超过 1s，或者frame_rate帧过后，主动更新 title
      if delta_count > frame_rate || delta_time > 1.0
        RGM::Ext::Window.set_fps(delta_count / delta_time)
        @@fps_last_frame_count += delta_count
        @@fps_last_time += delta_time
      end
    end
    # toggle fps
    return unless Input.trigger?(Input::FPS_TOGGLE)

    if @@show_fps
      @@show_fps = false
      RGM::Ext::Window.set_fps(nil)
    else
      @@show_fps = true
      @@fps_last_frame_count = @@frame_count
      @@fps_last_time = Time.now.to_f
      RGM::Ext::Window.set_fps(-1)
    end
  end

  def freeze
    # Fixes the current screen in preparation for transitions.
    # Screen rewrites are prohibited until the transition method is called.
    @@freeze_bitmap = snap_to_bitmap
  end

  def transition(duration = 8, filename = '', vague = 40)
    # Carries out a transition from the screen fixed in Graphics.freeze to the current screen.
    # [duration] is the number of frames the transition will last.
    #            When omitted, this value is set to 8.
    # [filename] specifies the transition graphic file name.
    #            When not specified, a standard fade will be used.
    #            Also automatically searches files included in RGSS-RTP and encrypted archives.
    #            File extensions may be omitted.
    # [vague] sets the ambiguity of the borderline between the graphic's starting and ending points.
    #         The larger the value, the greater the ambiguity. When omitted, this value is set to 40.
    @@freeze_bitmap ||= snap_to_bitmap
    RGM::Base.graphics_update
    @@current_bitmap = snap_to_bitmap

    duration /= @@low_fps_ratio if @@low_fps_mode

    if filename.empty? || RGM::Config::Render_Driver == RGM::Driver::Software
      duration.times do |i|
        RGM::Base.graphics_transition(
          @@freeze_bitmap.id, @@current_bitmap.id, i / duration.to_f, 0, 0
        )
        present
      end
    else
      transition_bitmap = Bitmap.new(filename)
      duration.times do |i|
        RGM::Base.graphics_transition(
          @@freeze_bitmap.id, @@current_bitmap.id, i / duration.to_f, transition_bitmap.id, vague.to_i
        )
        present
      end
      transition_bitmap.dispose
    end

    @@freeze_bitmap.dispose
    @@current_bitmap.dispose
    @@freeze_bitmap = nil
    @@current_bitmap = nil
  end

  def frame_reset
    # Resets the screen refresh timing.
    # After a time-consuming process, call this method to prevent extreme frame skips.

    # 避免一帧以内多次同步，这里只是设置同步的标记。
    # 在下一次 update 前，如果标记为 true，则会等待渲染线程的同步信号。
    @@flag_synchronize = true
  end

  def present
    RGM::Base.present_window
    RGM::Base.check_delay(Graphics.frame_rate)
    @@frame_count += 1
  end

  def resize_window(width, height, scale_mode = 0)
    RGM::Ext::Window.resize(width, height, scale_mode)

    # resize window 后，现存的 bitmap 可能会损坏，需要重新读取 Cache
    # 所以 resize_window 需尽可能在程序开始时
    # 使用 Direct3D11 渲染不会损坏现存的 bitmap
    puts '[Warning] resize window might destory all bitmaps.'
    puts 'Bitmaps in RPG::Cache are automatically reloaded.'

    RPG::Cache.reload

    frame_reset
  end

  def resize_screen(width, height)
    @@width = width.to_i
    @@height = height.to_i
    RGM::Base.resize_screen(@@width, @@height)
  end

  def snap_to_bitmap
    bitmap = Bitmap.new(@@width, @@height)
    bitmap.capture_screen
    bitmap
  end

  def frame_count
    @@frame_count
  end

  def frame_count=(value)
    @@frame_count = value
  end

  def frame_rate=(value)
    value = 10 if value < 10
    @@frame_rate = value
  end

  def frame_rate
    @@low_fps_mode ? @@frame_rate / @@low_fps_ratio : @@frame_rate
  end

  def width
    @@width
  end

  def height
    @@height
  end

  def enable_low_fps(ratio)
    return if ratio == 1

    @@low_fps_mode = true
    @@low_fps_ratio = ratio
  end

  # The screen's refresh rate count. Set this property to 0 at game start and
  # the game play time (in seconds) can be calculated by dividing this value by
  # the frame_rate property value.
  @@frame_count = 0

  # 帧率，在 Graphics.update 中使用以限制逻辑线程的运行速度
  @@frame_rate = 60

  # 画面和窗口的大小
  @@width = 640
  @@height = 480

  @@freeze_bitmap = nil
  @@current_bitmap = nil
  @@flag_synchronize = false

  @@show_fps = (RGM::Config::Build_Mode < 2)
  @@fps_last_frame_count = 0
  @@fps_last_time = Time.now.to_f

  @@low_fps_mode = false
  @@low_fps_ratio = 2
  @@low_fps_countdown = 0
end
