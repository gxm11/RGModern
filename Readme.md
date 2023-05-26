# Modern Ruby Game Engine (RGModern)
![GitHub](https://img.shields.io/github/license/gxm11/RGModern)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/gxm11/RGModern)
[![CodeFactor](https://www.codefactor.io/repository/github/gxm11/rgmodern/badge)](https://www.codefactor.io/repository/github/gxm11/rgmodern)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue)
![Ruby32](https://img.shields.io/badge/Ruby-3.2.2-red)
![SDL2](https://img.shields.io/badge/SDL-2.26.5-132b48)

![release build](https://github.com/gxm11/RGModern/actions/workflows/main.yml/badge.svg?event=release)
![push build](https://github.com/gxm11/RGModern/actions/workflows/main.yml/badge.svg?event=push)

当前版本：v1.0.2

在线文档：[RGModern使用指南](https://docs.qq.com/doc/DUklCTWNvdmVEdVhY)

## 简介
RGModern 是极具现代化特色的 RMXP 新 runtime，主要使用 C++20 和 Ruby 3 编写。

RGModern 特点：
1. 对 RGSS 功能的几乎 100% 覆盖，并提供了一些额外的实用功能；
2. 使用各种优化技巧提升执行效率，画面流畅，性能强劲；
3. 使用 SDL2 作为底层，支持 Direct3D9，Direct3D11 和 OpenGL 绘制；
4. 驱动多个 worker 完成脚本逻辑、画面渲染等任务。worker 以多线程或者协程的方式合作；
5. 借助 C++ 模板元编程，RGModern 能自由配置每个 worker 的功能模块，利于二次开发。

## 编译
游戏制作者请下载：[RGM小助手](https://7niu.gxmatmars.com/p1/RGModern/RGM-assistant.7z)，并按照以下步骤操作：
1. 更新编译环境 1 次；
2. 更新 RGModern 1 次；
3. 选择合适的编译、链接选项，开始编译；
4. 打开产物所在目录。

注意：
 - 第一次使用请更新 2 次编译环境并更新 2 次 RGModern；
 - 小助手自带的 RGModern 仓库安装在 `./msys64/opt/RGModern` 下；
 - 选择 Data 文件夹将制作加密包，选择后将强制使用标准模式。

二次开发者请查阅使用指南和 Makefile，或参考 Github Actions 的设置。

## 使用
在 Windows 上使用必须安装 [DirectX 最终用户运行时](https://www.microsoft.com/zh-CN/download/details.aspx?id=35)。

直接使用 Game.exe 代替原版的 Game.exe 即可。大多数脚本都会兼容，如果出现不兼容的情况或者其他运行时报错，请查看 error.log 尝试排查错误。

如果出现 RGModern Internal Error 或其他复杂问题，请提出 Issue。如果不方便使用 Issue，可以在此文档反馈需求和缺陷：[RGModern用户反馈](https://docs.qq.com/doc/DUmJoemN5TXN5a0dE)

如果要修改 RGModern 的功能或进行二次开发，可以用开发者模式编译的 main.exe，此 exe 需要读取当前路径下的 `src/script` 文件夹中的 ruby 脚本运行，可以修改这些脚本文件自定义对 RGSS 的实现。

## config.ini
RGModern 在运行时会读取 config.ini 中的配置信息。如果 config.ini 不存在，RGModern 会产生一份新的配置文件，但是可能部分配置项未能正确读取。建议分发游戏时始终携带 config.ini。

config.ini 中有以下几个 section：
1. Game，配置游戏标题和 RTP 路径；
2. System，配置游戏的显示（分辨率、全屏等）和音乐选项；
3. Keymap，配置键盘（或控制器）按键与游戏内的虚拟按键的映射关系；
4. Font，配置游戏中使用的字体名和字体文件路径的映射关系；
5. Kernel，配置渲染器、协作模式等高级选项。

## 加密
RGModern 支持加密图片素材。

首先需要制作加密包。使用 7-zip 软件打包 Graphics 文件夹并设置密码即可。注意，打包后的 zip 文件中需要包含 Graphics 目录。

在游戏开始加载图片素材前调用：
```ruby
Finder.regist("Graphics.zip", "password")
```
其中第一个参数是加密包的文件名，第二个参数是加密包的密码。此后就会优先从加密包中读取资源，如果加密包中没有资源，则会从游戏目录和 RTP 中寻找。

RGModern 不支持加密音乐素材。

RGModern 支持加密 Data 文件夹。如果使用 RGM 小助手编译，选择对应的 Data 文件夹即可。不使用小助手请参照 Makefile 中 Gamew.exe 的编译。

RGModern 不支持原版的加密方案。众所周知，原版加密方案等同于没有加密。RGModern 未来也不会支持原版的加密方案。

## 设计
如 `src/main.hpp` 所示，引擎本身是一个由多个 worker 组合而成的 scheduler，每个 worker 管理各自的数据并执行不同的任务，scheduler 负责 worker 之间的任务转发和运行调度。

具体运作原理请查看 `src` 内的 c++ 源码。RGModern 的代码使用 gcc12 在 `-Wall -Wextra -Werror` 的选项下编译通过，代码质量有保障，附带详细的 Doxygen 风格的注释，可放心查看。

## 依赖
本工程主要使用了以下第三方库：
1. [ruby](https://github.com/ruby/ruby)
2. [SDL2](https://github.com/libsdl-org/SDL)
3. [SDL2 Image](https://github.com/libsdl-org/SDL_image)
4. [SDL2 Mixer](https://github.com/libsdl-org/SDL_mixer)
5. [SDL2 TTF](https://github.com/libsdl-org/SDL_ttf)
6. [centurion](https://github.com/albin-johansson/centurion)
7. [concurrentqueue](https://github.com/cameron314/concurrentqueue)
8. [incbin](https://github.com/graphitemaster/incbin)
9. [xorstr](https://github.com/JustasMasiulis/xorstr)
10. [libzip](https://libzip.org)
11. [paladin-t/fiber](https://github.com/paladin-t/fiber)

本工程受到了其他 RPG Maker 复刻项目的启发：
1. [RGD](https://cirno.blog/archives/290)
2. [RGU](https://rpg.blue/thread-486473-1-1.html)
3. [Lanziss](https://rpg.blue/thread-480426-1-1.html)
4. [RGA](https://rpg.blue/thread-484466-1-1.html)
5. [LiteRGSS2](https://gitlab.com/pokemonsdk/litergss2)
6. [mkxp](https://github.com/Ancurio/mkxp)
7. [tapir](https://github.com/qnighy/tapir)

以及旧项目：
1. [sdlrgss](https://gitee.com/rmxp/sdlrgss)
2. [rgsos](https://gitlab.com/gxm/rgsos)

本工程还受到以下项目的启发：
1. [Rice](https://github.com/jasonroelofs/rice)
2. [StarEngine](https://zhuanlan.zhihu.com/p/83095672)
3. [Morden C++ 模板元编程](https://netcan.github.io/presentation/metaprogramming/#/)
4. [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
5. [sdlrenderer-hlsl](https://github.com/felipetavares/sdlrenderer-hlsl)
6. [SDL2 + OPENGL GLSL 实践](https://blog.csdn.net/qq_40369162/article/details/122641658)
7. [sdl2glsl](https://github.com/AugustoRuiz/sdl2glsl/)
8. [The Definitive Guide to Ruby's C API](https://silverhammermba.github.io/emberb/)
