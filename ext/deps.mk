# -----------------------------------------------
# packages settings
# -----------------------------------------------
include ext/settings.txt
GITHUB_URL ?= https://github.com

CENTURION_VERSION=7.2.0
CONCURRENTQUEUE_VERSION=1.0.3
READERWRITERQUEUE_VERSION=1.0.6

path_packages := ./packages
path_download := $(path_packages)/download

system := $(shell uname | cut -d'-' -f1)
ifeq ($(system), MINGW64_NT)
	path_vendors := $(subst \,/,$(shell cygpath -aw ./))/vendors
else
	path_vendors := $(subst \,/,$(shell cygpath -aw ./))/vendors_mingw32
endif

# -----------------------------------------------
# centurion library
# -----------------------------------------------
url_centurion := $(GITHUB_URL)/albin-johansson/centurion/archive/refs/tags/v.$(CENTURION_VERSION).zip
path_centurion := $(path_download)/centurion_v$(CENTURION_VERSION).zip
lib_centurion := $(path_vendors)/include/centurion

# -----------------------------------------------
# concurrentqueue and readerwriterqueue library
# -----------------------------------------------
url_concurrentqueue := $(GITHUB_URL)/cameron314/concurrentqueue/archive/refs/tags/v$(CONCURRENTQUEUE_VERSION).zip
path_concurrentqueue := $(path_download)/concurrentqueue_v$(CONCURRENTQUEUE_VERSION).zip
url_readerwriterqueue := $(GITHUB_URL)/cameron314/readerwriterqueue/archive/refs/tags/v$(READERWRITERQUEUE_VERSION).zip
path_readerwriterqueue := $(path_download)/readerwriterqueue_v$(READERWRITERQUEUE_VERSION).zip
lib_queue := $(path_vendors)/include/cameron314

# -----------------------------------------------
# incbin library
# -----------------------------------------------
url_incbin := $(GITHUB_URL)/graphitemaster/incbin/archive/refs/heads/main.zip
path_incbin := $(path_download)/incbin-main.zip
lib_incbin := $(path_vendors)/include/incbin

# -----------------------------------------------
# xorstr library
# -----------------------------------------------
url_xorstr := $(GITHUB_URL)/JustasMasiulis/xorstr/archive/refs/heads/master.zip
path_xorstr := $(path_download)/xorstr-master.zip
lib_xorstr := $(path_vendors)/include/xorstr

# -----------------------------------------------
# ruby static library
# -----------------------------------------------
RUBY_VERSION := $(RUBY_MAJOR_VERSION).$(RUBY_MINOR_VERSION).$(RUBY_PATCH_VERSION)
RUBY_LIBRARY_VERSION := $(RUBY_MAJOR_VERSION)$(RUBY_MINOR_VERSION)0

url_ruby_static := https://cache.ruby-lang.org/pub/ruby/$(RUBY_MAJOR_VERSION).$(RUBY_MINOR_VERSION)/ruby-$(RUBY_VERSION).tar.gz
zip_ruby_static := $(path_download)/ruby-$(RUBY_VERSION).tar.gz
path_ruby_static := $(path_packages)/ruby-$(RUBY_VERSION)

ifeq ($(system), MINGW64_NT)
	lib_ruby_static := $(path_ruby_static)/libx64-ucrt-ruby$(RUBY_LIBRARY_VERSION)-static.a
else
	lib_ruby_static := $(path_ruby_static)/libmsvcrt-ruby$(RUBY_LIBRARY_VERSION)-static.a
endif

lib_ruby := $(path_vendors)/lib/ruby-$(RUBY_VERSION)
configure_ruby = --disable-install-doc --disable-rubygems --with-static-linked-ext --with-out-ext=openssl --without-gmp

# -----------------------------------------------
# msys2 packages (including SDL2 library)
# -----------------------------------------------
pkgs = SDL2 SDL2_ttf SDL2_image SDL2_mixer SDL2_gfx SDL2_net zlib libzip
pkgs_msys2 = $(addprefix $(MINGW_PACKAGE_PREFIX)-,$(pkgs))
pkgs_msys2 += bison libffi gmp upx p7zip vim unzip

