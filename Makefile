# -----------------------------------------------
# packages settings
# -----------------------------------------------
include ./ext/settings.txt

# -----------------------------------------------
# UUID and PASSWORDS
# -----------------------------------------------
UUID ?= $(shell uuidgen | cut -c-32)
PASSWORD := $(shell uuidgen | sha256sum | cut -d' ' -f1)
RGM_VERSION ?= $(shell git tag --list | tail -1)*
RGM_FULLVERSION = $(RGM_VERSION) ($(shell date +%F) revision $(shell find ./src -type f | sort | xargs -n 1 cat | md5sum | cut -c-10))

system := $(shell uname | cut -d'-' -f1)
ifeq ($(system), MINGW64_NT)
	path_vendors := $(subst \,/,$(shell cygpath -aw ./))/vendors
else
	path_vendors := $(subst \,/,$(shell cygpath -aw ./))/vendors_mingw32
endif

path_script := ./src/script
deps := ./ext/deps.mk
Script := ./src/script ./src/config.ini
Data ?= ./Project1/Data

slient = 1>/dev/null
zip_embeded = ./embeded.zip
zip_publish = ./publish_$(RGM_VERSION).zip
zip_temp_add := 7z a -tzip -mx9 -p'$(PASSWORD)' $(zip_embeded) $(slient)
zip_publish_add := 7z a -tzip $(zip_publish) $(slient)
# -----------------------------------------------
# include and link path
# -----------------------------------------------
path_include = $(MSYSTEM_PREFIX)/include
path_lib = $(MSYSTEM_PREFIX)/lib
path_include += $(path_vendors)/include ./src
path_lib += $(path_vendors)/lib

cc = $(MSYSTEM_PREFIX)/bin/g++
cflags = -std=c++20 -pipe -fstack-protector-strong -MMD -MP -static -Wall -Wextra
ifeq ($(system), MINGW64_NT)
	cflags +=
else
	cflags += -m32 -mfma
endif

cflags += -DRGM_FULLVERSION="\"$(RGM_FULLVERSION)\""
cflags += -DCC_VERSION="\"$(shell $(cc) --version | head -n1)\""
cflags += -DRGM_SHADER_OPENGL

# build mode
# 0 = debug    -> debug.exe
# 1 = develop  -> main.exe
# 2 = standard -> Game.exe
# 3 = encrypt  -> Gamew.exe
cflags_debug    = -DRGM_BUILDMODE=0 -g -DDEBUG
cflags_develop  = -DRGM_BUILDMODE=1 -s -O3
cflags_standard = -DRGM_BUILDMODE=2 -s -O3 -DPASSWORD="\"$(PASSWORD)\""
cflags_encrypt  = -DRGM_BUILDMODE=3 -s -O3 -DPASSWORD="\"$(PASSWORD)\"" -mwindows

clibs = icon.o
libs = pthread zip uuid

# -----------------------------------------------
# ruby static library
# -----------------------------------------------
RUBY_VERSION := $(RUBY_MAJOR_VERSION).$(RUBY_MINOR_VERSION).$(RUBY_PATCH_VERSION)
RUBY_LIBRARY_VERSION := $(RUBY_MAJOR_VERSION)$(RUBY_MINOR_VERSION)0

lib_ruby := $(path_vendors)/lib/ruby-$(RUBY_VERSION)
lib_ruby_ext := $(lib_ruby)/ext
path_lib += $(lib_ruby)

ifeq ($(system), MINGW64_NT)
	libs += x64-ucrt-ruby$(RUBY_LIBRARY_VERSION)-static
else
	libs += msvcrt-ruby$(RUBY_LIBRARY_VERSION)-static
endif
libs += gmp stdc++ shell32 ws2_32 iphlpapi imagehlp shlwapi bcrypt
# -----------------------------------------------
# SDL2 static library
# -----------------------------------------------
path_include += $(MSYSTEM_PREFIX)/include/SDL2
libs += mingw32 SDL2main
libs += SDL2 dinput8 shell32 setupapi advapi32 uuid version oleaut32 ole32 imm32 winmm gdi32 user32 m
libs += SDL2_ttf harfbuzz freetype bz2 brotlidec png z graphite2 intl gdi32 rpcrt4 brotlicommon dwrite usp10
libs += SDL2_image png z tiff webp jpeg jbig lzma deflate zstd lerc jxl hwy
libs += SDL2_mixer flac mpg123 vorbisfile opusfile winmm ogg vorbis opus opusfile ogg shlwapi

# -----------------------------------------------
# ruby ext
# -----------------------------------------------
libs += :fiddle.a :zlib.a
libs += ffi

# -----------------------------------------------
# set cflags
# -----------------------------------------------
cflags += $(addprefix -I,$(path_include))
cflags += $(addprefix -L,$(path_lib))
clibs += $(addprefix -l,$(libs))

# -----------------------------------------------
# tasks
# -----------------------------------------------
-include main.d debug.d Game.d Gamew.d

.PHONY : all deps clean publish

all : main.exe

main.exe : ./src/main.cpp Makefile icon.o
	@echo "compile $@"
	@time $(cc) $< -o $@ $(cflags) $(clibs) $(cflags_develop)

debug.exe : ./src/main.cpp Makefile icon.o
	@echo "compile $@"
	@$(cc) $< -o $@ $(cflags) $(clibs) $(cflags_debug)

Game.exe : ./src/main.cpp Makefile icon.o
	@echo "pack $(zip_embeded)"
	@rm -f $(zip_embeded)
	@$(zip_temp_add) $(Script)
	@echo "compile $@"
	@$(cc) $< -o $@ $(cflags) $(clibs) $(cflags_standard)

Gamew.exe : ./src/main.cpp Makefile icon.o
	@echo "pack $(zip_embeded)"
	@rm -f $(zip_embeded)
	@$(zip_temp_add) $(Data)
	@$(zip_temp_add) $(Script)
	@echo "compile $@"
	@$(cc) $< -o $@ $(cflags) $(clibs) $(cflags_encrypt)
	@echo "compress $@"
	@upx -q $@ $(slient)

icon.o : ext/icon.rc ext/favicon.ico
	@echo "make icon"
	@windres -i $< -o $@ --input-format=rc --output-format=coff

clean :
	@rm -f main.d debug.d Game.d Gamew.d
	@rm -f main.exe debug.exe Game.exe Gamew.exe
	@rm -f *.log *.png
	@rm -f embeded.zip config.ini icon.o

envs : $(deps)
	@make envs -f $(deps)

deps : $(deps)
	@make -f $(deps)

publish : Game.exe main.exe
	@echo "pack $(zip_publish)"
	@rm -f $(zip_publish)
	@$(zip_publish_add) Game.exe main.exe src Project1

misc.7z :
	# @7z a $@ history Project1/Audio Project1/Graphics Project1/RGSS103J.dll Project1/RGSS104E.dll Project1/RPGXP.chm Project1/RPGXP.chw resource vendors/lib Doxyfile
	wget https://7niu.gxmatmars.com/p1/RGModern/misc.7z
	
misc : misc.7z
	@7z x -y $^