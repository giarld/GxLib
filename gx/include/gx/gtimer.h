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

#ifndef GX_GTIMER_H
#define GX_GTIMER_H

#include "gobject.h"

#include "gtime.h"
#include "gx/gmutex.h"

#include <memory>
#include <functional>
#include <queue>
#include <vector>


GX_NS_BEGIN

using GTimerEvent = std::function<void()>;

/**
 * @class GTimerTask
 */
class GX_API GTimerTask
{
private:
    explicit GTimerTask(GTimerEvent event, int64_t delay, int64_t interval);

public:
    void cancel();

private:
    friend class GTimer;

    friend class GTimerScheduler;

    GTimerEvent mEvent;
    int64_t mInterval;
    GTime mTime;
    std::atomic_bool mValid;
    bool mOneShot {false};
};

class GX_API GTimerScheduler
{
public:
    using GTimerTaskPtr = std::shared_ptr<GTimerTask>;

private:
    explicit GTimerScheduler(std::string name);

public:
    static std::shared_ptr<GTimerScheduler> create(std::string name);

    static void makeGlobal(const std::shared_ptr<GTimerScheduler> &obj);

    static std::shared_ptr<GTimerScheduler> global();

    ~GTimerScheduler();

    GTimerScheduler(const GTimerScheduler &) = delete;

    GTimerScheduler(GTimerScheduler &&) noexcept = delete;

    GTimerScheduler &operator=(const GTimerScheduler &) = delete;

    GTimerScheduler &operator=(GTimerScheduler &&) noexcept = delete;

public:
    bool run();

    void loop();

    /**
     * @brief Before using the loop() driver, you must first call start()
     */
    void start();

    void stop();

    bool isRunning() const;

    /**
     * @brief Push a one-time scheduled task
     * @param event
     * @param delay
     */
    GTimerTaskPtr post(GTimerEvent event, int64_t delay);

private:
    GTimerTaskPtr addTask(GTimerEvent event, int64_t delay, int64_t interval, bool oneShot);

private:
    friend class GTimer;

    std::string mName;

    mutable GMutex mLock;
    std::condition_variable mTaskCond;
    std::atomic<bool> mIsRunning{false};

    using CompFunc = std::function<bool(const GTimerTaskPtr &, const GTimerTaskPtr &)>;

    std::priority_queue<GTimerTaskPtr, std::vector<GTimerTaskPtr>, CompFunc> mTaskQueue{
            [](const GTimerTaskPtr &lhs, const GTimerTaskPtr &rhs) {
                return lhs->mTime > rhs->mTime;
            }
    };
};

using GTimerSchedulerPtr = std::shared_ptr<GTimerScheduler>;


class GX_API GTimer : public GObject
{
public:
    explicit GTimer(const std::shared_ptr<GTimerScheduler> &scheduler = nullptr, bool oneShot = false);

    ~GTimer() override;

    GTimer(const GTimer &) = delete;

    GTimer(GTimer &&rh) noexcept;

    GTimer &operator=(const GTimer &) = delete;

    GTimer &operator=(GTimer &&rh) noexcept;

public:
    virtual void timeout();

    void timerEvent(GTimerEvent event);

    void setOneShot(bool oneShot = true);

    void start(int64_t interval);

    void start(int64_t delay, int64_t interval);

    void stop();

private:
    std::weak_ptr<GTimerScheduler> mScheduler;
    GTimerEvent mEvent;
    std::weak_ptr<GTimerTask> mTask;
    bool mOneShot{false};
};

GX_NS_END

#endif //GX_GTIMER_H
