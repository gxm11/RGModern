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

module RGM::Base
  module_function

  def decorate_drawable(klass)
    # intern attributes: disposed, id, viewport, visible, z
    decorate_drawable_base klass

    # value-type attributes, as integer / double / bool
    values = %i[active angle blend_type bush_depth cursor_count flash_hidden height
                mirror ox oy pause repeat_x repeat_y scale_mode stretch update_count
                width x y zoom_x zoom_y]
    decorate_drawable_setter klass, Code_SetValue, *values

    # bitmap-type attributes, as SDL_Texture
    bitmaps = %i[bitmap contents windowskin tileset]
    decorate_drawable_setter klass, Code_SetBitmap, *bitmaps

    # table-type attributes, as std::vector<int16_t>
    tables = %i[flash_data map_data priorities]
    decorate_drawable_setter klass, Code_SetTable, *tables

    # opacity-type attributes, automatically corrected in range 0-255
    opacities = %i[back_opacity contents_opacity opacity]
    decorate_drawable_setter klass, Code_SetOpacity, *opacities
  end

  def decorate_drawable_base(klass)
    drawable_create = RGM::Base.method("#{klass.name.downcase}_create".to_sym)

    klass.class_eval do
      # -----------------------------------------------------------------------
      # initializer / finalizer
      # -----------------------------------------------------------------------
      def self.create_finalizer(viewport, id)
        proc {
          RGM::Base.drawable_dispose(viewport, id)
        }
      end

      define_method(:drawable_create, ->(obj) { drawable_create.call(obj) })

      attr_reader :viewport, :visible, :z

      alias_method :origin_initialize, :initialize

      def initialize(viewport = nil)
        # RGSS 的实现里，Drawable 类虽然定义了 getter 和 setter 方法，但并没有实例变量。
        # 据可靠消息，Drawable 类其实是先定义了相应的 C Struct，然后绑定到 ruby 的对象上。
        # RGM 并没有这么做，而是在 ruby 和 C++ 层各自定义了一组相似但不完全一样的数据结构，
        # 同时定义了 RGM::Base 模块下的一些方法供 ruby 层调用，以操作 C++ 层相应的数据。
        # 而 C++ 层也能通过 instance_variable 类来访问 ruby 层的各实例变量，不过是只读的。
        # 以这种宽松的耦合方式，Drawable 类的实现更加自然，对后续的扩展或改动也更加友好。

        # 初始化内置的实例变量，这些实例变量是所有 Drawable 都拥有的。
        @disposed = false
        @id = RGM::Base.new_id
        @viewport = viewport
        @visible = true
        @z = 0

        # 初始化其他实例变量
        origin_initialize

        # 构建 C++ 层的 Drawable 对象并互相绑定
        @data_ptr = drawable_create(self)

        # 定义 finalizer 使得自身被垃圾回收时，C++ 层的 Drawable 对象也被析构
        # 由于 finalizer 只会被触发一次，所以不会造成多次析构
        ObjectSpace.define_finalizer(self, self.class.create_finalizer(@viewport, @id))
      end

      def dispose
        # 调用 dispose 方法只会导致该 Drawable 对象永远不可见，与 RGSS 文档中的说明不同。
        # 这一设计有以下 2 点考虑：
        # 1. RGSS 的默认脚本，Drawable 类型的对象通常作为实例变量，由场景类管理，而只在场景切换时才会
        #    调用 dispose 方法释放 Drawable，随后场景类对应的实例会立刻进入 GC。换句话说，RGSS 中的
        #    Drawable 会在 dispose 后立刻进入 GC，从而触发 finalizer 析构相应的资源，最终表现并无本
        #    质区别。
        # 2. RGSS 中的 Drawable 类型在调用 dispose 方法后，再访问属性（比如 visible）会导致 RGSSERROR。
        #    而现在的设计则不会导致此类报错，从而无需在访问属性前判断是否已经被释放，提升效率。
        @visible = false
        @disposed = true
        @data_ptr = nil
        RGM::Base.drawable_dispose(@viewport, @id)
      end

      def disposed?
        @disposed
      end

      def visible=(visible)
        @visible = visible & (!@disposed)
      end

      def z=(z)
        return @z if @z == z

        @z = RGM::Base.drawable_set_z(self, @viewport, z.to_i) unless @disposed
      end
    end
  end

  def decorate_drawable_setter(klass, pattern, *symbols)
    symbols.collect(&:to_s).each do |name|
      klass.class_eval(
        pattern.gsub('__attr__', name).gsub('__class__', klass.name.downcase)
      )
    end
  end

  def decorate_builtin_setter(klass, name, min, max)
    klass.class_eval(
      Code_SetBounded
        .gsub('__attr__', name.to_s)
        .gsub('__class__', klass.name.downcase)
        .gsub('__max__', max.to_s)
        .gsub('__min__', min.to_s)
    )
  end

  Code_SetValue = <<~END
    if defined? :@__attr__
      def __attr__=(value)
        value = @__attr__.is_a?(Integer) ? value.to_i : value
        if @__attr__ != value
          @__attr__ = value
          RGM::Base.__class___refresh_value(@data_ptr, RGM::Word::Attribute___attr__) unless @disposed
        end
        @__attr__
      end
    end
  END

  Code_SetBitmap = <<~END
    if defined? :@__attr__
      def __attr__=(bitmap)
        raise ArgumentError, 'Argument 1 should be nil or Bitmap.' if bitmap && !bitmap.is_a?(Bitmap)
        if @__attr__ != bitmap
          @__attr__ = bitmap
          RGM::Base.__class___refresh_value(@data_ptr, RGM::Word::Attribute___attr__) unless @disposed
        end
        @__attr__
      end
    end
  END

  Code_SetTable = <<~END
    if defined? :@__attr__
      def __attr__=(table)
        raise ArgumentError, 'Argument 1 should be nil or Table.' if table && !table.is_a?(Table)
        if @__attr__ != table
          @__attr__ = table
          RGM::Base.__class___refresh_value(@data_ptr, RGM::Word::Attribute___attr__) unless @disposed
        end
        @__attr__
      end
    end
  END

  Code_SetOpacity = <<~END
    if defined? :@__attr__
      def __attr__=(opacity)
        opacity = opacity.to_i
        opacity = 255 if opacity > 255
        opacity = 0 if opacity < 0

        if @__attr__ != opacity
          @__attr__ = opacity
          RGM::Base.__class___refresh_value(@data_ptr, RGM::Word::Attribute___attr__) unless @disposed
        end
        @__attr__
      end
    end
  END

  Code_SetBounded = <<~END
    def __attr__=(value)
      value = value.to_i
      value = __max__ if value > __max__
      value = __min__ if value < __min__

      @__attr__ = value
    end
  END
end
