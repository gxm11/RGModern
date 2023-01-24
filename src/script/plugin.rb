b = Bitmap.new('6.webp')
b.hue_change(120)
b.save_png('7.png')

# RGM::Base.synchronize(3)
# exit
$s = Sprite.new
$s.z = 3000
$s.bitmap = b
$s.x = 30
$s.y = 30
