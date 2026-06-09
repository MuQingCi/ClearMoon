#include "HttpServer.h"

using namespace clearmoon;
using namespace net;

void HttpServer::OnMessage(const TcpConnectionPtr& TcpConn, Buffer* buf, Timestamp recive)