# RGModern todo list

## 战斗测试
战斗测试时，载入太快了，无渐变效果。但是在游戏内是有的。（FEATURE）

32位下render的stopwatch精度有问题，无法计时。

## 发布前的TODO
shader_tone可以简化写法。

考虑到创建drawable和texture的开销，应该在每个线程使用一个资源池：https://zhuanlan.zhihu.com/p/359409607。使用std::pmr下的unsynchronized_pool_resource
https://en.cppreference.com/w/cpp/memory/polymorphic_allocator
https://www.cppstories.com/2020/06/pmr-hacking.html/
https://zhuanlan.zhihu.com/p/96089089
https://wanghenshui.github.io/2019/04/28/allocator.html

考虑到进一步的兼容性，可以把SDL2和ruby都用动态链接，并且可以设置默认的渲染器。添加对opengl，d3d9的支持，增加相应的shader，对于software，禁用shader。（实际上动态链接就可以在win7上跑了，也支持动态链接加密数据，虽然意义不大）

仔细检查32位系统的兼容性设置，可以将xorstr需要的AVXXXX宏，在makefile里实现，尝试linux下编译（假设已经安装了SDL和ruby，或者ruby可以静态编译）

https://wiki.libsdl.org/SDL_RenderReadPixels 实现Bitmap#get_pixel。注意这是一个同步方法，所以需要等待渲染线程完成这个指令。（已完成）

对于image，Finder会缓存长和宽，避免多次读取。（已完成）
向palette添加convert_to_bitmap方法。与Graphics的snap_to_bitmap方法一样，添加默认的rect参数，表示截取的矩形部分，默认是nil，表示截取全部。

新增同步的passive kernel，修改了 << 方法使其立刻执行，并且在 scheduler 的 run 里也不会开独立线程。（已解决，增加了 scheduler_synchornized）

添加load data和save data的第二个参数为密码，这样就会用加密的zip格式保存文件。由于zip格式和marshal格式的开头是不一样的，如果读取时发现是marshal格式，就无视密码的效果，做到向前兼容。

范例工程和文档。

添加鼠标、joystick等控制，但是这些应该位于rgm::ext下。包括Movie，加入视频播放功能