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

#ifndef GX_GTHREAD_H
#define GX_GTHREAD_H

#include "gobject.h"

#include "gx/enum.h"

#include <thread>
#include <functional>
#include <atomic>
#include <string>


GX_NS_BEGIN

DEF_ENUM_5(ThreadPriority, 0,
           Normal,
           AboveNormal,
           BelowNormal,
           Highest,
           Lowest
)

/**
 * @class GThread
 * @brief Thread class provides convenient methods for creating and using threads,
 * as well as methods for setting thread names and thread priorities.
 */
class GX_API GThread : public GObject
{
public:
    using ThreadIdType = std::thread::id;

    using Runnable = std::function<void()>;

public:
    GThread(std::string name = "GThread");

    explicit GThread(const Runnable &runnable, std::string name = "GThread");

    GThread(const GThread &b) = delete;

    GThread(GThread &&b) = delete;

    virtual ~GThread();

    GThread &operator=(const GThread &b) = delete;

    GThread &operator=(GThread &&b) noexcept = delete;

    /**
     * @brief Set Runnable, which will override the implementation of the run() method
     * @param runnable
     * @return *this
     */
    GThread &setRunnable(const Runnable &runnable);

    bool joinable() const;

    void start();

    void join();

    void detach();

    ThreadIdType getId();

    void setName(const std::string &name);

    std::string getName() const;

    void setPriority(ThreadPriority priority);

    ThreadPriority getPriority() const;

public:
    std::string toString() const override;

protected:
    /**
     * @brief Thread executes this method when Runnable is not set
     */
    virtual void run();

public: //static
    /**
     * @brief Sleep Current Thread
     * @param seconds Unit seconds
     */
    static void sleep(int64_t seconds);

    /**
     * @brief Sleep Current Thread
     * @param ms Unit millisecond
     */
    static void mSleep(int64_t ms);

    static void nSleep(int64_t nanos);

    static ThreadIdType currentThreadId();

    /**
     * @brief Get the number of physical threads
     * @return
     */
    static uint32_t hardwareConcurrency();

private:
    std::thread mThread;
    Runnable mRunnable;

    std::string mName;
    ThreadPriority mPriority = ThreadPriority::Normal;
};

GX_NS_END

#endif //GX_GTHREAD_H
