RGM::Base.synchronize(3)

Thread.start do
  sleep 1
  music = RGM::Ext::Music.new(Finder.find('001-Battle01.mp3', :music))
  music.play(-1)
  RGM::Ext.music_set_volume(100)
  sleep 3
  RGM::Ext.music_halt
end
