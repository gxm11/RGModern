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

def debug(b)
  # 使用方法：
  # debug(binding)，在执行到这一句时进入 debug 模式。

  STDOUT.puts '>>> DEBUG MODE <<<'
  loop do
    begin
      STDOUT.print('>: ')
      code = gets
      ret = eval(code, b)
    rescue Exception
      $!.inspect.split("\n").each do |line|
        STDOUT.puts("!> #{line}")
      end
      retry
    end
    if code == "\n"
      STDOUT.puts '>>> EXIT DEBUG <<<'
      break
    else
      STDOUT.puts("=: #{ret}")
    end
  end
end

def debug(b); end if RGM::Config::Build_Mode >= 3 || !$DEBUG

def msgbox(text)
  RGM::Base.message_show(text.to_s)
end

def force_utf8_encode(obj)
  case obj
  when Array
    obj.each { |item| force_utf8_encode(item) }
  when Hash
    obj.each { |_key, item| force_utf8_encode(item) }
  when String
    obj.force_encoding('utf-8')
  else
    if obj.class.name.start_with?('RPG::')
      obj.instance_variables.each do |name|
        item = obj.instance_variable_get(name)
        force_utf8_encode(item)
      end
    end
  end
end

def load_data(fn)
  fn = Finder.find(fn, :data)
  data = nil
  File.open(fn, 'rb') do |f|
    data = Marshal.load(f)
  end
  force_utf8_encode(data)
  data
end

def save_data(obj, fn)
  File.open(fn, 'wb') do |f|
    Marshal.dump(obj, f)
  end
end

def load_file(fn)
  File.open('./src/' + fn, 'rb') do |f|
    return f.read
  end
end

if RGM::Config::Build_Mode >= 3
  def load_data(fn)
    data = nil
    if fn.start_with?('Data/')
      bin = RGM::Base.embeded_load(fn)
      data = Marshal.load(bin)
    else
      fn = Finder.find(fn, :data)
      File.open(fn, 'rb') do |f|
        data = Marshal.load(f)
      end
    end
    force_utf8_encode(data)
    data
  end
end

if RGM::Config::Build_Mode >= 2
  def load_file(fn)
    RGM::Base.embeded_load(fn)
  end
end
