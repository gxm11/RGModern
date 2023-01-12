# 代码结构
进入相应文件夹查看 Readme.md 了解细节。

## core
引擎的核心代码，实现多线程执行框架，多态数据管理器，线程间信息传递等基础功能。

## base
封装基本的 Ruby 和 SDL2 功能。这些功能跟具体的业务（游戏引擎）无关。

## rmxp
RMXP 各功能及数据结构的具体实现。

## script
存放所有的 Ruby 脚本。

## shader
Pixel shader 的 HLSL 脚本和编译后的 .h 文件。