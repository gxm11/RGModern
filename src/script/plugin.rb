b = Bitmap.new('6.webp')
b.hue_change(120)
b.hue_change(120)
# Graphics.present2
b.save_png('7.png')

RGM::Base.synchronize(3)
# exit
Thread.new do
  $s = Sprite.new
  $s.z = 3000
  $s.bitmap = b
  $s.x = 30
  $s.y = 30
  loop do
    sleep 0.05
    $s.x += 1
    $s.y += 1
  end
end
