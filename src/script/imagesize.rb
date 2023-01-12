# Copyright (c) 2022 Xiaomi Guo
# Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

module Imagesize
  # get the width and height of figure file
  # usage:
  # width, height = Imagesize.load(filename)
  # width, height = Imagesize.load_raw(f)

  TYPE_UNKNOWN = 0
  TYPE_BMP = 1
  TYPE_GIF = 2
  TYPE_PNG = 3
  TYPE_JPEG = 4
  TYPE_TIFF = 5
  TYPE_WEBP = 6

  Header_BMP = [66, 77].pack('C2')
  Header_GIF = [71, 73, 70].pack('C3')
  Header_PNG = [137, 80, 78, 71].pack('C4')
  Header_JPEG = [255, 216].pack('C2')
  Header_TIFF_0 = [73, 73, 42, 0].pack('C4')
  Header_TIFF_1 = [77, 77, 0, 42].pack('C4')

  module_function

  def load(filename)
    load_raw(File.binread(filename))
  end

  def load_raw(f)
    image_size(f, *image_type(f))
  end

  def image_type(f)
    return TYPE_BMP, 0 if f[0...2] == Header_BMP
    return TYPE_GIF, 0 if f[0...3] == Header_GIF
    return TYPE_PNG, 0 if f[0...4] == Header_PNG
    return TYPE_JPEG, 0 if f[0...2] == Header_JPEG
    return TYPE_TIFF, 0 if f[0...4] == Header_TIFF_0
    return TYPE_TIFF, 1 if f[0...4] == Header_TIFF_1

    if f[0...4] == 'RIFF' && f[8...12] == 'WEBP'
      case f[12...16]
      when 'VP8 '
        return TYPE_WEBP, 0
      when 'VP8L'
        return TYPE_WEBP, 1
      when 'VP8X'
        return TYPE_WEBP, 2
      end
    end
    [TYPE_UNKNOWN, 0]
  end

  def image_size(f, type, type2 = 0)
    case type
    when TYPE_UNKNOWN
      raise 'Unknown image type.'
    when TYPE_BMP
      f[18...26].unpack('V2')
    when TYPE_GIF
      f[6...10].unpack('v2')
    when TYPE_PNG
      f[16...24].unpack('N2')
    when TYPE_JPEG
      image_size_jpeg(f)
    when TYPE_TIFF
      image_size_tiff(f, type2)
    when TYPE_WEBP
      image_size_webp(f, type2)
    else
      raise 'Invalid image type.'
    end
  end

  def image_size_jpeg(f)
    i = 2
    loop do
      sig, type = f[i...i + 2].unpack('C2')
      raise 'Invalid JPEG format.' if sig != 255
      break if type == 192

      size = f[i + 2...i + 4].unpack1('n')
      i += size + 2
    end
    f[i + 5...i + 9].unpack('n2').reverse
  end

  def image_size_tiff(f, type)
    fmt_short = type == 0 ? 'v' : 'n'
    fmt_long = type == 0 ? 'V' : 'N'

    offset = f[4...8].unpack1(fmt_long)

    i = offset + 2
    w = 0
    h = 0
    loop do
      return w, h if w * h != 0

      if f[i...i + 2] == [256].pack(fmt_short)
        w = if f[i + 2...i + 4] == [3].pack(fmt_short)
              f[i + 8...i + 10].unpack1(fmt_short)
            else
              f[i + 8...i + 12].unpack1(fmt_long)
            end
      end

      if f[i...i + 2] == [257].pack(fmt_short)
        h = if f[i + 2...i + 4] == [3].pack(fmt_short)
              f[i + 8...i + 10].unpack1(fmt_short)
            else
              f[i + 8...i + 12].unpack1(fmt_long)
            end
      end

      i += 12
    end
  end

  def image_size_webp(f, type)
    case type
    when 0
      f[26...30].unpack('v2')
    when 1
      a, b = f[21...25].unpack('v2')
      [(a & 0x3fff) + 1, (((b << 2) | (a >> 14)) & 0x3fff) + 1]
    when 2
      a, b, c = f[22...28].unpack('v3')
      [(a | ((b & 0xff) << 16)) + 1, ((c << 8) | (b >> 8)) + 1]
    else
      raise 'Invalid WEBP type.'
    end
  end
end
