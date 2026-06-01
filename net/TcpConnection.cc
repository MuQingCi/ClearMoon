#include "TcpConnection.h"
#include "Channel.h"
#include "Timestamp.h"

#include <cassert>
#include <cerrno>
#include <unistd.h>
#include <utility>

using namespace clearmoon;
using namespace clearmoon::net;

TcpConnection::TcpConnection(EventLoop* loop, std::string name, 
                            Socket socket, const InetAddress& localAddr, 
                            const InetAddress& peerAddr) 
                            : loop_(loop), 
                              channel_(loop,socket.Fd()),
                              name_(name), 
                              socket_(std::move(socket)), 
                              localAddr_(std::move(localAddr)), 
                              peerAddr_(std::move(peerAddr))
                              
{
    channel_.setReadCallback([this]{ handleRead(); });
    channel_.setWriteCallback([this] { handleWrite(); });
    channel_.setErrorCallback([this] { handleError(); });
    socket_.setNonBlocking(true);
}

void TcpConnection::connectEstablelished()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    channel_.enableReading();
    setState(kConnected);
    if(connectionCallback_) 
        connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();

    
    if(state_ == kConnected)
    {
        setState(kDisConnected);
        channel_.disableAll();
    }
}


void TcpConnection::shutdown()
{
    if(state_ == kConnected)
    {
        setState(kDisConnecting);
        loop_->runInLoop([this] { shutdownInLoop(); });
    }
}

void TcpConnection::forceClose()
{
    if(state_ == kConnected || state_ == kDisConnecting)
    {
        setState(kDisConnecting);
        loop_->runInLoop([this]{ forceCloseInLoop(); });
    }
}

//---------------private---------------
void TcpConnection::handleRead()
{
    loop_->assertInLoopThread();

    int savedErrno = 0;
    ssize_t n = readBuffer_.readFd(channel_.getFd(), &savedErrno);

    Timestamp t = Timestamp::now();
    if(n > 0 )
    {
        if(messageCallback_) messageCallback_(shared_from_this(), &readBuffer_, t);
    }
    else if(n == 0)
    {
        handleClose();
    }
    else {
        if(savedErrno != EAGAIN)
        //TODO 处理错误
        handleError();
    }
        
}

void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();

    if(!channel_.isWriting()) return;

    int savedErrno = 0;
    ssize_t n = writeBuffer_.WriteFd(channel_.getFd(), &savedErrno);

    if(n > 0)
    {
        if(writeBuffer_.readableBytes() == 0)
            channel_.disableWriting();
        if(writeCompleteCallback_)
            writeCompleteCallback_(shared_from_this());
        if(state_ == kDisConnecting)
            shutdownInLoop();
    }
    else {
        if(savedErrno != EAGAIN)
        //TODO 处理错误
        handleError();
    }
}

void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisConnecting);
    setState(kDisConnected);
    channel_.disableAll();
    if(connectionCallback_)
        connectionCallback_(shared_from_this());
    if(closeCallback_)
        closeCallback_(shared_from_this());
}

void TcpConnection::handleError()
{
    //TODO 日志记录错误
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if(!channel_.isWriting())
        socket_.shutdownWrite();
}

void TcpConnection::forceCloseInLoop()
{
    loop_->assertInLoopThread();
    if(state_ == kConnected || state_ == kDisConnecting)
        handleClose();
}