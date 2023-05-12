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

class Table
  # The multidimensional array class. Each element takes up 2 signed bytes,
  # ranging from -32,768 to 32,767. Ruby's Array class does not run efficiently
  # when handling large amounts of data, hence the inclusion of this class.

  # 在 C++ 层中数据存储在 std::vector<int16_t> 中，而所有的 vector 都存储在 Hash 中。
  # 不会通过哈希表查找 Table 对应的 vector，而是通过 @data_ptr 里记录的 &vector.front()
  # 位置，加上偏移进行数据访问。在 STL 的 vector 实现下只有 create 和 resize 可能会改
  # 变 &vector.front() 的地址，这两个函数都会返回新的 @data_ptr 的值，从而合法访问数据。

  attr_reader :id, :xsize, :ysize, :zsize

  def self.create_finalizer
    proc { |object_id| RGM::Base.table_dispose(object_id) }
  end

  def initialize(xsize, ysize = 1, zsize = 1)
    # Creates a Table object.
    # Specifies the size of each dimension in the multidimensional array.
    # 1-, 2-, and 3-dimensional arrays are possible.
    # Arrays with no parameters are also permitted.

    @id = object_id
    @xsize = xsize
    @ysize = ysize
    @zsize = zsize

    @data_ptr = RGM::Base.table_create(@id, xsize, ysize, zsize)
    ObjectSpace.define_finalizer(self, self.class.create_finalizer)
  end

  def resize(xsize, ysize = 1, zsize = 1)
    # Change the size of the array. All data from before the size change is retained.

    @xsize = xsize
    @ysize = ysize
    @zsize = zsize
    @data_ptr = RGM::Base.table_resize(@id, xsize, ysize, zsize)
  end

  def [](x, y = 0, z = 0)
    if invalid?(x, y, z)
      nil
    else
      index = x + @xsize * (y + @ysize * z)
      RGM::Base.table_get(@data_ptr, index)
    end
  end

  def []=(x, y = 0, z = 0, value)
    raise 'Invalid element index of table' if invalid?(x, y, z)

    index = x + @xsize * (y + @ysize * z)
    RGM::Base.table_set(@data_ptr, index, value)
  end

  def inspect
    format('#<Table:%d> [%d x %d x %d] (0x%016x)', object_id, @xsize, @ysize, @zsize, @data_ptr)
  end

  def self._load(s)
    obj = Table.new(*s[4...16].unpack('L3'))
    RGM::Base.table_load(obj.id, s)
    obj
  end

  def _dump(_depth = 1)
    size = @xsize * @ysize * @zsize
    dimension = 3
    dimension -= 1 if @zsize == 1
    dimension -= 1 if @ysize == 1
    str = [dimension, @xsize, @ysize, @zsize, size].pack('L5')
    str << RGM::Base.table_dump(@id)
    str
  end

  def clone
    Marshal.load(Marshal.dump(self))
  end

  def invalid?(x, y, z)
    x >= @xsize || y >= @ysize || z >= @zsize || x < 0 || y < 0 || z < 0
  end
end
