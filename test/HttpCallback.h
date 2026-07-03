#ifndef CLEARMOON_TEST_HTTPCALLBACK_H
#define CLEARMOON_TEST_HTTPCALLBACK_H

#include "../net/Http/HttpRequest.h"
#include "../net/Http/HttpResponse.h"
#include "../net/TcpConnection.h"

namespace clearmoon {
namespace net {

void httpCallback(const TcpConnectionPtr& conn, HttpRequest& req, HttpResponse& resp);

} // namespace net
} // namespace clearmoon

#endif