# AgoraDualTeacher

_**其他语言版本：** [**简体中文**](README.zh.md)_

## Overview

The AgoraDualTeacher project is an open-source demo that will show you how to integrate Agora SDK APIs into your project.

## How to run the sample project

#### Developer Environment Requirements

- Agora.io [Developer Account](https://dashboard.agora.io/signin/)
- [CMake](https://cmake.org/download/) >= 3.5
- [QT x64](https://www.qt.io/download-qt-installer) >= 6.2.1
- [Vistual Studio](https://visualstudio.microsoft.com/downloads/) >= 2019


#### Steps to run

- Create a developer account at [Agora.io](https://dashboard.agora.io/signin/), and obtain an AppID.
- Clone project files
```shell
$ git clone https://github.com/AgoraIO-Usecase/AgoraDualTeacher.git
$ cd AgoraDualTeacher
```

- Download specified version of sdk and extract to `deps/win`.
- Set the `APP_ID` in [src/AgoraEnv.h](src/AgoraEnv.h) with your AppID.
- Replace the value of `QT_DIR` in [CMakeLists.txt](CMakeLists.txt) with the truely QT folder.

Then do the following:

```shell 
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```

## Feedback

If you have any problems or suggestions regarding the sample projects, feel free to file an issue.

## Reference

- You can find full API document at [Document Center](https://docs.agora.io/en/Video/API%20Reference/electron/index.html)
- You can file issues about this demo at [issue](https://github.com/AgoraIO/Electron-SDK/issues)

## Related resources

- Check our [FAQ](https://docs.agora.io/en/faq) to see if your issue has been recorded.
- Dive into [Agora SDK Samples](https://github.com/AgoraIO) to see more tutorials
- Take a look at [Agora Use Case](https://github.com/AgoraIO-usecase) for more complicated real use case
- Repositories managed by developer communities can be found at [Agora Community](https://github.com/AgoraIO-Community)
- If you encounter problems during integration, feel free to ask questions in [Stack Overflow](https://stackoverflow.com/questions/tagged/agora.io)

## License

The sample projects are under the MIT license.