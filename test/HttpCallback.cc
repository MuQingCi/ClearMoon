#include "HttpCallback.h"
#include "../net/Http/HttpResponse.h"
#include "../net/Http/HttpRequest.h"

#include <fstream>
#include <sstream>
#include <string>

namespace clearmoon {
namespace net {

void httpCallback(const TcpConnectionPtr& conn, HttpRequest& req, HttpResponse& resp)
{
    if (req.getPath() == "/video")
    {
        // 读取 video.html 文件内容作为响应体
        std::string htmlPath = std::string(PROJECT_ROOT) + "/resource/video.html";
        std::ifstream file(htmlPath);
        if (file.is_open())
        {
            std::ostringstream ss;
            ss << file.rdbuf();
            resp.setStatus(HttpStatusCode::k200Ok);
            resp.setBody(ss.str());
            resp.setContentType("text/html; charset=utf-8");
        }
        else
        {
            resp.setStatus(HttpStatusCode::k404NotFound);
            resp.setBody("HTML file not found\n");
            resp.setContentType("text/plain");
        }
    }
    else if (req.getPath() == "/video.mp4")
    {
        // 使用 sendfile 零拷贝发送视频文件
        std::string videoPath = std::string(PROJECT_ROOT) + "/resource/video.mp4";
        if (resp.setFilePath(videoPath, "video/mp4"))
        {
            resp.setStatus(HttpStatusCode::k200Ok);
        }
        else
        {
            resp.setStatus(HttpStatusCode::k404NotFound);
            resp.setBody("Video file not found\n");
            resp.setContentType("text/plain");
        }
    }
    else if (req.getPath() == "/JTH.jpg")
    {
        std::string PicturePath = std::string(PROJECT_ROOT) + "/resource/JTH.jpg";
        if (resp.setFilePath(PicturePath, "image/jpg"))
        {
            resp.setStatus(HttpStatusCode::k200Ok);
        }
        else
        {
            resp.setStatus(HttpStatusCode::k404NotFound);
            resp.setBody("Picture file not found\n");
            resp.setContentType("text/plain");
        }
    }
    else if (req.getPath() == "/hello")
    {
        resp.setStatus(HttpStatusCode::k200Ok);
        resp.setBody("Hello!");
        resp.setContentType("text/plain");
    }
    else
    {
        resp.setStatus(HttpStatusCode::k404NotFound);
        resp.setBody("Not Found\n");
    }
}

} // namespace net
} // namespace clearmoon