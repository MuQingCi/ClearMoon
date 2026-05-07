#ifndef CLEARMOON_NET_CHANNEL_H
#define CLEARMOON_NET_CHANNEL_H


#include "../base/Types.h"

#include <sys/epoll.h>
namespace clearmoon
{
namespace net 
{

class EventLoop;
struct epoll_event;

class Channel
{
public:
    Channel(EventLoop* loop, int fd);
    ~Channel();

    int getFd() const { return fd_; }
    int getRevent() const { return revents_; }

    void setWriteCallback(WriteCallback cb) { writeCallback_ = cb; }
    void setReadCallback(ReadCallback cb) { readCallback_ = cb; }
    void setErrorCallback(ErrorCallback cb) { errorCallback_ = cb; }

    void enableReading(){ event_ |= EPOLLIN; update();}
    void enableWriting(){ event_ |= EPOLLOUT; update();}

    void update();

    void handleEvent();
private:
    EventLoop* loop_;

    int fd_;
    int revents_;
    int event_;

    WriteCallback writeCallback_;
    ReadCallback readCallback_;
    ErrorCallback errorCallback_;
    
};

}

}


#endif