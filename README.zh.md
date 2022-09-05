# AgoraDualTeacher

_Read this in other languages: [English](README.md)_

## 简介

这个开源示例项目演示了双师场景下，Agora SDK的集成逻辑。


## 如何运行示例程序

#### 运行环境

- Agora.io [Developer Account](https://dashboard.agora.io/signin/)
- [CMake](https://cmake.org/download/) >= 3.5
- [QT x64](https://www.qt.io/download-qt-installer) >= 6.2.1
- [Vistual Studio](https://visualstudio.microsoft.com/downloads/) >= 2019

#### 运行步骤

- 首先在 [Agora.io 注册](https://dashboard.agora.io/cn/signup/) 注册账号，并创建自己的测试项目，获取到 AppID。
- 获取代码
```shell
$ git clone https://github.com/AgoraIO-Usecase/AgoraDualTeacher.git
$ cd AgoraDualTeacher
```
- 获取指定版本sdk并将其解压至当前目录下的 `deps/win` 文件夹中。
- 将获取到的AppID填入 [src/AgoraEnv.h](src/AgoraEnv.h)。
- 修改 [CMakeLists.txt](CMakeLists.txt) 中 `QT_DIR` 为您的QT安装路径。

然后进行以下操作:

```shell
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```

## 反馈

如果您对示例项目有任何问题或建议，请随时提交问题。

## 参考文档

- 您可以在 [文档中心](https://docs.agora.io/cn/Video/API%20Reference/electron/index.html)找到完整的 API 文档

## 相关资源

- 你可以先参阅[常见问题](https://docs.agora.io/cn/faq)
- 如果你想了解更多官方示例，可以参考[官方 SDK 示例](https://github.com/AgoraIO)
- 如果你想了解声网 SDK 在复杂场景下的应用，可以参考[官方场景案例](https://github.com/AgoraIO-usecase)
- 如果你想了解声网的一些社区开发者维护的项目，可以查看[社区](https://github.com/AgoraIO-Community)
- 若遇到问题需要开发者帮助，你可以到[开发者社区](https://rtcdeveloper.com/)提问
- 如果需要售后技术支持, 你可以在[Agora Dashboard](https://dashboard.agora.io/)提交工单

## 代码许可

示例项目遵守 MIT 许可证。