#include "TcpConnection.h"
#include "Channel.h"
#include "Timestamp.h"

#include <cassert>
#include <cerrno>
#include <cstddef>
#include <unistd.h>
#include <utility>

using namespace clearmoon;
using namespace clearmoon::net;

TcpConnection::TcpConnection(EventLoop* loop, std::string name, 
                            Socket socket, InetAddress& localAddr, 
                            InetAddress& peerAddr) 
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

TcpConnection::~TcpConnection()
{
    assert(state_ == kDisConnected);
    if(state_ != kDisConnected)
        forceClose();
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
    channel_.remove();
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

void TcpConnection::send(const std::string& message)
{
    send(message.data(), message.size());
}
void TcpConnection::send(const void* data, size_t len)
{
    if(state_ != kConnected) return;

    if (loop_->isInThread()) {
        sendInLoop(data, len);
    } else {
        // 为保证跨线程发送时数据有效性，拷贝数据到 shared_ptr<string>
        auto buf = std::make_shared<std::string>(static_cast<const char*>(data), len);
        loop_->runInLoop([this, buf]{ sendInLoop(buf->data(), buf->size()); });
    }
}

void TcpConnection::sendInLoop(const void* data, size_t len)
{
    loop_->assertInLoopThread();
    if(state_ == kDisConnected) return;

    ssize_t n = 0;
    if(!channel_.isWriting() && writeBuffer_.readableBytes() == 0)
    {
        n = ::write(channel_.getFd(), data, len);
        if(n >= 0)
        {
            if(static_cast<size_t>(n) == len)
            {
                if(writeCompleteCallback_) writeCompleteCallback_(shared_from_this());
                return;
            }
        }
        else
        {
            n = 0;
            if(errno != EAGAIN)
            {
                //TODO 错误处理
            }
        }
    }

    if(static_cast<size_t>(n) < len)
    {
        writeBuffer_.append(static_cast<const char*>(data) + n, len - n);
        if(!channel_.isWriting())
        {
            channel_.enableWriting();
        }
    }

    if(state_ == kDisConnecting && writeBuffer_.readableBytes() == 0)
        shutdownInLoop();
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
    channel_.remove();

    if(closeCallback_)
        closeCallback_(shared_from_this());
}

void TcpConnection::handleError()
{
    //TODO 日志记录错误
    handleClose();
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

