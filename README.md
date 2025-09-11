# C-Socket-Chat

一个用 **C 语言** 实现的简易聊天室，基于 **TCP Socket + select()**，支持多客户端并发、群聊、私聊（通过发送 `@username: message` 可扩展）。

## 快速开始

```bash
make            # 生成 server 和 client
./server 8888   # 启动服务端
./client 127.0.0.1 8888  # 启动客户端（可开多个终端模拟多人）
```

客户端输入 `/quit` 可退出。

## 目录结构
```
server.c
client.c
Makefile
README.md
```

## 说明

- 服务器使用 `select()` 同步多路复用处理多个客户端连接。
- 演示版本做了最小实现，足以用于课程展示与代码讲解。
- 可扩展点：用户名登录、私聊协议、消息带时间戳、命令帮助、日志记录等。