#ifndef CLEARMOON_BASE_TYPES_H
#define CLEARMOON_BASE_TYPES_H

#include <functional>
namespace clearmoon
{

using ReadCallback = std::function<void()>;
using WriteCallback = std::function<void()>;
using ErrorCallback = std::function<void()>;
}

#endif