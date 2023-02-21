# Modern Ruby Game Engine (RGModern)

当前版本：v0.9.0

在线文档：[RGModern使用指南](https://docs.qq.com/doc/DUklCTWNvdmVEdVhY)

如果不方便使用Issue，可以在此文档反馈需求和缺陷：[RGModern用户反馈](https://docs.qq.com/doc/DUmJoemN5TXN5a0dE)

## 简介
RGModern 是极具现代化特色的 RMXP 新 runtime，主要使用 C++20 和 Ruby 3 编写。RGModern 特点：
1. 使用 SDL2 作为底层，支持 Direct3D 和 OpenGL 绘制。
2. 驱动多线程完成脚本逻辑、画面渲染等任务。
3. 借助 C++ 模板元编程，RGModern 能自由组合各种功能模块，并使用静态多态技巧提升执行效率。

## 编译
参见在线文档。

## 设计
如 `main.cpp` 所示，引擎本身由多个 worker 组合而成，每个 worker 在不同的线程执行不同的任务。详细设计参见 `src/` 内文档。

## 依赖
本工程主要使用了以下第三方库：
1. [ruby](https://github.com/ruby/ruby)
2. [SDL2](https://github.com/libsdl-org/SDL)
3. [centurion](https://github.com/albin-johansson/centurion)
4. [concurrentqueue](https://github.com/cameron314/concurrentqueue)
5. [readerwriterqueue](https://github.com/cameron314/readerwriterqueue)
6. [incbin](https://github.com/graphitemaster/incbin)
7. [xorstr](https://github.com/JustasMasiulis/xorstr)
8. [libzip](https://libzip.org)

本工程受到了其他 RPG Maker 复刻项目的启发：
1. [RGD](https://cirno.blog/archives/290)
2. [RGU](https://rpg.blue/thread-486473-1-1.html)
3. [Lanziss](https://rpg.blue/thread-480426-1-1.html)
4. [RGA](https://rpg.blue/thread-484466-1-1.html)
5. [LiteRGSS2](https://gitlab.com/pokemonsdk/litergss2)
6. [mkxp](https://github.com/Ancurio/mkxp)

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

## 小提示
创建d3d11 Shader：`fxc /O3 /T ps_5_0 src\shader\direct3d11\gray.hlsl /Fh src\shader\direct3d11\gray.h /Vn rgm_shader_gray_data`，然后将 `const BYTE` 改成 `extern constexpr unsigned char`。

对于d3d9，使用ps_2_a：
```bash
fxc /O3 /T ps_2_a src\shader\direct3d11\gray.hlsl /Fh src\shader\direct3d9\gray.h /Vn rgm_shader_gray_dx9_data
```