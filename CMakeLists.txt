cmake_minimum_required(VERSION 3.25)
project(RGModern)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# 读取 RGM 版本号
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ext/settings.txt")
    file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/ext/settings.txt" RGM_VERSION_LINE REGEX "^RGM_VERSION=([0-9]+\.*)+$")
    string(REGEX REPLACE "^RGM_VERSION=(([0-9]+\.*)+)$" "\\1" RGM_VERSION_STRING "${RGM_VERSION_LINE}")
    unset(RGM_VERSION_LINE)
endif()
find_package(Git)
if(Git_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short=10 HEAD
        OUTPUT_VARIABLE RGM_REVISION_STRING
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()
string(TIMESTAMP RGM_REVISION_DATE "%Y-%m-%d")
set(RGM_FULLVERSION "${RGM_VERSION_STRING} (${RGM_REVISION_DATE} revision ${RGM_REVISION_STRING})")

# 目录连接
set(SRC_DIR "src")
file(CREATE_LINK
    "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/${SRC_DIR}"
    SYMBOLIC
)
# RGM
add_executable(Game)
aux_source_directory(${SRC_DIR} SRC_LIST)
target_sources(Game PRIVATE
    ${SRC_LIST} # 源代码
    ext/icon.rc # 资源文件
)
enable_language(RC)
set_property(SOURCE ext/icon.rc
    PROPERTY LANGUAGE RC    # 设置资源文件属性
)
set_property(TARGET Game
    PROPERTY 
    CXX_STANDARD 20         # 使用C++20
)
target_compile_options(Game PRIVATE
    "$<$<CXX_COMPILER_ID:MSVC>:-utf-8>"        # MSVC需要显式指定源文件使用utf8
    "$<$<CXX_COMPILER_ID:GNU>:-Wa,-mbig-obj>"  # 用的模板太多+header only=报错
    "$<$<CXX_COMPILER_ID:MSVC>:-bigobj>"       # 同上
)
target_compile_definitions(Game PRIVATE
    "NOMINMAX" # 使std::max和std::min生效
    "RGM_FULLVERSION=\"${RGM_FULLVERSION}\""
    "CC_VERSION=\"${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}\""
    "$<$<STREQUAL:${RGM_RENDER_BACKEND},D3D11>:RGM_USE_D3D11>"
    "$<$<STREQUAL:${RGM_RENDER_BACKEND},OPENGL>:RGM_USE_OPENGL>"
)
if(WIN32)
    # 静态链接声明所需宏
    target_compile_definitions(Game PRIVATE
        "RUBY_EXPORT"
        "ZIP_STATIC"
    )
endif()
# 搜索 ruby 标准库路径
if(Ruby_EXT_LIBRARY_DIR)
    function(_RUBY_FIND_EXT OUTVAR RBVAR)
        unset(Ruby_EXT_LIBRARY)
        find_library(Ruby_EXT_LIBRARY
            NAMES ${RBVAR}
            PATHS $ENV{Ruby_EXT_LIBRARY_DIR} ${Ruby_EXT_LIBRARY_DIR}
            PATH_SUFFIXES lib
            NO_CACHE
            REQUIRED
        )
        set(${OUTVAR} ${Ruby_EXT_LIBRARY} PARENT_SCOPE)
    endfunction()

    # Ruby fiddle
    unset(Ruby_EXT_FIDDLE_LIBRARIES)
    _RUBY_FIND_EXT(Ruby_EXT_FIDDLE_LIBRARIES "fiddle;fiddle.a")
    target_link_libraries(Game ${Ruby_EXT_FIDDLE_LIBRARIES})
    # Ruby zlib
    unset(Ruby_EXT_ZLIB_LIBRARIES)
    _RUBY_FIND_EXT(Ruby_EXT_ZLIB_LIBRARIES "zlib;zlib.a")
    target_link_libraries(Game ${Ruby_EXT_ZLIB_LIBRARIES})
else()
    # message(SEND_ERROR "Could NOT find Ruby EXT: ${_Ruby_EXT_LIBRARIES_NAME} (missing Ruby_EXT_LIBRARY_DIR)")
endif()
# 搜索基础三方库路径
if(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.lib")
endif()
find_package(Ruby REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_net REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(LibZip REQUIRED)
find_package(libffi REQUIRED)
set(CENTURION_INCLUDE_DIRS "./third_party/centurion/src")
set(CONCURRENTQUEUE_INCLUDE_DIRS "./third_party/concurrentqueue")
set(INCBIN_INCLUDE_DIRS "./third_party/incbin")
set(READERWRITERQUEUE_INCLUDE_DIRS "./third_party/readerwriterqueue")
set(XORSTR_INCLUDE_DIRS "./third_party/xorstr/include")
# INCLUDE
target_include_directories(Game PRIVATE
    ${SRC_DIR}
    ${Ruby_INCLUDE_DIRS}
    ${Ruby_CONFIG_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIR}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
    ${SDL2_NET_INCLUDE_DIRS}
    ${LIBZIP_INCLUDE_DIR}
    ${CENTURION_INCLUDE_DIRS}
    ${CONCURRENTQUEUE_INCLUDE_DIRS}
    ${INCBIN_INCLUDE_DIRS}
    ${READERWRITERQUEUE_INCLUDE_DIRS}
    ${XORSTR_INCLUDE_DIRS}
)
# LIB
target_link_libraries(Game
    ${Ruby_LIBRARIES}
    SDL2::Core
    SDL2::Main
    SDL2::Image
    SDL2::TTF
    SDL2::Mixer
    SDL2::Net
    OpenGL::GL
    ZLIB::ZLIB
    ${LIBZIP_LIBRARY}
    libffi::libffi
    "$<$<BOOL:${MINGW}>:mingw32>"
)

if(WIN32)
    # windows 静态链接特有选项
    if(MINGW)
        target_link_libraries(Game
            "m;harfbuzz;freetype;bz2;brotlidec;graphite2;intl;brotlicommon;"
            "png;tiff;webp;jpeg;jbig;lzma;deflate;zstd;lerc;jxl;hwy;sharpyuv"
            "flac;mpg123;vorbisfile;opusfile;ogg;vorbis;opus"
            "d3dx9"
        )
    endif()
    target_link_libraries(Game
        "d3d9;d3d11"
        "dinput8;dwrite;gdi32"
        "shell32;ws2_32;iphlpapi;imagehlp;shlwapi;bcrypt"
        "setupapi;advapi32;uuid;version;oleaut32;ole32;imm32;winmm;user32;rpcrt4;usp10"
    )
endif()

# 内嵌包
set(ZIP_EMBEDED "embeded")
add_custom_target(${ZIP_EMBEDED}
    BYPRODUCTS ${ZIP_EMBEDED}.zip
    COMMAND ${CMAKE_COMMAND} -E tar cf ${CMAKE_CURRENT_BINARY_DIR}/${ZIP_EMBEDED}.zip --format=zip
    VERBATIM
)
add_dependencies(Game ${ZIP_EMBEDED})
if(RGM_BUILDMODE)
    target_compile_definitions(Game PRIVATE
        "RGM_BUILDMODE=${RGM_BUILDMODE}"
    )
    find_program(7ZIP_EXECUTABLE
        NAMES "7z" "7za" "7zz"
        HINTS $ENV{7ZIP_HOME} ${7ZIP_HOME}
        PATH_SUFFIXES bin
        REQUIRED
    )
    string(RANDOM LENGTH 64 _PASSWORD256) # 生成密码
    if(${RGM_BUILDMODE} GREATER_EQUAL "2")
        # 打包 RGM 脚本
        add_custom_command(TARGET ${ZIP_EMBEDED}
            POST_BUILD
            BYPRODUCTS ${ZIP_EMBEDED}.zip
            COMMAND ${7ZIP_EXECUTABLE} a -tzip -mx9 -p${_PASSWORD256} ${CMAKE_CURRENT_BINARY_DIR}/${ZIP_EMBEDED}.zip ${CMAKE_CURRENT_SOURCE_DIR}/src/script ${CMAKE_CURRENT_SOURCE_DIR}/src/config.ini
            VERBATIM
        )
    endif()
    if(${RGM_BUILDMODE} GREATER_EQUAL "3")
        # 打包工程脚本
        add_custom_command(TARGET ${ZIP_EMBEDED}
            POST_BUILD
            BYPRODUCTS ${ZIP_EMBEDED}.zip
            COMMAND ${7ZIP_EXECUTABLE} a -tzip -mx9 -p${_PASSWORD256} ${CMAKE_CURRENT_BINARY_DIR}/${ZIP_EMBEDED}.zip ${CMAKE_CURRENT_SOURCE_DIR}/Project1/Data
            VERBATIM
        )
        # 设置入口函数
        set_target_properties(Game PROPERTIES WIN32_EXECUTABLE true)
    endif()
    target_compile_definitions(Game PRIVATE
        "PASSWORD=\"${_PASSWORD256}\""
    )
    unset(_PASSWORD256 CACHE)
else()
    target_compile_definitions(Game PRIVATE
        "RGM_BUILDMODE=$<IF:$<CONFIG:Debug>,0,1>"
    )
endif()

if(MSVC)
    # MSVC 的 INCBIN 支持
    file(GLOB_RECURSE INCBIN_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/*.hpp)
    # 构建工具
    add_executable(incbin
        "${INCBIN_INCLUDE_DIRS}/incbin.c"
    )
    target_include_directories(incbin PRIVATE
        ${INCBIN_INCLUDE_DIRS}
    )
    # 执行工具
    set(_INCBIN_OUTPUT data.c)
    add_custom_command(TARGET incbin
        POST_BUILD
        BYPRODUCTS ${_INCBIN_OUTPUT}
        COMMAND $<TARGET_FILE:incbin> -Ssnakecase -p rgm_ ${INCBIN_SRCS} -o ${CMAKE_CURRENT_BINARY_DIR}/${_INCBIN_OUTPUT}
        VERBATIM
    )
    add_dependencies(incbin ${ZIP_EMBEDED})
    add_dependencies(Game incbin)
    target_sources(Game PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/${_INCBIN_OUTPUT}"
    )
endif()
