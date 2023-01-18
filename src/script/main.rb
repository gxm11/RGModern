# Copyright (c) 2022 Xiaomi Guo
# Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.
# entry of RGSS

def rgss_main
  scripts = load_data('Data/Scripts.rxdata')
  Graphics.transition
  RGM::Base.synchronize(3)

  scripts.each do |_id, title, data|
    script = Zlib::Inflate.inflate(data)
    force_utf8_encode(script)
    begin
      eval script, nil, title
    rescue SignalException
      puts 'The interrupt signal is catched, program stops safely.'
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
      return
    end
  end
end

rgss_main
