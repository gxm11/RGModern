include Makefile

custom_cflags := $(cflags) -s -O3
tmp_dir = /tmp/RGMBuild

ifdef CUSTOM_STATIC
custom_clibs := $(clibs_static)
else
custom_clibs := $(clibs_dynamic)
endif

ifdef CUSTOM_NOCONSOLE
custom_clibs += -mwindows
endif

ifdef CUSTOM_DATA
custom_cflags += -DRGM_BUILDMODE=3 -DPASSWORD="\"$(PASSWORD)\""
custom_encrypt_data = $(shell cygpath $(CUSTOM_DATA))
else
ifdef CUSTOM_STANDARD
custom_cflags += -DRGM_BUILDMODE=2 -DPASSWORD="\"$(PASSWORD)\""
else
custom_cflags += -DRGM_BUILDMODE=1
endif
endif

custom.exe : ./src/main.cpp icon.o
	rm -f $(zip_embeded)
ifdef CUSTOM_DATA
	$(zip_temp_add) $(Script)
	$(zip_temp_add) $(custom_encrypt_data)
else
ifdef CUSTOM_STANDARD
	$(zip_temp_add) $(Script)
endif
endif
	$(cc) $< -o $@ $(custom_cflags) $(custom_clibs)
ifdef CUSTOM_DATA
	upx -q $@ $(slient)
endif
	rm -rf $(tmp_dir)
	mkdir -p $(tmp_dir)
	cp $@ ./src/config.ini $(tmp_dir)
ifndef CUSTOM_STATIC
	cp ./Project1/SDL2*.dll ./Project1/lib*.dll $(tmp_dir)
endif
ifndef CUSTOM_DATA
ifndef CUSTOM_STANDARD
	cp -r ./src $(tmp_dir)/src
endif
endif