#include "Channel.h"

#include "EventLoop.h"
#include <cassert>
#include <sys/epoll.h>

using namespace clearmoon;
using namespace clearmoon::net;

Channel::Channel(EventLoop* loop, int fd) : loop_(loop),
                                            fd_(fd),
                                            events_(0),
                                            revents_(0), 
                                            added_(false),
                                            index_(0)
{
}

Channel::~Channel()
{
    remove();
}

void Channel::handleEvent()
{
    //HUP
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if(errorCallback_) errorCallback_();
    }

    //Error
    if((revents_ & EPOLLERR ))
    {
        if(errorCallback_) errorCallback_();
    }

    //Read
    if(revents_ & (EPOLLIN | EPOLLRDHUP | EPOLLPRI) )
    {
        if(readCallback_) readCallback_();
    }

    //Write
    if(revents_ & EPOLLOUT)
    {
        if(writeCallback_) writeCallback_();
    }
}

void Channel::update()
{
    loop_->assertInLoopThread();

    if(loop_)
    {
        loop_->updateChannel(this);
        added_ = true;
    }
}

void Channel::remove()
{
    loop_->assertInLoopThread();

    if(added_)
    {
        loop_->removeChannel(this);
        added_ = false;
    }
}