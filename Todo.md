# RGModern todo list

## 战斗测试
战斗测试时，载入太快了，无渐变效果。但是在游戏内是有的。（FEATURE）

32位下render的stopwatch精度有问题，无法计时。（32位的兼容交给有需求的开发者）

## 发布前的TODO
shader_tone可以简化写法。（已经不知道为什么有这个TODO，疑似改好了）

考虑到创建drawable和texture的开销，应该在每个线程使用一个资源池：https://zhuanlan.zhihu.com/p/359409607。使用std::pmr下的unsynchronized_pool_resource
https://en.cppreference.com/w/cpp/memory/polymorphic_allocator
https://www.cppstories.com/2020/06/pmr-hacking.html/
https://zhuanlan.zhihu.com/p/96089089
https://wanghenshui.github.io/2019/04/28/allocator.html

考虑到进一步的兼容性，可以把SDL2和ruby都用动态链接，并且可以设置默认的渲染器。添加对opengl，d3d9的支持，增加相应的shader，对于software，禁用shader。（实际上动态链接就可以在win7上跑了，也支持动态链接加密数据，虽然意义不大）（opengl shader已经完成）

仔细检查32位系统的兼容性设置，可以将xorstr需要的AVXXXX宏，在makefile里实现，尝试linux下编译（假设已经安装了SDL和ruby，或者ruby可以静态编译）

添加load data和save data的第二个参数为密码，这样就会用加密的zip格式保存文件。由于zip格式和marshal格式的开头是不一样的，如果读取时发现是marshal格式，就无视密码的效果，做到向前兼容。（交给用户自行解决）

范例工程和文档。

添加鼠标、joystick等控制，但是这些应该位于rgm::ext下。包括Movie，加入视频播放功能。joystick优先，鼠标和Movie待定。

添加Window、Sprite的额外invisible条件：在viewport或者屏幕之外。从而减少绘制指令。（给window和viewport添加了，但是Sprite的Bitmap的宽和高的数据，保存在渲染线程，这里计划提供一个辅助函数RGM::Ext.sprite_in_screen?来处理）

将d3d和opengl的切换，以及是否使用异步模型的切换放到运行时判断，直接读取config.ini。如果文件不存在则使用默认。

transition，尝试移除alpha blend，而是改用默认的color模式。尽量移除其他的自定义blend mode。

提供software模式，此模式下所有的shader无效，且无法使用自定义的blend mode。需要修改add和sub的实现方式，可以不完全跟XP一样（参照screen和multiply）。只能说勉强可以玩，提高兼容性。

## opengl参考资料
[OpenGL笔记（二）Shader及纹理](https://zhuanlan.zhihu.com/p/447584535?utm_id=0)

[openGL着色器 (shader)](https://blog.csdn.net/xueangfu/article/details/117084647)

opengl可以不链接到glew，见 https://github.com/AugustoRuiz/sdl2glsl/blob/master/src/main.cpp

## 单元测试
音乐部分
1. BGM播放、切换、停止
2. ME播放、切换、停止
3. BGM播放，插播ME，切换BGM、停止
4. BGS播放、切换、停止、音调
5. SE播放、停止、音调
6. BGM+BGS+SE混合播放

画面部分
1. 画面、窗口分辨率修改
2. freeze测试，transition测试
3. Sprite测试
4. Viewport测试
5. Window测试
6. Plane测试
7. Tilemap测试
8. Animation测试
9. Weather测试

功能部分
1. 帧率显示、帧率修改
2. 读取config，切换按键
3. 输入法
4. 右上角关闭按钮和Alt+F4
5. embeded文件读取
6. 外部resource读取
7. snap_to_bitmap，save_png
8. Palette测试

## 核心内容修改
std::apply和std::tuple 参见：https://godbolt.org/z/8K3WrEzd4

高级参数包编程：https://www.scs.stanford.edu/~dm/blog/param-pack.html#recursing-over-argument-lists

基于consteval和tuple的编程：https://godbolt.org/z/nrMM79o7j