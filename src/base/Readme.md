# Base 代码结构
Base 封装基本的 Ruby 和 SDL2 功能。提供框架的初始化、退出等与具体的业务无关的功能。

## validator.hpp
提供宏 RGMCHECK，在非 release 模式下会打印出内置脚本文件的 crc32 值，在 release 模式下会内置脚本的校验 crc32 值，如果校验失败则退出程序。

## textures.hpp 
定义了 textures 类，继承自 std::unordered_map，用来管理所有的 SDL2 纹理。

## renderstack.hpp
定义了 renderstack 类，继承自 std::vector，用来管理分层渲染的 SDL2 纹理：
1. 最下面一层是 screen
2. 每次渲染非 viewport 时，直接渲染到最后一层的 SDL2 纹理上
3. 每当开始渲染 viewport 时则增加一层 SDL2 纹理
4. 每当结束渲染 viewport 时则将最后一层绘制到倒数第二层上，并移除最后一层

renderstack 类拥有方法 operator<< 和 merge。前者用于添加新的一层 SDL2 纹理，后者接受一个 std::function，表示合并后两层的方式，merge 会按照这个方式合并后两层。

## init_sdl2.hpp
定义了以下任务：
1. poll_event，用来一次性处理积压的 windows 消息事件，但是处理方式在后面绑定
2. init_sdl2，用来初始化和退出SDL。

## init_ruby.hpp
定义了以下任务：
1. synchronize_signal，在其他线程执行，解锁 ruby 线程
2. interrupt_signal，发出 Interrupt 信号，结束 ruby 线程
3. synchronize_ruby，暂停 ruby 线程，等待其他线程的解锁信号
4. init_ruby，初始化 ruby 运行环境，结束时清理 ruby 环境

## kenerl_ruby.hpp
定义了 kerenl_ruby 类，继承自 core::kernel_active。该类只定义了 run 方法，执行预设的 ruby 脚本，这个脚本是游戏的主程序，通常不会主动退出。

## counter.hpp
定义了 counter 类，拥有方法 fetch_and_add，每次的返回值都会 +1。

定义了 init_counter 任务，用于给 ruby 添加 RGM::Base.new_id 方法。

## init_word.hpp
定义了 details 模板类，接受的参数是枚举类型。details 拥有方法 setup，用于在运行时生成 ruby 符号所对应的 ID。details 类的作用是避免每次调用 rb_intern 造成的查询开销。details 还需要实例化 word2name 方法。

## base.hpp
定义了 executor_ruby 和 executor_sdl2 方便后续定义 executor。