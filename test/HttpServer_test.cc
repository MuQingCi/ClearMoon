#include "../net/Http/HttpServer.h"
#include "../net/Log/Logger.h"
#include "../net/Log/AsyncLogger.h"
#include "HttpCallback.h"

using namespace clearmoon::net;

#define LOG_DIR PROJECT_ROOT "/Log"

int main()
{
    AsyncLogger logger("ClearMoon", 1 * 1024, LOG_DIR);
    Logger::set_AsyncLogger(&logger);
    logger.start();

    Logger::set_GlobalLevel(LogLevel::DEBUG);

    EventLoop loop;
    InetAddress InAddr("127.0.0.1", 12345, false);
    
    TcpServer tcpServer(&loop, TcpServer::ThreadPoolInitCallback(), InAddr);
    HttpServer httpServer(&tcpServer);
    
    httpServer.setHttpCallback(httpCallback);

    tcpServer.start();
    loop.loop();
}
