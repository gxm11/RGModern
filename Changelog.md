# 更新日志
> 更新日志规范参照：https://www.bestyii.com/topic/75

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
- rgm::ext::zip_data_external 类中的 load_texture 和 load_surface 现在返回 std::optional，而不是裸指针。若返回了 std::nullopt，后续流程中会抛出异常。
### 新增
- config.ini 的 `[System]` 栏目中添加了 ScreenScaleMode 词条，此值可以设置为 0~3，分别对应不同的画面缩放模式：0：最近邻，1：线性，2：最佳，3：不缩放且居中。
- Input 的 bind 的第二个参数可以使用整数，之前必须是常量名对应的 Symbol。
- config.ini 的 `[Keymap]` 可以使用数字代替虚拟按键的常量名。比如：`K_DOWN=2`。这样便于绑定自定义虚拟按键，因为在读取这部分内容时，很可能自定义虚拟按键的常量尚未定义，如果使用常量名则会报错。
- 新增了 src/scripts/rgm_defines.rb 文件，可以查看 RGM 模块中定义的方法和常量。仅用于提示开发者 RGM 模块中的内容，不能在脚本中执行。此外，此文件是扫描 C++ 代码自动生成的，故 sprite_create 等方法未列出。
- CMakeList 添加了 MSVC 中预编译头的设置。

## [1.0.0] - 2023-05-04
正式版发布。

[1.0.0]: https://github.com/gxm11/RGModern/releases/tag/v1.0.0
[1.0.1]: https://github.com/gxm11/rgmodern/compare/v1.0.1...v1.0.0