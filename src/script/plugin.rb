b = Bitmap.new('6.png')
b.hue_change(120)
b.save_png('7.png')

RGM::Base.synchronize(3)
exit
