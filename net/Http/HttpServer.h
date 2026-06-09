#ifndef CLEARMOON_NET_HTTP_HTTPSERVER_H
#define CLEARMOON_NET_HTTP_HTTPSERVER_H

#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#include "Callbacks.h"

#include <functional>
#include <map>
#include <string>

namespace clearmoon 
{
namespace net 
{

class HttpServer : noncopyable
{
public:
using HttpCallback = std::function<void(const TcpConnectionPtr&, HttpRequest&, HttpResponse&)>;

    void OnMessage(const TcpConnectionPtr& TcpConn, Buffer* buf, Timestamp ts);

    void setHtppCallback(const HttpCallback& cb)
    {
        cb_ = cb;
    }

private:
    std::map<std::string, HttpContext> context_;
    HttpCallback cb_;
};
}
}

#endif