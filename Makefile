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

path_thirdparty := \
	./third_party/centurion/src \
	./third_party/concurrentqueue \
	./third_party/fiber \
	./third_party/incbin \
	./third_party/readerwriterqueue \
	./third_party/ruby$(RUBY_M_VERSION)/include \
	./third_party/xorstr/include

path_script := ./src/script
deps := ./ext/deps.mk
Script := ./src/script ./src/config.ini
Data ?= ./Project1/Data
libgch := ./src/lib/lib.hpp.gch
slient := 1>/dev/null
zip_embeded := ./embeded.zip
zip_publish := ./publish_v$(RGM_VERSION).zip
zip_temp_add := 7z a -tzip -mx9 -p'$(PASSWORD)' $(zip_embeded) $(slient)
zip_publish_add := 7z a -tzip $(zip_publish) $(slient)
targets = main main_win7 Game Game_win7 Gamew Gamew_win7
# -----------------------------------------------
# include and link path
# -----------------------------------------------
path_include = $(MSYSTEM_PREFIX)/include
path_lib = $(MSYSTEM_PREFIX)/lib
path_include += $(path_thirdparty) ./src

cc = $(MSYSTEM_PREFIX)/bin/g++
cflags = -std=c++20 -pipe -fstack-protector-strong -MMD -MP -static -Wall -Wextra
ifeq ($(system), MINGW64_NT)
	cflags +=
else
	cflags += -m32 -mfma
endif

cflags += -DRGM_FULLVERSION="\"$(RGM_FULLVERSION)\""
cflags += -DCC_VERSION="\"$(shell $(cc) --version | head -n1)\""
cflags += -DRUBY_EXPORT -DZIP_STATIC

# build mode
# 0 = debug    -> debug.exe
# 1 = develop  -> main.exe
# 2 = standard -> Game.exe
# 3 = encrypt  -> Gamew.exe
cflags_debug    = -DRGM_BUILDMODE=0 -Og -g -DDEBUG
cflags_develop  = -DRGM_BUILDMODE=1 -s -O3
cflags_standard = -DRGM_BUILDMODE=2 -s -O3 -DPASSWORD="\"$(PASSWORD)\""
cflags_encrypt  = -DRGM_BUILDMODE=3 -s -O3 -DPASSWORD="\"$(PASSWORD)\"" -mwindows

clibs = icon.o
libs = pthread stdc++
libs_dynamic =
# -----------------------------------------------
# ruby static library
# -----------------------------------------------
path_lib += ./third_party/ruby$(RUBY_M_VERSION)/lib
ifeq ($(system), MINGW64_NT)
	libs += x64-ucrt-ruby$(RUBY_M_VERSION)0-static
else
	libs += msvcrt-ruby$(RUBY_M_VERSION)0-static
endif
libs += gmp zip uuid shell32 ws2_32 iphlpapi imagehlp bcrypt

# -----------------------------------------------
# SDL2 static library
# -----------------------------------------------
path_include += $(MSYSTEM_PREFIX)/include/SDL2
libs_dynamic += mingw32 SDL2main SDL2 SDL2_ttf SDL2_image SDL2_mixer
libs += dinput8 setupapi advapi32 version oleaut32 ole32 imm32 winmm user32 m
libs += harfbuzz freetype bz2 brotlidec png z graphite2 intl rpcrt4 brotlicommon dwrite usp10
libs += tiff webp jpeg jbig lzma deflate zstd lerc jxl hwy sharpyuv
libs += flac mpg123 vorbisfile opusfile ogg vorbis opus gdi32 shlwapi

# -----------------------------------------------
# opengl
# -----------------------------------------------
libs += opengl32 d3dx9

# -----------------------------------------------
# ruby static extension
# -----------------------------------------------
libs += :fiddle.a :zlib.a
libs += ffi

# -----------------------------------------------
# set cflags
# -----------------------------------------------
cflags += $(addprefix -I,$(path_include))
cflags += $(addprefix -L,$(path_lib))

