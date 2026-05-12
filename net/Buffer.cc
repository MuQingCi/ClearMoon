#include "Buffer.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <string>

using namespace clearmoon;
using namespace clearmoon::net;

const char Buffer::kCRLF[] = "\r\n";

// =========== 读取操作 =========== //
int Buffer::read(char* dst, size_t len)
{
    if(len > readableBytes()) 
        len = readableBytes();
    std::copy(peek(), peek() + len, dst);
    retrieve(len);

    return len; 
}

std::string Buffer::readAsString(size_t len)
{
    if(len > readableBytes())
        len = readableBytes();
    std::string result(peek(), peek() + len);
    retrieve(len);
    return result;
}

const char* Buffer::findCRLF() const
{
    const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
    return (crlf == beginWrite() ? nullptr : crlf);
}

void Buffer::retrieve(size_t len)
{
    assert(len <= readableBytes());
    if(len < readableBytes())
        readIndex_ += len;
    else
        retrieveAll();
}

// =========== 写入操作 =========== //

int Buffer::write(const char* src, size_t len)
{
    ensureWriteable(len);
    std::copy(src, src + len, beginWrite());
    hasWritten(len);
    return static_cast<int>(len);
}


void Buffer::append(const char* src, size_t len)
{
    ensureWriteable(len);
    std::copy(src, src+len, beginWrite());
    hasWritten(len);
}


void Buffer::prepend(const void* data, size_t len)
{
    assert(len <= prependBytes());
    const char* src = static_cast<const char*>(data);
    readIndex_ -= len;
    std::copy(src, src + len, begin() + readIndex_);
}


// =========== 私有成员 =========== //
void Buffer::ensureWriteable(size_t len)
{
    if(len > writeableBytes())
    {
        // 如果可写空间加上前面预留空间(readIndex - kcheapPrepend) 小于len的话就扩容
        if(writeableBytes() + prependBytes() < len + kCheapPrepend)
        {
            buff_.resize(writeIndex_ + len);
        }
        else 
        {
            size_t readalbeSize = readableBytes();
            std::copy(begin() + readIndex_, begin() + writeIndex_, begin() + kCheapPrepend);
            readIndex_ = kCheapPrepend;
            writeIndex_ = readIndex_ + readalbeSize;
        }
    }
    assert(writeableBytes() >= len);
}

