#include "Channel.h"
#include "EventLoop.h"

using namespace clearmoon;
using namespace clearmoon::net;

Channel::Channel(EventLoop* loop, int fd) : loop_(loop),
                                            fd_(fd),
{
}

Channel::~Channel()
{
}



void Channel::update()
{
    if(loop_)
    {
        loop_->updateChannel(this);
    }
}

void Channel::handleEvent()
{
    
}