clibs_dynamic = $(clibs) -static -Wl,-Bdynamic $(addprefix -l,$(libs_dynamic)) -Wl,-Bstatic $(addprefix -l,$(libs))
clibs_static = $(clibs) $(addprefix -l,$(libs_dynamic)) $(addprefix -l,$(libs))
# -----------------------------------------------
# tasks
# -----------------------------------------------
-include main.d debug.d Game.d Gamew.d

.PHONY : clean publish misc envs headers

.DEFAULT_GOAL := main.exe

$(libgch) : ./src/lib/lib.hpp
	@echo "compile $@"
	@g++ -x c++-header -c $< $(cflags) $(cflags_develop)

icon.o : ext/icon.rc ext/favicon.ico
	@echo "make icon"
	@windres -i $< -o $@ --input-format=rc --output-format=coff

main.o : ./src/main.cpp Makefile $(libgch)
	@echo "compile $@"
	@time $(cc) $< -o $@ -c $(cflags) $(cflags_develop)

Game.o : ./src/main.cpp Makefile $(libgch)
	@echo "pack $(zip_embeded)"
	@rm -f $(zip_embeded)
	@$(zip_temp_add) $(Script)
	@echo "compile $@"
	@$(cc) $< -o $@ -c $(cflags) $(cflags_standard)

Gamew.o : ./src/main.cpp Makefile $(libgch)
	@echo "pack $(zip_embeded)"
	@rm -f $(zip_embeded)
	@$(zip_temp_add) $(Data)
	@$(zip_temp_add) $(Script)
	@echo "compile $@"
	@$(cc) $< -o $@ -c $(cflags) $(cflags_encrypt)

debug.exe : ./src/main.cpp Makefile icon.o
	@echo "compile $@"
	@rm -f $(libgch)
	@$(cc) $< -o $@ $(cflags) $(clibs_static) $(cflags_debug)

main.exe : main.o icon.o
	@echo "link $@"
	@$(cc) $< -o $@ $(cflags) $(cflags_develop) $(clibs_static)

main_win7.exe : main.o icon.o
	@echo "link $@"
	@$(cc) $< -o $@ $(cflags) $(cflags_develop) $(clibs_dynamic)

Game.exe : Game.o icon.o
	@echo "link $@"
	@$(cc) $< -o $@ $(cflags) $(cflags_standard) $(clibs_static)
	@cp $@ ./Project1/

Game_win7.exe : Game.o icon.o
	@echo "link $@"
	@$(cc) $< -o $@ $(cflags) $(cflags_standard) $(clibs_dynamic)
	@cp $@ ./Project1/

Gamew.exe : Gamew.o icon.o
	@echo "link $@"
	@$(cc) $< -o $@ $(cflags) $(cflags_encrypt) $(clibs_static)
	@echo "compress $@"
	@upx -q $@ $(slient)
	@cp $@ ./Project1/

Gamew_win7.exe : Gamew.o icon.o
	@echo "link $@"
	@$(cc) $< -o $@ $(cflags) $(cflags_encrypt) $(clibs_dynamic)
	@echo "compress $@"
	@upx -q $@ $(slient)
	@cp $@ ./Project1/

clean :
	@rm -f $(addsuffix .d,$(targets)) debug.d
	@rm -f $(addsuffix .o,$(targets)) debug.o
	@rm -f $(addsuffix .exe,$(targets)) debug.exe
	@rm -f *.log *.png
	@rm -f $(zip_embeded) $(libgch) lib.d
	@rm -f config.ini icon.o

publish : $(addsuffix .exe,$(targets))
	@echo "pack $(zip_publish)"
	@rm -f $(zip_publish)
	@cp ./src/config.ini ./Project1/
	@rm -f ./Projec1/error.log
	@rm -f $(libgch)
	@$(zip_publish_add) main.exe main_win7.exe SDL2*.dll src Project1

misc.7z :
	wget -q https://7niu.gxmatmars.com/p1/RGModern/misc.7z

misc : misc.7z
	@7z x -y $^

envs : $(deps)
	@make envs -f $(deps)

%.o : %.hpp
	@g++ -x c++-header -c -o /dev/null $< $(cflags)

headers :
	@find ./src/ -type f -name *.hpp | sed s/.hpp/.o/g | xargs -n 1 make 2> make.log