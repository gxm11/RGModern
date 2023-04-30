# -----------------------------------------------
# packages settings
# -----------------------------------------------
include ext/settings.txt
GITHUB_URL ?= https://github.com

path_packages := ./packages
path_download := $(path_packages)/download

system := $(shell uname | cut -d'-' -f1)
ifeq ($(system), MINGW64_NT)
	path_vendors := $(subst \,/,$(shell cygpath -aw ./))/vendors
else
	path_vendors := $(subst \,/,$(shell cygpath -aw ./))/vendors_mingw32
endif

# -----------------------------------------------
# ruby static library
# -----------------------------------------------
RUBY_VERSION := $(RUBY_MAJOR_VERSION).$(RUBY_MINOR_VERSION).$(RUBY_PATCH_VERSION)
RUBY_LIBRARY_VERSION := $(RUBY_M_VERSION)0

url_ruby_static := https://cache.ruby-lang.org/pub/ruby/$(RUBY_MAJOR_VERSION).$(RUBY_MINOR_VERSION)/ruby-$(RUBY_VERSION).tar.gz
zip_ruby_static := $(path_download)/ruby-$(RUBY_VERSION).tar.gz
path_ruby_static := $(path_packages)/ruby-$(RUBY_VERSION)

ifeq ($(system), MINGW64_NT)
	lib_ruby_static := $(path_ruby_static)/libx64-ucrt-ruby$(RUBY_LIBRARY_VERSION)-static.a
else
	lib_ruby_static := $(path_ruby_static)/libmsvcrt-ruby$(RUBY_LIBRARY_VERSION)-static.a
endif

lib_ruby := ./third_party/ruby$(RUBY_M_VERSION)
configure_ruby = --disable-install-doc --disable-jit-support --disable-rubygems --with-static-linked-ext --with-out-ext=openssl --without-gmp

# -----------------------------------------------
# msys2 packages (including SDL2 library)
# -----------------------------------------------
pkgs = SDL2 SDL2_ttf SDL2_image SDL2_mixer SDL2_gfx SDL2_net zlib libzip libyaml
pkgs_msys2 = $(addprefix $(MINGW_PACKAGE_PREFIX)-,$(pkgs))
pkgs_msys2 += bison libffi gmp upx p7zip vim unzip

# -----------------------------------------------
# miscs
# -----------------------------------------------
miscs = history resource SDL2*.dll
miscs += third_party/ruby$(RUBY_M_VERSION) Doxyfile
miscs += Project1/Audio Project1/Graphics Project1/Graphics.zip
miscs += Project1/*.dll Project1/RPGXP.chm Project1/RPGXP.chw 

# -----------------------------------------------
# tasks
# -----------------------------------------------
.PHONY: all envs ruby doc

envs:
	pacman -S --noconfirm --needed --quiet --noprogressbar $(pkgs_msys2)
	mkdir -p $(path_download) $(path_vendors)

ruby : $(lib_ruby)

$(zip_ruby_static) :
	mkdir -p $(path_download)
	wget -O $(zip_ruby_static) $(url_ruby_static)
	touch $(zip_ruby_static)

$(lib_ruby_static) : $(zip_ruby_static)
	rm -rf $(path_ruby_static)
	tar -C $(path_packages)/ -xzf $(zip_ruby_static)
	cd $(path_ruby_static)/ && ./configure $(configure_ruby)
	cd $(path_ruby_static)/ && make -j4
	touch $(lib_ruby_static)

$(lib_ruby) : $(lib_ruby_static)
	mkdir -p $(lib_ruby) $(lib_ruby)/include $(lib_ruby)/lib $(lib_ruby)/script
	cp -r $(path_ruby_static)/include/ruby* $(lib_ruby)/include
ifeq ($(system), MINGW64_NT)
	cp -r $(path_ruby_static)/.ext/include/x64-mingw-ucrt/ruby/config.h $(lib_ruby)/include/ruby/config.h
else
	cp -r $(path_ruby_static)/.ext/include/i386-mingw32/ruby/config.h $(lib_ruby)/include/ruby/config.h
endif
	cp -r $(path_ruby_static)/*.a $(path_ruby_static)/ext/*/*.a $(lib_ruby)/lib
	cp -r $(path_ruby_static)/ext/fiddle/lib/* $(lib_ruby)/script
	sed -i -r "s/^require '.+'/\# \0/g" $(lib_ruby)/script/*.rb
	sed -i -r "s/^require '.+'/\# \0/g" $(lib_ruby)/script/*/*.rb
	cp -r $(lib_ruby)/script/* ./src/script

misc.7z :
	7z a $@ $(miscs)

doc : manual.md
	curl https://api.csswg.org/bikeshed/ -F file=@manual.md -F force=1 > manual.html