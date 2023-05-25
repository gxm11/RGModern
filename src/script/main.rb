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
# entry of RGSS

def rgss_main
  # clear screen
  Graphics.update

  # load Scripts.rxdata and eval
  load_data('Data/Scripts.rxdata').each do |_id, title, data|
    script = Zlib::Inflate.inflate(data)
    force_utf8_encode(script)
    begin
      eval script, nil, title
    rescue SignalException
      puts 'The quit event is catched, program stops safely.'
      return
    rescue SystemExit
      puts 'The exit() method is catched, program stops safely.'
      return
    rescue Exception => e
      msg = ['Error occurs when load Data/Scripts.rxdata.']
      msg << e.to_s
      msg += e.backtrace.collect { |x| ' - ' + x }

      puts msg.join("\n")
      File.open('error.log', 'w') do |f|
        f << Time.now << "\n"
        f << msg.join("\n")
      end
      msgbox "Error occurs when load Data/Scripts.rxdata.\n" +
             'Please check the error.log for more infomation.'
      return
    end
  end
end

rgss_main
