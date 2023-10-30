/*
 * Copyright (c) 2023 Gxin
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

#include "gx/gtimer.h"

#include "gx/debug.h"

#if GX_PLATFORM_WINDOWS

#include <windows.h>

#pragma comment(lib, "winmm.lib")

#endif

GX_NS_BEGIN

#if GX_PLATFORM_WINDOWS

void timeBeginPeriod(int32_t t)
{
    ::timeBeginPeriod(t);
}

void timeEndPeriod(int32_t t)
{
    ::timeEndPeriod(t);
}

#else

inline void timeBeginPeriod(int32_t t)
{
}

inline void timeEndPeriod(int32_t t)
{
}

#endif

static std::weak_ptr<GTimerScheduler> sGlobalScheduler;

GTimerTask::GTimerTask(GTimerEvent event, int64_t delay, int64_t interval)
        : mEvent(std::move(event)),
          mInterval(interval),
          mTime(GTime::currentSteadyTime()),
          mValid(true)
{
    mTime.addMilliSecs(delay);
}

void GTimerTask::cancel()
{
    if (mValid.exchange(false)) {
        mEvent = nullptr;
    }
}

GTimerScheduler::GTimerScheduler(std::string name)
        : mName(std::move(name))
{
}

std::shared_ptr<GTimerScheduler> GTimerScheduler::create(std::string name)
{
    auto obj = std::shared_ptr<GTimerScheduler>(GX_NEW(GTimerScheduler, std::move(name)));
    if (sGlobalScheduler.expired()) {
        sGlobalScheduler = obj;
    }
    return obj;
}

void GTimerScheduler::makeGlobal(const std::shared_ptr<GTimerScheduler> &obj)
{
    if (obj) {
        sGlobalScheduler = obj;
    }
}

std::shared_ptr<GTimerScheduler> GTimerScheduler::global()
{
    return sGlobalScheduler.lock();
}

GTimerScheduler::~GTimerScheduler() = default;

bool GTimerScheduler::run()
{
    mIsRunning.store(true);

    while (true) {
        GTimerTaskPtr task;
        {
            GLocker<GMutex> locker(mLock);
            mTaskCond.wait(locker, [this] {
                return !mIsRunning.load() || !mTaskQueue.empty();
            });
            if (!mIsRunning.load()) {
                break;
            }
            task = mTaskQueue.top();
            GTime now = GTime::currentSteadyTime();
            if (task->mValid.load() && task->mTime > now) {
                GTime::TimeType timeDiff = task->mTime.milliSecsTo(now);
                if (timeDiff > 0) {
                    task.reset();
                    timeBeginPeriod(1);
                    mTaskCond.wait_for(locker, std::chrono::milliseconds(timeDiff));
                    timeEndPeriod(1);
                    continue;
                }
            }
            mTaskQueue.pop();
        }
        GTimerEvent event;
        if (task && task->mValid.load() && (event = task->mEvent)) {
            event();
            if (!task->mOneShot) {
                task->mTime.update();
                task->mTime.addMilliSecs(task->mInterval);
                {
                    GLocker<GMutex> locker(mLock);
                    mTaskQueue.push(task);
                }
            }
        }
    }

    return true;
}

void GTimerScheduler::loop()
{
    if (!mIsRunning.load()) {
        return;
    }

    GTimerTaskPtr task;
    {
        GLocker<GMutex> locker(mLock);
        if (mTaskQueue.empty()) {
            return;
        }
        GTime now = GTime::currentSteadyTime();
        task = mTaskQueue.top();
        if (task->mValid.load() && task->mTime > now) {
            return;
        }
        mTaskQueue.pop();
    }
    GTimerEvent event;
    if (task && task->mValid.load() && (event = task->mEvent)) {
        event();
        if (!task->mOneShot && task->mValid.load()) {
            task->mTime.update();
            task->mTime.addMilliSecs(task->mInterval);
            {
                GLocker<GMutex> locker(mLock);
                mTaskQueue.push(task);
            }
        }
    }
}

void GTimerScheduler::start()
{
    mIsRunning.store(true);
}

void GTimerScheduler::stop()
{
    if (mIsRunning.exchange(false)) {
        GLockerGuard locker(mLock);
        while (!mTaskQueue.empty()) {
            mTaskQueue.pop();
        }
        mTaskCond.notify_all();
    }
}

bool GTimerScheduler::isRunning() const
{
    return mIsRunning.load();
}

GTimerScheduler::GTimerTaskPtr GTimerScheduler::post(GTimerEvent event, int64_t delay)
{
    return addTask(std::move(event), delay, 0, true);
}

GTimerScheduler::GTimerTaskPtr GTimerScheduler::addTask(GTimerEvent event,
                                                        int64_t delay,
                                                        int64_t interval,
                                                        bool oneShot)
{
    auto task = std::shared_ptr<GTimerTask>(new GTimerTask(std::move(event), delay, interval));
    task->mOneShot = oneShot;
    GLockerGuard locker(mLock);
    mTaskQueue.push(task);
    mTaskCond.notify_one();
    return task;
}


GTimer::GTimer(const std::shared_ptr<GTimerScheduler> &scheduler, bool oneShot)
        : mScheduler(scheduler),
          mOneShot(oneShot)
{
    if (mScheduler.expired()) {
        mScheduler = sGlobalScheduler;
    }
    mEvent = [this] { timeout(); };
}

GTimer::~GTimer()
{
    stop();
}

GTimer::GTimer(GTimer &&rh) noexcept
        : mScheduler(std::move(rh.mScheduler)),
          mEvent(std::move(rh.mEvent)),
          mTask(std::move(rh.mTask))
{
}

GTimer &GTimer::operator=(GTimer &&rh) noexcept
{
    if (this != &rh) {
        mScheduler = std::move(rh.mScheduler);
        mEvent = std::move(rh.mEvent);
        mTask = std::move(rh.mTask);
    }
    return *this;
}

void GTimer::timeout()
{
}

void GTimer::timerEvent(GTimerEvent event)
{
    if (!event) {
        mEvent = [this] { timeout(); };
    } else {
        mEvent = std::move(event);
    }
    if (mTask.expired()) {
        return;
    }
    auto taskPtr = mTask.lock();
    if (taskPtr && taskPtr->mValid.load()) {
        taskPtr->mEvent = mEvent;
    }
}

void GTimer::setOneShot(bool oneShot)
{
    mOneShot = oneShot;
    if (mTask.expired()) {
        return;
    }
    auto taskPtr = mTask.lock();
    if (taskPtr && taskPtr->mValid.load()) {
        taskPtr->mOneShot = mOneShot;
    }
}

void GTimer::start(int64_t interval)
{
    start(interval, interval);
}

void GTimer::start(int64_t delay, int64_t interval)
{
    if (!mEvent) {
        return;
    }
    stop();

    GX_ASSERT_S(!mScheduler.expired(), "GTimer: Invalid scheduler");
    auto scheduler = mScheduler.lock();
    if (scheduler) {
        mTask = scheduler->addTask(mEvent, delay, interval, mOneShot);
    }
}

void GTimer::stop()
{
    if (mTask.expired()) {
        return;
    }
    auto taskPtr = mTask.lock();
    if (taskPtr) {
        taskPtr->cancel();
        mTask.reset();
    }
}

GX_NS_END