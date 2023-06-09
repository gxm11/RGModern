# 更新日志
> 更新日志规范参照：https://www.bestyii.com/topic/75

## [1.0.4] - 2023-06-07
### 修复
- Tone#new和Tone#set现在可以省略第4个参数gray，默认值是0。
- Finder#find现在会检查路径是否为文件夹，并且不会返回对应于文件夹的路径。
- 修复了Font#exist?未生效的BUG。

### 优化
- 允许Graphics#transition使用小于或等于0的duration。
- 现在可以在中文工程路径下读取中文文件名的素材。
- 现在可以读取加密包中的中文素材，不过加密包必须使用`7z a -tzip Graphics2.zip Graphics -scsUTF-8 -p123`指令制作，否则文件名不会以UTF-8的格式保存。
- 范例里的箱子事件现在会读取加密包中的中文文件名素材“加菲猫.jpg”。

### 新增
- Graphics.zip已更新。如果使用RGM小助手，请删掉misc.7z然后更新RGModern。

## [1.0.3] - 2023-05-30
### 修复
- 在Bitmap#font_size函数的C++实现中，将两个未初始化的变量初始化为0。
- 在controller_buttonmap和key_map的各个函数中，显式指定作为函数参数的元素类型为std::pair类型。
- 添加了Viewport#rect=方法。

### 优化
- config.rb中的修改窗口标题、全屏和修改窗口分辨率改为调用RGM::Ext::Window的模块方法。
- Graphics中的update_fps方法改为调用RGM::Ext::Window的模块方法。
- 在读取config之前先等待所有的worker，原来是在rgss_main的开头，即读取scripts.rxdata之前。

### 新增
- 新增了mouse.rb，实现了RGM::Ext::Mouse模块，以提供鼠标功能。提供以下方法：
  - `position` 返回鼠标当前的位置，对全屏模式或者分辨率扩张的场合，此位置会自动映射到正确的坐标。
  - `raw_position` 返回鼠标的当前的位置，未经过坐标映射。
  - `press?` 检查鼠标某个按键是否被按下，目前支持LEFT/MIDDLE/RIGHT/X1/X2共5个按键。
  - `trigger?` 检查鼠标某个按键是否刚刚被按下。
  - `double_click?` 检查鼠标的某个按键是否被双击，默认是帧数的1/3，也就是1/3秒内按下被识别为双击。
  - `double_click_interval=` 设置双击的判定间隔，单位是帧数，范围只能在1~63帧之间。
  - `wheel_down?` 检查鼠标滚轮是否在向下滚动。
  - `wheel_up?` 检查鼠标滚轮是否在向上滚动。
- 新增了window.rb，实现了RGM::Ext::Window模块，用于管理窗口，包括修改分辨率、全屏和获取HWND等。部分原来在Graphics模块中功能被移动到此类中。提供以下方法：
  - `set_title` 设置窗口标题，原属于Graphics模块中的功能。
  - `set_fps` 设置 FPS 的值。设置为nil不显示FPS，设置为-1则显示 Sampling...
  - `set_fullscreen` 设置全屏模式，原属于Graphics模块中的功能。
  - `resize` 重设窗口大小。
  - `refresh_size` 在全屏模式下，获取窗口真实的大小。
  - `cast_to_screen` 在全屏模式下，将鼠标在窗口中的坐标映射到屏幕中的坐标。
  - `get_hwnd` 获取窗口的 HWND，此值只在Windows操作系统有效，其他操作系统始终返回0。

### 删除
- 移除Graphics的@@screen_width和@@screen_height，改为使用@@width和@@height。
- 移除Graphics#set_title和Graphics#set_fullscreen方法。相关功能在RGM::Ext::Window中实现。

## [1.0.2] - 2023-05-27
### 新增
- 新增了 ext/gamecontrollerdb.txt 文件，并在程序中内嵌此文件以支持多种控制器。

## [1.0.1] - 2023-05-12
### 修复
- 修复了 Game.exe（Build Mode 2）无法在中文路径下运行的问题。
- 给 config.ini 文件头部添加了 License，以解决 UTF8-BOM 头导致 `[Game]` 匹配失败的问题。
### 优化
- Viewport 的 dispose 会将 @data_ptr 设置为 nil。
- Graphics 的 set_title 在显示 FPS 的场合会立即生效，而不是跟随下一次 FPS 更新。
- 对 Finder 中缓存的路径字符串执行 freeze。
- 给 src/shader 下的 C++ 代码添加了注释。
- 给部分 C++ 函数添加了`[[nodiscard]]`、`noexcept` 和 `const` 的标记。
- rgm::ext::zip_data_external 类中的 load_texture 和 load_surface 返回 std::optional，而不是裸指针。若返回了 std::nullopt，后续流程中会抛出异常。
- 第 4 个 worker 的标签从 table 改名为 aside。
- rgm::shader 中各类的静态变量全部 inline 处理。
### 新增
- config.ini 的 `[System]` 栏目中添加了 ScreenScaleMode 词条，此值可以设置为 0~3，分别对应不同的画面缩放模式：0：最近邻，1：线性，2：最佳，3：不缩放且居中。
- Input 的 bind 的第二个参数可以使用整数，之前必须是常量名对应的 Symbol。
- config.ini 的 `[Keymap]` 可以使用数字代替虚拟按键的常量名。比如：`K_DOWN=2`。这样便于绑定自定义虚拟按键，因为在读取这部分内容时，很可能自定义虚拟按键的常量尚未定义，如果使用常量名则会报错。
- 新增了 src/script/rgm_defines.rb 文件，可以查看 RGM 模块中定义的方法和常量。仅用于提示开发者 RGM 模块中的内容，不能在脚本中执行。此外，此文件是扫描 C++ 代码自动生成的，故 sprite_create 等方法未列出。
- CMakeList 添加了 MSVC 中预编译头的设置。
- 新增了 src/script/kernel.rb 文件，此文件中定义了所有的全局函数。
### 删除
- 移除了 src/script 中的 main-xp.rb 和 test.rb 文件。
- 移除了 src/script 下的 debug.rb 文件，代码移动到新增的 kernel.rb 文件中。
- 移除了 src/rmxp/drawable.hpp 中复杂的 pmr 方案。此方案并未实装，目前使用的是简单的 pmr 方案。
- 移除了 rgm::init_shader 的定义，使用 rgm::shader::init_shader。

## [1.0.0] - 2023-05-04
正式版发布。

[1.0.0]: https://github.com/gxm11/RGModern/releases/tag/v1.0.0
[1.0.1]: https://github.com/gxm11/rgmodern/compare/v1.0.0...v1.0.1
[1.0.2]: https://github.com/gxm11/rgmodern/compare/v1.0.1...v1.0.2
[1.0.3]: https://github.com/gxm11/rgmodern/compare/v1.0.2...v1.0.3
[1.0.4]: https://github.com/gxm11/rgmodern/compare/v1.0.3...v1.0.4