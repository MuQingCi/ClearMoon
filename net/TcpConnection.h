#ifndef CLEARMOON_NET_TCPCONNECTION_H
#define CLEARMOON_NET_TCPCONNECTION_H

#include "../base/noncopy.h"
#include "Callbacks.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"

#include <string>

namespace clearmoon 
{
namespace net 
{
class TcpConnection : public noncopyable
{
public:
    TcpConnection(EventLoop* loop, std::string name, Socket socket, const InetAddress& localAddr, const InetAddress& peerAddr);
    ~TcpConnection();

    void shutdown();
    void connectEstablelished();
    void connectDestroyed();

    //设置回调函数
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

    //获取本类成员变量
    const std::string name() const { return name_; }
    EventLoop* getLoop() const { return loop_; }

private:
    EventLoop* loop_;
    InetAddress localAddr_;
    InetAddress peerAddr_;
    Socket socket_;

    std::string name_;

    Buffer writeBuffer_;
    Buffer readBuffer_;
    
    enum State{
        kConnecting,
    };

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    CloseCallback closeCallback_;

};
}
}

#endif