# Copyright (c) 2022 Xiaomi Guo
# Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

def debug(b, key = Input::DEBUG)
  # 使用方法：
  # 1. debug(binding, nil)，在执行到这一句时进入 debug 模式。
  # 2. debug(binding, key)，在执行到这一句时，如果 key 按下，则进入 debug 模式。
  #    key 的默认值是 Input::DEBUG，默认绑定到 F5（见下面）。
  return if key && !Input.press?(key)

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

def debug; end if RGM::BuildMode >= 2
