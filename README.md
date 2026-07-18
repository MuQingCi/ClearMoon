# ClearMoon —— 高性能 C++ 网络库

[![C++11](https://img.shields.io/badge/C++-11/14-blue.svg)](https://en.cppreference.com/w/cpp/11)
[![Build](https://img.shields.io/badge/build-cmake-green.svg)](CMakeLists.txt)
[![License](https://img.shields.io/badge/license-MIT-yellow.svg)](LICENSE)

ClearMoon 是一个自底向上实现的高性能 C++ 网络库，遵循 **Reactor 多线程模型**，基于 **epoll(LT) 事件驱动**，集成了 HTTP 协议解析、异步日志系统、定时器管理及零拷贝文件传输等能力。支持万级并发连接，经 wrk 压测可达 QPS 16.2 万。

---

## 目录

- [特性](#特性)
- [技术栈](#技术栈)
- [压测数据](#压测数据)
- [项目架构](#项目架构)
- [核心模块详解](#核心模块详解)
  - [1. 基础组件层 (base)](#1-基础组件层-base)
  - [2. Reactor 网络层 (net)](#2-reactor-网络层-net)
  - [3. HTTP 服务层 (net/Http)](#3-http-服务层-nethttp)
  - [4. 异步日志系统 (net/Log)](#4-异步日志系统-netlog)
- [构建与使用](#构建与使用)
- [演示](#演示)

---

## 特性

- **Reactor 多线程架构**：Main Reactor（主线程）负责 accept 分发新连接，Sub Reactors（IO 线程池）负责已连接套接字的读写事件处理
- **epoll LT 模式**：事件循环基于 epoll 水平触发，配合非阻塞 I/O 实现高吞吐
- **跨线程唤醒**：通过 `eventfd` 实现跨线程任务投递与唤醒
- **HTTP 协议解析**：基于有限状态机实现 HTTP 请求行/头部/消息体的逐字节解析，支持长连接（Keep-Alive）与路径路由
- **零拷贝文件传输**：集成 Linux `sendfile` 系统调用，大文件传输无需经过用户态缓冲区，支持 Buffer 模式与零拷贝模式自动切换
- **异步日志系统**：前端日志流式接口 + 后端线程批量落盘，避免磁盘 I/O 阻塞网络线程
- **定时器管理**：基于 `timerfd` + `std::set` 的定时器队列，支持一次性/周期性定时器、超时重传与空闲连接定期清理
- **自动缓冲区管理**：非阻塞 I/O Buffer 带 8 字节预留头部（prependable space），支持自动扩容与收缩
- **RAII 资源管理**：智能指针管理连接生命周期，所有资源（Socket、Channel、Timer）遵循 RAII 原则

---

## 技术栈

| 类别 | 技术 |
|------|------|
| 语言标准 | C++11/14 |
| 事件驱动 | epoll (LT 模式) |
| 并发模型 | Reactor 多线程（1 Main Reactor + N Sub Reactors）|
| 零拷贝 | `sendfile` 系统调用 |
| IPC/唤醒 | `eventfd` |
| 定时器 | `timerfd` + `std::set` 有序管理 |
| 日志 | 双缓冲异步日志，后端线程批量刷盘 |
| 构建 | CMake |

---

## 压测数据

> **测试环境**  
> CPU: AMD-R7 7745HX（8 物理核心 / 16 逻辑核心）  
> 系统: WSL2 + ArchLinux  
> 测试工具: wrk  
> 分配方案：8 核分配给服务器进程，8 核分配给 wrk 施压  
> 测试接口: `/hello`（文本返回）

| 并发数 | QPS | 平均延迟 | 吞吐量 |
|--------|-----|---------|--------|
| 500    | 15.4 万 | 3.9 ms | ~13 MB/s |
| 800    | **16.2 万** | 5.2 ms | ~14 MB/s |
| 1000   | 稳定 ≥ 15 万 | 5.5 ms | ~13 MB/s |

![c500 压测](./test/c500_d30.png)
![c800 压测](./test/c800_d30.png)
![c1000 压测](./test/c1000_d30.png)

---

## 项目架构

```
├── base/                          # 基础组件层（无网络依赖）
│   ├── Thread.h/cc               # pthread 封装，支持线程命名
│   ├── ThreadPool.h/cc           # 线程池，基于 BlockQueue
│   ├── BlockQueue.h/cc           # 线程安全阻塞队列（生产者-消费者）
│   ├── Mutex.h                   # RAII 互斥锁 MutexGuard
│   ├── Condition.h               # pthread_cond_t 封装
│   ├── CountDownLatch.h/cc        # 倒计时门闩（线程同步）
│   ├── CurrentThread.h/cc         # 线程本地缓存 tid
│   ├── noncopy.h / copy.h         # 不可拷贝 / 可拷贝基类
│   └── Types.h                    # 类型别名
│
├── net/                           # 网络核心层
│   ├── EventLoop.h/cc             # 事件循环（epoll + eventfd 唤醒）
│   ├── Channel.h/cc               # fd 事件分发器
│   ├── Poller/                    # epoll 封装（LT 模式）
│   │   ├── Poller.h/cc
│   │   ├── Epoller.h/cc
│   │   └── DefaultPoller.h
│   ├── Acceptor.h/cc              # TCP 监听器（accept 新连接）
│   ├── TcpConnection.h/cc         # TCP 连接（读写缓冲区、sendfile、重传、空闲清理）
│   ├── TcpServer.h/cc             # TCP 服务器（Main Reactor + 线程池分发）
│   ├── TcpClient.h/cc             # TCP 客户端
│   ├── Connector.h/cc             # 异步连接器
│   ├── EventLoopThread.h/cc       # 单线程 EventLoop 封装
│   ├── EventLoopThreadPool.h/cc   # IO 线程池（轮询分发）
│   ├── Socket.h/cc                # RAII socket 封装
│   ├── InetAddress.h/cc           # IPv4/IPv6 地址封装
│   ├── Buffer.h/cc                # 非阻塞 I/O 缓冲区（8 字节预留头部）
│   ├── Endian.h                   # 大小端字节序转换
│   ├── Timer.h/cc / TimerId.h     # 定时器与 ID 封装
│   ├── TimerQueue.h/cc            # 定时器队列（timerfd + std::set）
│   ├── Timestamp.h/cc             # 时间戳（微秒精度）
│   └── Callbacks.h                # 回调类型定义
│
├── net/Http/                      # HTTP 服务层
│   ├── HttpServer.h/cc            # HTTP 服务器（解析 + 路由 + 文件传输）
│   ├── HttpContext.h/cc           # HTTP 有限状态机解析器
│   ├── HttpRequest.h              # HTTP 请求结构
│   ├── HttpResponse.h             # HTTP 响应结构（支持文件模式/Buffer 模式）
│   └── HttpParse.h                # HTTP 方法/版本字符串工具
│
├── net/Log/                       # 异步日志系统
│   ├── Logger.h/cc                # 前端 Logger（流式接口 + 宏定义）
│   ├── AsyncLogger.h/cc           # 后端异步日志线程
│   └── [...]
│
├── test/                          # 测试 & 示例
│   ├── HttpServer_test.cc         # HTTP 服务器测试入口
│   ├── HttpCallback.h/cc          # HTTP 路由回调示例
│   ├── integration_test.cc        # 集成测试
│   └── *.png                      # 压测结果截图
│
├── resource/                      # 资源目录（用户创建，存放静态文件）
├── CMakeLists.txt
└── main.cc
```

---

## 核心模块详解

### 1. 基础组件层 (base)

提供与网络无关的线程安全基础组件，为上层的多线程 Reactor 模型提供支撑。

| 组件 | 说明 |
|------|------|
| **Thread** | `pthread_create` / `join` / `detach` 封装，支持设置线程名（`prctl`），使用 `CountDownLatch` 同步确保线程启动后 `tid` 立即可用 |
| **ThreadPool** | 固定大小线程池，内部维护 `BlockQueue` 作为任务队列，工作线程通过 `getTask()` 阻塞获取任务执行 |
| **BlockQueue** | 线程安全的阻塞队列，支持 `maxSize` 限制、满时等待、停止时唤醒所有等待线程 |
| **Mutex + MutexGuard** | `pthread_mutex_t` 封装 + RAII 守卫类，支持调试时记录持有者 `tid` |
| **Condition** | `pthread_cond_t` 封装，提供 `wait` / `notify` / `notifyAll` |
| **CountDownLatch** | 倒计时门闩，适用于"所有线程就绪后再开始"的场景（如线程启动同步） |
| **CurrentThread** | `thread_local` 缓存线程 `tid` 及字符串表示，避免频繁系统调用 |

### 2. Reactor 网络层 (net)

网络核心层，实现完整的 Reactor 多线程模型。

#### 2a. 事件循环 —— EventLoop + Channel + Poller

```
┌─────────────────────────────────────────────────┐
│                  EventLoop                       │
│  ┌──────────────────────────────────────────┐   │
│  │   epoll_wait()  ──→ activeChannels       │   │
│  │         │                                │   │
│  │         ▼                                │   │
│  │   for each Channel: handleEvent()        │   │
│  │         │                                │   │
│  │         ▼                                │   │
│  │   doPendingFuncs() (跨线程任务)           │   │
│  └──────────────────────────────────────────┘   │
│                                                  │
│  wakeup fd (eventfd)     timerfd                 │
└─────────────────────────────────────────────────┘
```

- **EventLoop**：每个 IO 线程一个实例，内部持有 `epoll` 实例。`loop()` 方法不断轮询事件，处理活跃 Channel，再执行跨线程投递的待处理回调
- **Channel**：封装一个文件描述符及其关注的事件（EPOLLIN / EPOLLOUT / EPOLLERR），持有对应的三种回调（read / write / error）。`handleEvent()` 根据 `revents` 分发到合适回调
- **Poller/Epoller**：`epoll_create` / `epoll_ctl` / `epoll_wait` 封装，使用 LT 水平触发模式
- **跨线程唤醒**：通过 `eventfd` 实现，其他线程向该 EventLoop 投递任务时写入 eventfd，epoll 立即返回并执行待处理任务列表

#### 2b. 多线程架构 —— Main Reactor + Sub Reactors

```
                    ┌──────────────┐
                    │  Main Loop   │  ← epoll_wait (只监听 Acceptor)
                    │  (Thread 0)  │
                    └──────┬───────┘
                           │ accept 新连接
                           ▼
              ┌────────────────────────┐
              │  EventLoopThreadPool   │  ← 轮询（Round-Robin）分发
              └───┬───┬───┬───┬───┬──┘
                  │   │   │   │   │
                  ▼   ▼   ▼   ▼   ▼
              ┌────────────────────────┐
              │  Sub Loop 1 .. N      │  ← 各 IO 线程独立 epoll_wait
              │  (IO Threads)         │     处理已连接套接字的读写
              └────────────────────────┘
```

- **TcpServer**：持有 `Acceptor`（绑定在 Main Loop）和 `EventLoopThreadPool`。新连接到达时，Main Loop 的 Acceptor 调用 `accept`，获取 Socket 后轮询选择一个 Sub Loop，在该 Sub Loop 中创建 `TcpConnection`
- **TcpConnection**：每个客户端连接对应一个 `TcpConnection` 实例，包含 `Channel`、读写 `Buffer`、文件发送状态、超时重传状态、空闲连接定时器等。所有操作均在所属的 Sub Loop 线程中执行
- **EventLoopThreadPool**：创建若干 `EventLoopThread`，每个内部运行一个独立的 `EventLoop`，采用轮询（Round-Robin）分发新连接

#### 2c. Socket 与地址封装

- **Socket**：RAII 风格封装，移动语义管理所有权。支持 `socket` / `bind` / `listen` / `accept4`（非阻塞 + CLOEXEC），以及 `setsockopt` 常用选项（REUSEADDR、REUSEPORT、TCP_NODELAY、KEEPALIVE）
- **InetAddress**：IPv4/IPv6 兼容的地址封装，内部使用 union 存储 `sockaddr_in` / `sockaddr_in6`，支持 IP:Port 字符串互转

#### 2d. 非阻塞 Buffer

- **Buffer**：设计参考 muduo，预留 8 字节头部空间（prependable），读写索引分离
  - `readIndex_` → `writeIndex_` 之间为可读数据
  - `prependBytes()` 可用于在已读数据前写入长度信息（如消息头编码）
  - `readFd()` 利用 `readv` + 栈上 64KB 额外缓冲区的两段式读取，避免缓冲区不足时的多次系统调用
  - 自动扩容：当可写空间不足时，优先整理前端空闲空间，再考虑扩容

#### 2e. 定时器系统

- **TimerQueue**：使用一个全局 `timerfd` 驱动，内部以 `std::set<Entry>` 按到期时间有序管理所有定时器
- 每次最早的定时器到期时，`timerfd` 触发可读事件，`handleRead()` 取出所有到期的定时器并执行回调
- 重复定时器在执行后会重新计算到期时间并插入集合
- 支持 `cancel` 操作，采用延迟删除机制避免正在执行回调时删除定时器导致的竞态

#### 2f. 可靠传输与空闲连接管理

- **超时重传**：`TcpConnection` 支持 `sendWithRetransmit()`，为每个消息包记录序列号，启动重传定时器。未收到 ACK 则按指数退避重传，超过最大重试次数（5 次）则断开连接
- **空闲连接清理**：`TcpConnection` 维护一个空闲定时器（默认 60 秒），每次读写事件触发时重置。超时未活动的连接将被强制关闭，防止连接泄漏

### 3. HTTP 服务层 (net/Http)

构建在网络核心层之上的 HTTP 协议实现。

#### 3a. 有限状态机解析 (HttpContext)

```
请求行 (kExpectRequestLine)
    ── 解析 "GET /path HTTP/1.1\r\n" ──→
头部 (kExpectHeaders)
    ── 逐行解析 "Key: Value\r\n" ──→
    ── 空行 "\r\n" ──→
消息体 (kExpectBody) [仅在 POST/PUT/PATCH 时]
    ── 按 Content-Length 读取 body ──→
解析完成 (kGotAll)
```

- 状态机逐字节推进，读入不足时等待更多数据，不会阻塞
- 自动区分 GET/POST/PUT/PATCH 是否需要消息体
- 支持长连接：解析完成后 `context.reset()` 将状态机重置到 `kExpectRequestLine`，准备解析下一个请求

#### 3b. 文件传输模式

- **普通模式**：响应体通过 Buffer 序列化后由 `TcpConnection::send()` 发送
- **文件模式（零拷贝）**：响应仅序列化 HTTP 头部，再调用 `sendfile()` 直接从内核态将文件内容发送到 TCP 连接，用户态零拷贝
- 用户通过 `HttpResponse::setFilePath()` 开启文件模式，HttpServer 自动选择发送路径

#### 3c. HTTP 回调示例

```cpp
// test/HttpCallback.cc
void httpCallback(const TcpConnectionPtr& conn, HttpRequest& req, HttpResponse& resp)
{
    if (req.getPath() == "/hello")
    {
        resp.setStatus(HttpStatusCode::k200Ok);
        resp.setBody("Hello!");
        resp.setContentType("text/plain");
    }
    else if (req.getPath() == "/video.mp4")
    {
        std::string path = std::string(PROJECT_ROOT) + "/resource/video.mp4";
        resp.setFilePath(path, "video/mp4");  // 零拷贝发送
        resp.setStatus(HttpStatusCode::k200Ok);
    }
    // ...
}
```

### 4. 异步日志系统 (net/Log)

```
┌──────────┐   LOG_INFO << "msg"    ┌──────────────┐
│  Thread 1 ├───────────────────────►              │
├──────────┤                        │   Logger     │
│  Thread 2 ├───────────────────────►  (前端流式)   │
├──────────┤                        │              │
│  Thread 3 ├───────────────────────►              │
└──────────┘                        └──────┬───────┘
                                           │ append()
                                           ▼
                                   ┌──────────────┐
                                   │ AsyncLogger   │
                                   │ (后端线程)     │
                                   │              │
                                   │ Buffer1  ◄── │ ← 当前缓冲
                                   │ Buffer2      │ ← 预备缓冲
                                   │ buffers_     │ ← 待落盘缓冲队列
                                   │              │
                                   │  file_       │ ← ofstream
                                   └──────────────┘
```

- **Logger**：前端类，提供流式接口 `LOG_INFO << ...`，在构造函数中格式化时间戳、线程 ID、日志级别、文件名、行号等信息
- **AsyncLogger**：后端线程，使用双缓冲策略（当前缓冲 + 预备缓冲 + 待落盘缓冲队列）
  - 前端 `append()` 写入当前缓冲，满时交换预备缓冲继续写入
  - 后端线程定期（或缓冲满时）将待落盘队列中的缓冲写入磁盘文件
  - 支持按文件大小滚动
- **日志宏**：`LOG_DEBUG` / `LOG_INFO` / `LOG_WARNING` / `LOG_ERROR`，未启用异步日志时输出到 `stderr`

---

## 构建与使用

### 前置要求

- CMake ≥ 3.10
- 支持 C++11/14 的编译器（GCC / Clang）
- Linux 内核 ≥ 3.9（需要 eventfd、timerfd、sendfile 支持）

### 构建步骤

```bash
# 1. 创建 build 目录并构建
mkdir build && cd build
cmake .. && make

# 2. 准备静态资源目录（可选，测试 HTTP 文件服务需要）
mkdir ../resource
# 将测试文件放入 ../resource/，如 video.mp4、JTH.jpg 等

# 3. 运行 HTTP 服务器测试
./HttpServer_test

# 4. 浏览器访问
# http://127.0.0.1:12345/hello       ← 文本接口
# http://127.0.0.1:12345/video.mp4   ← 视频文件（零拷贝）
# http://127.0.0.1:12345/JTH         ← 图片文件（零拷贝）
```

### 选项配置

可在 `HttpCallback.cc` 中修改的路由和文件路径，或通过继承 `HttpServer` 实现自定义业务逻辑。

---

## 演示

![ClearMoon 演示](./test/ClearMoon_Test.gif)

---

## 项目参考

本项目架构设计参考了 [muduo](https://github.com/chenshuo/muduo) 网络库的设计思想，并在此基础上增加了 HTTP 协议解析、sendfile 零拷贝、超时重传机制、空闲连接管理、异步日志等特性。