/*
 * Copyright (c) 2022 Gxin
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

#ifndef GX_TASK_SYSTEM_H
#define GX_TASK_SYSTEM_H

#include "gx/base.h"
#include "gx/gglobal.h"

#include "gthread.h"
#include "gx/gmutex.h"
#include "gtimer.h"

#include <atomic>
#include <future>
#include <functional>
#include <list>
#include <memory>
#include <vector>


GX_NS_BEGIN

/**
 * @class TaskSystem
 * @brief Multi threaded task system (thread pool)
 */
class GX_API TaskSystem final
{
public:
    template<class T>
    class Task
    {
    private:
        Task(std::future<T> &&future, const std::shared_ptr<std::atomic<bool>>& active)
                : mFuture(std::make_shared<std::future<T>>(std::move(future))), mActive(active)
        {}

    public:
        explicit Task()
        {}

        Task(const Task &other) = delete;

        Task(Task &&other) noexcept
        {
            mFuture = other.mFuture;
            mActive = other.mActive;
            other.mFuture = nullptr;
        }

        Task &operator=(const Task &other) = delete;

        Task &operator=(Task &&other) noexcept
        {
            if (this != &other) {
                mFuture = other.mFuture;
                mActive = other.mActive;

                other.mFuture = nullptr;
                other.mActive = nullptr;
            }
            return *this;
        }

    public:
        T get()
        {
            return mFuture->get();
        }

        void wait()
        {
            mFuture->wait();
        }

        bool waitFor(int64_t ms)
        {
            auto status = mFuture->wait_for(std::chrono::milliseconds(ms));
            return status != std::future_status::timeout;
        }

        void cancel()
        {
            auto active = mActive.lock();
            if (active) {
                active->store(false);
                auto timer = mObserverTimer;
                if (timer) {
                    timer->stop();
                    timer->timerEvent(nullptr);
                }
            }
        }

        template<typename Action>
        void subscribe(Action action, const GTimerSchedulerPtr &scheduler = nullptr)
        {
            auto timer = mObserverTimer = std::make_shared<GTimer>(scheduler);
            auto future = mFuture;
            auto active = mActive;
            mObserverTimer->timerEvent([future, active, timer, action]() {
                while (true) {
                    if (!future->valid()) {
                        break;
                    }
                    auto activeFlag = active.lock();
                    if (!activeFlag || !activeFlag->load()) {
                        break;
                    }
                    auto status = future->wait_for(std::chrono::milliseconds(0));
                    if (status != std::future_status::timeout) {
                        action(future->get());
                        break;
                    }
                    return;
                }
                timer->stop();
                timer->timerEvent(nullptr);
            });
            mObserverTimer->start(0, 10);
        }

        bool isValid() const
        {
            if (!mFuture->valid()) {
                return false;
            }
            auto active = mActive.lock();
            if (active) {
                return active->load();
            }
            return false;
        }

    private:
        friend class TaskSystem;

        std::shared_ptr<std::future<T>> mFuture;
        std::weak_ptr<std::atomic<bool>> mActive;

        std::shared_ptr<GTimer> mObserverTimer;
    };

public:
    /**
     * @brief Construct Task System
     * @param threadCount    Number of threads, when using the default value of 0, the number of allocated threads is the number of CPU cores
     */
    explicit TaskSystem(uint32_t threadCount = 0, std::string name = "TaskSystem");

    ~TaskSystem();

    TaskSystem(const TaskSystem &) = delete;

    TaskSystem(TaskSystem &&) noexcept = delete;

    TaskSystem &operator=(const TaskSystem &) = delete;

    TaskSystem &operator=(TaskSystem &&) noexcept = delete;

public:
    uint32_t threadCount() const;

    /**
     * @brief Start Task System
     */
    void start();

    /**
     * @brief Stop and wait for all tasks to end
     */
    void stopAndWait();

    /**
     * @brief Stop, do not wait for the task to end, all unfinished tasks will be cancelled
     */
    void stop();

    bool isRunning() const;

    void setThreadPriority(ThreadPriority priority);

    ThreadPriority getThreadPriority() const;

    template<typename F, typename... A,
            typename = std::enable_if_t<std::is_void_v<std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>>>>
    Task<bool> submit(const F &taskFunc, const A &&... args)
    {
        auto promise = std::make_shared<std::promise<bool>>();
        auto future = promise->get_future();
        auto taskRef = pushTask(
                [taskFunc, args..., promise] {
                    taskFunc(args...);
                    promise->set_value(true);
                });
        return {std::move(future), taskRef.active};
    }

    template<typename F, typename... A,
            typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>,
            typename = std::enable_if_t<!std::is_void_v<R>>>
    Task<R> submit(const F &taskFunc, const A &&... args)
    {
        auto promise = std::make_shared<std::promise<R>>();
        auto future = promise->get_future();
        auto taskRef = pushTask([taskFunc, args..., promise] { promise->set_value(taskFunc(args...)); });
        return {std::move(future), taskRef.active};
    }

    template<typename F, typename... A,
            typename = std::enable_if_t<std::is_void_v<std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>>>>
    Task<bool> submitFront(const F &taskFunc, const A &&... args)
    {
        auto promise = std::make_shared<std::promise<bool>>();
        auto future = promise->get_future();
        auto taskRef = pushTaskFront(
                [taskFunc, args..., promise] {
                    taskFunc(args...);
                    promise->set_value(true);
                });
        return {std::move(future), taskRef.active};
    }

    template<typename F, typename... A,
            typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>,
            typename = std::enable_if_t<!std::is_void_v<R>>>
    Task<R> submitFront(const F &taskFunc, const A &&... args)
    {
        auto promise = std::make_shared<std::promise<R>>();
        auto future = promise->get_future();
        auto taskRef = pushTaskFront([taskFunc, args..., promise] { promise->set_value(taskFunc(args...)); });
        return {std::move(future), taskRef.active};
    }

    uint64_t waitingTaskCount() const;

private:
    using TaskFunc = std::function<void()>;
    struct TaskFuncRef
    {
        TaskFunc func;
        std::shared_ptr<std::atomic<bool>> active;
    };

    TaskFuncRef pushTask(const TaskFunc &task);

    TaskFuncRef pushTaskFront(const TaskFunc &task);

    void clearTask();

private:
    uint32_t mThreadCount;
    std::string mName;
    ThreadPriority mPriority = ThreadPriority::Normal;

    std::vector<std::unique_ptr<GThread>> mThreads;
    std::list<TaskFuncRef> mTaskQueue;

    mutable GMutex mLock;
    std::condition_variable mTaskCond;
    std::atomic<bool> mIsRunning{false};
};

GX_NS_END

#endif //GX_TASK_SYSTEM_H
