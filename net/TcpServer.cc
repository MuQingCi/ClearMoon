#include "TcpServer.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"

using namespace clearmoon;
using namespace clearmoon::net;

TcpServer::TcpServer(EventLoop* loop, ThreadPoolInitCallback& cb, InetAddress& listenAddr) : baseloop_(loop), eventThreadPool_(cb)
{

}

void TcpServer::start()
{
    if(started_) return;

    eventThreadPool_.start();

}