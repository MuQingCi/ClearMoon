#include "./CurrentThread.h"

#include <cstdio>
#include <sched.h>
#include <unistd.h>
#include <sys/syscall.h>

using namespace clearmoon;
using namespace clearmoon::Current;

thread_local int t_cacheTid = 0;
thread_local char t_tidString[32];
thread_local int t_tidStringLength = 6;
thread_local const char* t_threadName = nullptr;

void Current::cacheTid()
{
    if(t_cacheTid == 0)
    {
        t_cacheTid = static_cast<pid_t>(::syscall(SYS_gettid));
        t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cacheTid);
    }
}