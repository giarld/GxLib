/*
 * Copyright (c) 2020 Gxin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "gx/gthread.h"

#include <gx/debug.h>

#include <chrono>
#include <memory.h>
#include <sstream>
#include <algorithm>


#if GX_PLATFORM_WINDOWS && GX_CRT_MSVC

#include <Windows.h>

const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push, 8)

typedef struct _THREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;

#pragma pack(pop)

void SetThreadName(DWORD dwThreadID, const char *threadName)
{
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable: 6320 6322)
    __try {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR *) &info);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {}
#pragma warning(pop)
}


void setNativeThreadPriority(void *thread, gx::ThreadPriority priority)
{
    using namespace gx;

    int32_t winPriority;
    switch (priority) {
        case ThreadPriority::Lowest:
            winPriority = THREAD_PRIORITY_LOWEST;
            break;
        case ThreadPriority::BelowNormal:
            winPriority = THREAD_PRIORITY_BELOW_NORMAL;
            break;
        case ThreadPriority::Normal:
            winPriority = THREAD_PRIORITY_NORMAL;
            break;
        case ThreadPriority::AboveNormal:
            winPriority = THREAD_PRIORITY_ABOVE_NORMAL;
            break;
        case ThreadPriority::Highest:
            winPriority = THREAD_PRIORITY_HIGHEST;
            break;
        default:
            winPriority = THREAD_PRIORITY_NORMAL;
            break;
    }

    GX_ASSERT(::SetThreadPriority((HANDLE) thread, winPriority));
}

void setNativeThreadName(void *thread, const std::string &name)
{
    DWORD tId = ::GetThreadId((HANDLE) thread);
    SetThreadName(tId, name.c_str());
}

#endif

#if (GX_PLATFORM_WINDOWS && GX_CRT_MINGW) || GX_PLATFORM_POSIX

#if GX_PLATFORM_EMSCRIPTEN

void setNativeThreadPriority(pthread_t thread, gx::ThreadPriority priority)
{}

void setNativeThreadName(pthread_t thread, const std::string &name)
{}

#else
#define GX_USE_PTHREAD 1

#include <pthread.h>

void setNativeThreadPriority(pthread_t thread, gx::ThreadPriority priority)
{
    using namespace gx;

    int32_t ptPriority;
    switch (priority) {
        case ThreadPriority::Highest:
            ptPriority = 30;
        case ThreadPriority::AboveNormal:
            ptPriority = 25;
        case ThreadPriority::Normal:
            ptPriority = 15;
        case ThreadPriority::BelowNormal:
            ptPriority = 5;
        case ThreadPriority::Lowest:
            ptPriority = 1;
        default:
            ptPriority = 0;
    }
    struct sched_param sched{};

    int32_t policy = SCHED_RR;
    pthread_getschedparam(thread, &policy, &sched);
    sched.sched_priority = ptPriority;
    pthread_setschedparam(thread, policy, &sched);
}

#if GX_PLATFORM_OSX || GX_PLATFORM_IOS
void setNativeThreadName(pthread_t thread, const std::string &name)
{
}

void setNativeThreadName(const std::string &name)
{
    try {
        pthread_setname_np(name.c_str());
    } catch (std::exception &e){
    }
}
#else
void setNativeThreadName(pthread_t thread, const std::string &name)
{
    try {
        pthread_setname_np(thread, name.c_str());
    } catch (std::exception &e){
    }
}
#endif
#endif

#endif


GX_NS_BEGIN


GThread::GThread(std::string name)
        : mName(std::move(name))
{}

GThread::GThread(const Runnable &runnable, std::string name)
        : mName(std::move(name))
{
    mRunnable = runnable;
    start();
}

GThread::~GThread()
{
    join();
}

GThread &GThread::setRunnable(const Runnable &runnable)
{
    if (!joinable()) {
        mRunnable = runnable;
    }
    return *this;
}

bool GThread::joinable() const
{
    return mThread.joinable();
}

void GThread::start()
{
    if (joinable()) {
        return;
    }

    if (mRunnable) {
        mThread = std::thread([this] {
#if GX_PLATFORM_OSX || GX_PLATFORM_IOS
            setNativeThreadName(mName);
#endif
            mRunnable();
        });
    } else {
        mThread = std::thread([this] {
#if GX_PLATFORM_OSX || GX_PLATFORM_IOS
            setNativeThreadName(mName);
#endif
            run();
        });
    }
    setNativeThreadName(mThread.native_handle(), mName);
    if (mPriority != ThreadPriority::Normal) {
        setNativeThreadPriority(mThread.native_handle(), mPriority);
    }
}

void GThread::join()
{
    if (mThread.joinable()) {
        mThread.join();
    }
}

void GThread::detach()
{
    if (mThread.joinable()) {
        mThread.detach();
    }
}

GThread::ThreadIdType GThread::getId()
{
    return mThread.get_id();
}

void GThread::setName(const std::string &name)
{
    mName = name;

    if (mThread.joinable()) {
        setNativeThreadName(mThread.native_handle(), name);
    }
}

std::string GThread::getName() const
{
    return mName;
}

void GThread::setPriority(ThreadPriority priority)
{
    if (priority != mPriority) {
        mPriority = priority;

        if (mThread.joinable()) {
            setNativeThreadPriority(mThread.native_handle(), priority);
        }
    }
}

ThreadPriority GThread::getPriority() const
{
    return mPriority;
}

std::string GThread::toString() const
{
    std::stringstream ss;
    ss << "<" << "GThread" << ", name: " << mName << ">";
    return ss.str();
}

void GThread::run()
{

}

void GThread::sleep(int64_t seconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

void GThread::mSleep(int64_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void GThread::nSleep(int64_t nanos)
{
    std::this_thread::sleep_for(std::chrono::nanoseconds(nanos));
}

GThread::ThreadIdType GThread::currentThreadId()
{
    return std::this_thread::get_id();
}

uint32_t GThread::hardwareConcurrency()
{
    return std::thread::hardware_concurrency();
}

GX_NS_END