# -----------------------------------------------
# tasks
# -----------------------------------------------
.PHONY: all envs

all: envs centurion queue incbin xorstr ruby

envs:
	pacman -S --noconfirm --needed --quiet $(pkgs_msys2)
	mkdir -p $(path_download) $(path_vendors)

centurion : $(lib_centurion)

queue : $(lib_queue)

incbin : $(lib_incbin)

xorstr : $(lib_xorstr)

ruby : $(lib_ruby)

$(path_centurion) :
	wget -O $(path_centurion) $(url_centurion)

$(lib_centurion) : $(path_centurion)
	mkdir -p $(lib_centurion)
	unzip -o -q $(path_centurion) -d $(path_packages)
	cp -r $(path_packages)/centurion-v.$(CENTURION_VERSION)/src/* $(path_vendors)/include
	touch $(lib_centurion)

$(path_concurrentqueue) :
	wget -O $(path_concurrentqueue) $(url_concurrentqueue)

$(path_readerwriterqueue) :
	wget -O $(path_readerwriterqueue) $(url_readerwriterqueue)

$(lib_queue) : $(path_concurrentqueue) $(path_readerwriterqueue)
	mkdir -p $(lib_queue)	
	unzip -o -q $(path_concurrentqueue) -d $(path_packages)
	cp $(path_packages)/concurrentqueue-$(CONCURRENTQUEUE_VERSION)/*.h $(lib_queue)	
	unzip -o -q $(path_readerwriterqueue) -d $(path_packages)
	cp $(path_packages)/readerwriterqueue-$(READERWRITERQUEUE_VERSION)/*.h $(lib_queue)
	touch $(lib_queue)

$(path_incbin) :
	wget -O $(path_incbin) $(url_incbin)

$(lib_incbin) : $(path_incbin)
	mkdir -p $(lib_incbin)	
	unzip -o -q $(path_incbin) -d $(path_packages)
	cp -r $(path_packages)/incbin-main/*.h $(lib_incbin)
	touch $(lib_incbin)

$(path_xorstr) :
	wget -O $(path_xorstr) $(url_xorstr)

$(lib_xorstr) : $(path_xorstr)
	mkdir -p $(lib_xorstr)
	unzip -o -q $(path_xorstr) -d $(path_packages)
	cp -r $(path_packages)/xorstr-master/include/*.hpp $(lib_xorstr)
	touch $(lib_xorstr)

$(zip_ruby_static) :
	mkdir -p $(path_download)
	wget -O $(zip_ruby_static) $(url_ruby_static)
	touch $(zip_ruby_static)

$(lib_ruby_static) : $(zip_ruby_static)
	rm -rf $(path_ruby_static)
	tar -C $(path_packages)/ -xzf $(zip_ruby_static)
	cd $(path_ruby_static)/ && ./configure $(configure_ruby)
	cd $(path_ruby_static)/ && make -j8
	touch $(lib_ruby_static)

$(lib_ruby) : $(lib_ruby_static)
	mkdir -p $(lib_ruby)
	mkdir -p $(path_vendors)/include/ruby
	cp -r $(path_ruby_static)/include/ruby/* $(path_vendors)/include/ruby
	cp -r $(path_ruby_static)/include/ruby.h $(path_vendors)/include/ruby.h
ifeq ($(system), MINGW64_NT)
	cp -r $(path_ruby_static)/.ext/include/x64-mingw-ucrt/* $(path_vendors)/include
else
	cp -r $(path_ruby_static)/.ext/include/i386-mingw32/* $(path_vendors)/include
endif
	cp -r $(path_ruby_static)/*.a $(lib_ruby)
	cp -r $(path_ruby_static)/ext/*/*.a $(lib_ruby)
	touch $(lib_ruby)
