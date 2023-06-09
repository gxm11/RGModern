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


## opengl的渐变bug（已解决）
在第二张地图切换时，整个画面是白色的。看上去是因为整个地图都是白色的，opengl没有正常绘制地图上的内容。

目前发现第一张地图（室内）更换海边道路05的tileset后，偶发帧率20帧的场合，目前已经发现：
1. 渲染引擎使用opengl或者d3d11都有此现象
2. 同步/异步模式下都有此现象
3. 使用N卡/集显渲染都有此现象

初步怀疑是地图中有大量优先级为1的图块导致tilemap的性能问题。仍然使用店内36的tileset，只是把外面绘制成自动元件的海洋，似乎是没有此问题的。

从效率上看，正常绘制第一张地图应该只需要0.5ms的时间，卡顿的时候需要50ms。

极大可能是table越界导致的问题。以下代码可以暂时解决table越界的问题：
```c++
int16_t get(int x, int y, int z) const {
int index = x + y * x_size + z * x_size * y_size;

if (index < 0 || static_cast<size_t>(index) >= data.size()) return 0;

return data[index];
}

int16_t get(int index) const {
if (index < 0 || static_cast<size_t>(index) >= data.size()) return 0;

return data[index];
}
```
由于从ruby层调用是没有问题的。所以直接排查一下c++层里所有`table&`和`table*`的变量，凡是调用了get方法，都要判断一下范围。还有`tables&`和`tables*`的变量，也要排除。

直接搜一下table，cpp文件也就不到10个，可以逐一排查一遍。

此外，tilemap_info的setup方法，下面那个三层循环，似乎没有多次做的必要，看上去这个setup方法会因为zi的不同被频繁调用，见tilemap_manager。这里主要是为了设置x_cache和y_cache，加速后面的绘制。但是x_cache和y_cache的值与zi无关，可以考虑再弄一个表专门存cache？或者只在第一次调用时执行下面的三层循环。

白屏的问题已经定位到opengl的shader_tone上了。因为之前不管有没有tone，opengl都会执行一次shader_tone的变化。

现在看来resize window还是有点问题。建议用户尽量避免在运行时使用resize_window，而是修改了config后再重新启动游戏。resize跟全屏的2个模式，是否使用独显效果都有关。看来不是一个急需解决的问题，提醒用户注意即可。试一下把cache清理掉？看上去就是当前的bitmap被重绘了，直接重新加载所有的bitmap就行。

# 20230405-todo
- [x] 摇杆映射方向键写到config里
- [x] 增加摇杆的 rumble
- [x] 增加切换全屏的快捷键 alt+enter（放弃）
- [x] resize_window 增加第 4 种模式，画面直接无缩放并居中。
- [x] 简易设置的ruby的callback任务
  1. 在ruby层发起请求，ruby层的函数要自带一个proc，会获得一个id
  2. 实际发送2个异步任务，第一个是执行的内容，第二个是将第一个的结果包装成callback任务发送给ruby
  3. 数据用string包装
  4. 主动线程执行callback时，会根据id找到对应的proc，然后把string传给对应的函数
- [x] 支持超长的tileset
  1. 直接修改RPG::Cache读取tileset的方式。用palette读进来之后，修改高度为8192，然后把原图切片折叠绘制到新的texture上。
  2. 如果是8192，那么最高支持262,144的tileset。如果是16384，则最高支持1,048,576的tileset。
  3. 相应在渲染线程读取tileset时，也需要对超出高度的部分进行处理。
- [x] 战斗测试可能要先执行Graphics，渲染一次画面。
- [x] Base.sync的定义要改成只等单个线程，跟RGMWAIT一致。
- [x] config重写，使用map等高级结构保存string，然后一口气分析完。
  1. 调整config在load里的位置，或者把生成config放到c++部分。
  2. 总之尽量统一ruby和c++对config的处理。
- [x] Bitmap里的fill_rect和draw_text加上to_i，可能是给color/rect等builtin加上比较合适。
- [x] 渐变突然没了？修复渐变消失的问题。
- [x] 增加 config::driver_name 和 RGM::Driver_Name 常量

# 20230419-todo
- [x] 文档和注释，注释规则：
  1. 使用 doxygen 格式的注释时，用 /// 开头
  2. 其他格式的注释使用 /**/ 的形式
  3. license 使用 // 的形式注释
  4. @name 包含 task / data / todo / meta（元编程相关）
- [x] 使用 std::string_view 代表 ruby 传过来的字符串
- [x] 增加 detail 里转换为 T* 的处理
- [x] wget 加上 -q，pacman 加上 --noprogressbar
- [x] 不存在 config.ini 时自动创建，这个逻辑放到 init_config 的 before 里执行（做不了）
- [x] controller 的几个按键条件是排他的，可以用一个uint8的不同位表示。
- [x] bitmap 使用 bgra，palette 使用 rgba，这些 format 可以在 config 里用常数来表示
- [x] 添加 bitmap.grayscale，对于shader操作，可能可以写一个模板
- [x] 在drawables里用 @data 保存原始指针了，refresh时可以避免查询操作
- [x] 可以用 soundfonts 指定 SF2 音源
- [x] @data改名为@data_ptr，避免重名
- [x] render等任务不再需要v_ptr的参数。此外，drawables需要一个默认的ptr，当drawable的viewport*是空指针时，调用这个ptr。这个ptr会随着screen变化而变化，并且是全局变量，或者指向drawables的静态成员。

# 20230428-todo
- [x] rgm 小助手
- [x] 对 win7 的支持
- [x] 尝试解决 opengl 的 tilemap 的 bug
- [x] 引入协程
- [x] misc.7z打包时带上dll
- [x] 多线程存档导致崩溃，触发方式多切换几次菜单或者场景，然后存档就会在存储$game_map时崩溃。怀疑是跟table的存储有关。可以先试一下移除pmr，看看是不是pmr的bug，因为pmr对效率的提升不明显。单线程（包括协程模式）没有此问题。或者试着多save几次$game_map。

# 20230503-todo
- [ ] 提供兼容不同分辨率的范例工程。切换除地图以外的场景时自动模糊地图背景，并将其他的窗口居中。
- [ ] 注释中 return 的格式规范一下，每种不同的 return 各自占一行。
- [ ] Regist 外部资源后触发 reload
  1. 等待渲染线程
  2. 清空 finder 的缓存
  3. reload 资源
- [ ] RGMDefine 作为未定义时的空函数使用。全部改成`def xxx; end unless defined? xxx`，直接引用。此时也不再需要定义empty的load_embeded_file了。

# 20230608-todo
- [ ] 提供tktk_bitmap.dll的全部功能，并给出兼容脚本。
- [ ] 绑定Mix_SetPanning，使得音乐可以调整左右声道，参考：https://wiki.libsdl.org/SDL2_mixer/Mix_SetPanning