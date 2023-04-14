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
  module Ext
    # def textinput_edit_text; end
    # def textinput_edit_pos; end
    # def textinput_start(x, y, width, height); end
    # def textinput_stop; end
    class TextBox
      # usage:
      # class Scene_Title
      #   alias _textbox_update update
      #   def update
      #     unless @textbox
      #       @textbox = RGM::Ext::TextBox.new
      #       @textbox.default_text = 'hello, rgm'
      #       @textbox.set_rect(128, 128, 192, 32)
      #       @textbox.enable_toggle = true
      #     end
      #     @textbox.update
      #     puts @textbox.last_text if Input.trigger?(Input::A)

      #     _textbox_update
      #   end
      # end

      # 执行 Input.update 方法时，如果检测到了输入事件，则会自动设置此变量为 true
      @@need_refresh = false

      Text = Struct.new(:left, :edit, :right)

      attr_reader :last_text
      attr_accessor :enable_toggle, :enable_clear, :background_color, :text_color, :default_text

      def initialize
        @sprite = Sprite.new
        @bitmap = Bitmap.new(32, 32)
        @sprite.bitmap = @bitmap
        @sprite.z = 65_535

        @text = Text.new('', '', '')
        @last_text = ''

        @background_color = Color.new(255, 255, 255, 255)
        @text_color = Color.new(0, 0, 0, 255)
        @enable_toggle = false
        @enable_clear = true
        @default_text = ''

        @suspend = false
        disable
      end

      def dispose
        disable
        @sprite.bitmap.dispose
        @sprite.dispose
      end

      def update
        if @@need_refresh
          @need_refresh = true
          @@need_refresh = false
        end
        # active 时
        if active
          # 更新画面内容，清空 last_text
          update_edit
          @last_text.clear
          @suspend = false
          if Input.trigger?(Input::TEXT_CONFIRM)
            # confirm 键清空输入，并修改 last_text 的值
            disable
            @last_text << @text.left << @text.edit << @text.right
            clear_text
          elsif Input.trigger?(Input::TEXT_TOGGLE) && enable_toggle
            # toggle 键禁用输入法，但是保存 text 的状态
            disable
            @suspend = true
          end
          # 清空 Input，后续只能读到空的输入
          Input.reset
        # inactive 时
        elsif Input.trigger?(Input::TEXT_TOGGLE) && enable_toggle
          # toggle 键激活输入法
          enable
        end
      end

      def update_edit
        if @text.edit.empty?
          if Input.repeat?(Input::LEFT) && !@text.left.empty?
            chs = @text.left.scan(/./)
            ch = chs.pop
            @text.left = chs.join
            @text.right = ch << @text.right

            @need_refresh = true
          end

          if Input.repeat?(Input::RIGHT) && !@text.right.empty?
            chs = @text.right.scan(/./)
            ch = chs.shift
            @text.left << ch
            @text.right = chs.join

            @need_refresh = true
          end

          if Input.repeat?(Input::TEXT_BACKSPACE) && !@text.left.empty?
            chs = @text.left.scan(/./)
            ch = chs.pop
            @text.left = chs.join

            @need_refresh = true
          end

          if Input.trigger?(Input::TEXT_CLEAR) && @enable_clear
            clear_text

            @need_refresh = true
          end
        end

        refresh if @need_refresh
      end

      def get_edit_text
        edit_text = Ext.textinput_edit_text
        edit_pos = Ext.textinput_edit_pos

        [edit_text, edit_pos]
      end

      def refresh
        @need_refresh = false

        edit_text, edit_pos = get_edit_text

        if edit_pos == -1
          @text.left << edit_text
          @text.edit.clear
        else
          @text.edit = edit_text
        end

        @bitmap.fill_rect(@bitmap.rect, @background_color)
        @bitmap.font.color = @text_color

        text_x = 4
        text_rect = @bitmap.text_size(@text.left)
        @bitmap.font.underlined = false
        @bitmap.draw_text(text_x, 0, text_rect.width, height, @text.left)
        text_x += text_rect.width

        text_rect = @bitmap.text_size(@text.edit)
        @bitmap.font.underlined = true
        @bitmap.draw_text(text_x, 0, text_rect.width, height, @text.edit)
        text_x += text_rect.width

        if @text.edit.empty?
          @bitmap.fill_rect(text_x, 1, 1, height - 2, @bitmap.font.color)
        else
          chs = @text.edit.scan(/./)
          delta_x = - text_rect.width
          delta_x += @bitmap.text_size(chs[0...edit_pos].join).width if edit_pos > 0
          @bitmap.fill_rect(text_x + delta_x, 1, 1, height - 2, @bitmap.font.color)
        end

        text_rect = @bitmap.text_size(@text.right)
        @bitmap.font.underlined = false
        @bitmap.draw_text(text_x, 0, text_rect.width, height, @text.right)
        text_x += text_rect.width
      end

      def enable
        return if @sprite.visible

        Ext.textinput_start(x, y + height, 0, 0)
        @sprite.visible = true
        # 如果是 STOP 状态，则设置 default_text
        unless @suspend
          clear_text
          @text.left = @default_text.clone
          @need_refresh = true
        end
      end

      def disable
        return unless @sprite.visible

        Ext.textinput_stop
        @sprite.visible = false
      end

      def clear_text
        @text.left.clear
        @text.edit.clear
        @text.right.clear
      end

      def set_rect(x, y, width, height)
        @sprite.x = x
        @sprite.y = y

        if @bitmap.width != width || @bitmap.height != height
          @bitmap.dispose
          @bitmap = Bitmap.new(width, height)
          @sprite.bitmap = @bitmap

          @need_refresh = true
        end
      end

      def x
        @sprite.x
      end

      def y
        @sprite.y
      end

      def z
        @sprite.z
      end

      def z=(z)
        @sprite.z = z
      end

      def width
        @bitmap.width
      end

      def height
        @bitmap.height
      end

      def active
        @sprite.visible
      end
    end

    Ext.textinput_stop
  end
end
