# TCPNetterCpp

> TCPNetter客户端的C++版本，纯调用版本，使用控制台交互，整体打包下来月3mb大小，整体详细参考：https://github.com/Mustenaka/TCPNetter

### 交互

直接在控制台输入，存在3个命令

| 名称 | 参数    | 结果                       |
| ---- | ------- | -------------------------- |
| w    | message | 发送消息给服务器           |
| r    |         | 读取当前本地存储的消息模型 |
| q    |         | 退出程序                   |

### 安装

确保安装了vcpkg，使用vcpkg命令进行安装，首先确保项目存在vcpkg.json和vcpkg-configuration.json并且baseline配置正确，这两个文件就在根目录，随后运行

```
 vcpkg install
```

安装第三方库，安装完成会在目录下生成./vcpkg_installed 文件夹以及在代码文件夹中生成成./TCPNetterCpp/vcpkg ./TCPNetterCpp/x64 ./TCPNetterCpp/x64-windows文件夹，三方库就在这里

整合进入visual studio

```
vcpkg integrate install
```

出错重新安装

```
vcpkg x-set-installed

vcpkg install
```

如何不适用visual studio而是使用cmake，则需要配合在cmake后命令添加路径链接，参考链接代码如下

```
CMake 项目应使用:“-DCMAKE_TOOLCHAIN_FILE=C:/Program Files/Microsoft Visual Studio/2022/Community/VC/vcpkg/scripts/buildsystems/vcpkg.cmake”
```

### 项目结构

1. message_model: 消息数据模型，需要和服务器侧收发消息一致
2. tcp_client：客户端链接服务器代码，具备链接，发送消息，校验，重连等基本功能
3. tcp_client_wrapper：客户端链接的包装层，使用多线程、队列，心跳包设计，维护客户端侧运行，且具备服务器掉线后的自动重连
4. main：调用模块

### 接口（关注）

- tcp_client_wrapper::send_message 发送消息，发送成功会立刻更改自己的message_model内容
- tcp_client_wrapper::get_model 获取当前model