#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "EventLoop.h"
#include "net/TcpServer.h"
#include "net/InetAddress.h"
#include "net/TcpConnection.h"
#include "net/Buffer.h"
#include "net/Timestamp.h"

using namespace clearmoon;
using namespace clearmoon::net;

int main()
{
    EventLoop loop;

    InetAddress listenAddr("127.0.0.1", 12345, false);

    TcpServer server(&loop, TcpServer::ThreadPoolInitCallback(), listenAddr);

    server.setMessageCallback([](const TcpConnectionPtr& conn, Buffer* buf, Timestamp){
        std::string msg = buf->readAllAsString();
        conn->send(msg);
    });

    server.start();

    // run client in a separate thread; EventLoop::loop must run in the thread where EventLoop was created
    std::thread clientThread([&](){
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        int sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if(sock < 0) { std::cerr << "socket failed\n"; return; }

        struct sockaddr_in serv{};
        serv.sin_family = AF_INET;
        serv.sin_port = htons(12345);
        inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr);

        if(connect(sock, (struct sockaddr*)&serv, sizeof(serv)) < 0) { std::cerr << "connect failed\n"; close(sock); return; }

        const char* msg = "hello world";
        send(sock, msg, strlen(msg), 0);

        char buf[1024];
        ssize_t n = recv(sock, buf, sizeof(buf) - 1, 0);
        if(n <= 0) { std::cerr << "recv failed\n"; close(sock); return; }
        buf[n] = '\0';

        std::string resp(buf);
        if(resp != "hello world") { std::cerr << "unexpected echo: " << resp << "\n"; close(sock); return; }

        std::cout << "echo ok\n";
        close(sock);

        // exit immediately to avoid cross-thread destructor ordering issues
        ::_exit(0);
    });

    // run loop in this thread
    loop.loop();

    clientThread.join();
    return 0;
}
