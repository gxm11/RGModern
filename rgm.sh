mkdir ../RGModern/ruby32/include ../RGModern/ruby32/lib ../RGModern/ruby32/script
cp -r ruby-3.2.1/include/ruby* ../RGModern/ruby32/include
cp -r ruby-3.2.1/*.a ruby-3.2.1/ext/*/*.a ../RGModern/ruby32/lib
cp -r ruby-3.2.1/.ext/include/x64-mingw-ucrt/ruby/config.h ../RGModern/ruby32/include/ruby
cp -r ruby-3.2.1/ext/fiddle/lib/* ../RGModern/ruby32/script
sed -i -r "s/^require '.+'/\# \0/g" ../RGModern/ruby32/script/fiddle.rb
sed -i -r "s/^require '.+'/\# \0/g" ../RGModern/ruby32/script/fiddle/*.rb
cp -r ../RGModern/ruby32/script/* ../RGModern/src/script