#ifndef CLEARMOON_NET_TCPSERVER_H
#define CLEARMOON_NET_TCPSERVER_H

#include "../base/noncopy.h"
#include "Acceptor.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "Socket.h"
#include "Callbacks.h"

#include <unordered_map>

namespace clearmoon 
{
namespace net 
{

class TcpServer : public noncopyable
{
public:
using ThreadPoolInitCallback = std::function<void(EventLoop*)>;

    TcpServer(EventLoop* loop, const ThreadPoolInitCallback& cb, const InetAddress& listenAddr);
    ~TcpServer();
    
    //设置回调
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb){ writeCompleteCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }


    void start();
    void stop();
private:
    void newConnection(Socket socket, InetAddress peerAddress);

    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    EventLoop* baseloop_;
    EventLoopThreadPool eventThreadPool_;
    
    Acceptor acceptor_;

    std::string threadName_;
    bool started_;
    int nextConnId_;

    std::unordered_map<std::string, TcpConnectionPtr> connections_;

    ConnectionCallback connectionCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    MessageCallback messageCallback_;
};

}//net

}//clearmoon


#endif