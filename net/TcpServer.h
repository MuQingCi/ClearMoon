#ifndef CLEARMOON_NET_TCPSERVER_H
#define CLEARMOON_NET_TCPSERVER_H

#include "../base/noncopy.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "Socket.h"

namespace clearmoon 
{
namespace net 
{
class TcpServer : public noncopyable
{
public:
using ThreadPoolInitCallback = std::function<void(EventLoop*)>;

    TcpServer(EventLoop* loop, ThreadPoolInitCallback& cb, InetAddress& listenAddr);
    ~TcpServer();
    
    void start();
    void stop();
private:
    Socket newConnection();

    EventLoop* baseloop_;
    EventLoopThreadPool eventThreadPool_;
    
    std::string threadName_;
    bool started_;


};

}//net

}//clearmoon


#endif