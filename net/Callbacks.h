#ifndef CLEARMOON_NET_CALLBACKS_H
#define CLEARMOON_NET_CALLBACKS_H

#include <functional>

namespace clearmoon
{
namespace net 
{
using ReadCallback = std::function<void()>;
using WriteCallback = std::function<void()>;
using ErrorCallback = std::function<void()>;

}

}

#endif