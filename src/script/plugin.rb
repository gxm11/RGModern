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
module Input
  module_function

  @@bcount = 0

  def trigger?(key)
    if key == Input::B && ($scene.is_a?(Scene_Map) || $scene.is_a?(Scene_Menu))
      @@bcount += 1
      return true if @@bcount % 5 == 0
    end

    key = const_get(key) if key.is_a?(Symbol)
    RGM::Base.input_trigger(key)
  end
end
