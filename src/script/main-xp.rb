# Copyright (c) 2022 Xiaomi Guo
# Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

def test_loop
  Graphics.frame_rate = 60

  3000.times do
    Graphics.update
    Input.update
    exit if Input.trigger?(Input::C)

    yield if block_given?
  end
end
