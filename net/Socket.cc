#include "Socket.h"
#include "InetAddress.h"

#include <cassert>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace clearmoon;
using namespace clearmoon::net;

int creatNonBlockFd(int domain, int type, int protocol)
{
    int sockfd = ::socket(domain, type | SOCK_NONBLOCK | SOCK_CLOEXEC, protocol);

    assert(sockfd >= 0);

    return sockfd;
}

//---------------------构造/析构函数---------------------
Socket::Socket() noexcept : fd_(-1)
{
}

Socket::Socket(int domain, int type, int protocol) : fd_(creatNonBlockFd(domain, type, protocol))
{
}

Socket::Socket(int sockfd, bool) : fd_(sockfd)
{
}

Socket Socket::fromFd(int sockfd)
{
    return Socket(sockfd, true);
}

Socket::Socket(Socket&& other) noexcept : fd_(other.fd_)
{
    other.fd_ = -1;
}


Socket& Socket::operator=(Socket&& other) noexcept
{
    if(this != &other)
    {
        if (fd_ >= 0) {
            ::close(fd_);
        }
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}


Socket::~Socket()
{
    if(fd_ >= 0)
        ::close(fd_);
}

//---------------------成员函数---------------------
void Socket::bind(const InetAddress& addr)
{
    int ret = ::bind(fd_, addr.getAddress(), addr.getSockLen());

    assert(ret == 0);
    //Log
}


void Socket::listen(int num)
{
    int ret = ::listen(fd_, num);
    
    assert(ret == 0);
    //Log
}


Socket Socket::accept(InetAddress* peerAddr)
{
    //不管对端地址是ipv4还是ipv6，直接选择sockadr_in6来装，后续在InetAddress类中处理(是否需要还原为ipv4)
    struct sockaddr_in6 addr;
    socklen_t addrLen = sizeof(sockaddr_in6);
    
    ::memset(&addr, 0, sizeof(addr));

    int connfd = ::accept4(fd_, reinterpret_cast<sockaddr*>(&addr), &addrLen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd < 0)
    {
        return Socket();
    }

    if(peerAddr)
    {
        //绝大部分情况都直接进入if内
        if(addr.sin6_family == AF_INET6)
            peerAddr->setAddress(*reinterpret_cast<sockaddr_in6*>(&addr));
        else
            peerAddr->setAddress(*reinterpret_cast<sockaddr_in*>(&addr));
    }

    return Socket::fromFd(connfd);
}

//设置socket相关选项
void Socket::setReuseAddress(bool on)
{
    int optval = on ? 1 : 0;

    int ret = ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    assert(ret == 0);
}

void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;

    int ret = ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    assert(ret == 0);
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;

    int ret = ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));

    assert(ret == 0);
}

void Socket::setNonBlocking(bool on)
{
    int flag = ::fcntl(fd_, F_GETFL, 0);

    if(on)
        flag |= O_NONBLOCK;
    else 
        flag &= ~O_NONBLOCK;

    int ret = ::fcntl(fd_, F_SETFL, flag);

    assert(ret == 0);
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on? 1 : 0;

    int ret = ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));

    assert(ret == 0);
}

void Socket::shutdownWrite()
{
    int ret = ::shutdown(fd_, SHUT_WR);
}

