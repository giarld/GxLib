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

#include <utility>

#include "gx/task_system.h"

#include "gx/gthread.h"
#include "gx/debug.h"

#include <sstream>


GX_NS_BEGIN

TaskSystem::TaskSystem(uint32_t threadCount, std::string name)
        : mThreadCount(threadCount),
          mName(std::move(name))
{
    if (mThreadCount == 0 || mThreadCount > GThread::hardwareConcurrency()) {
        mThreadCount = GThread::hardwareConcurrency();
    }
}

TaskSystem::~TaskSystem()
{
    stop();
}

uint32_t TaskSystem::threadCount() const
{
    return mThreadCount;
}

void TaskSystem::start()
{
    if (mIsRunning.load()) {
        return;
    }

    mIsRunning.store(true);

    mThreads.resize(mThreadCount);

    for (uint32_t i = 0; i < mThreadCount; i++) {
        std::stringstream tNameS;
        tNameS << mName << "_" << i;

        mThreads[i] = std::make_unique<GThread>([this] {
            TaskFuncRef taskFuncRef;
            while (true) {
                {
                    GLocker<GMutex> locker(mLock);
                    mTaskCond.wait(locker, [this] {
                        return !mIsRunning.load() || !mTaskQueue.empty();
                    });
                    if (!mIsRunning.load() && mTaskQueue.empty()) {
                        break;
                    }
                    taskFuncRef = mTaskQueue.front();
                    mTaskQueue.pop_front();
                }
                if (taskFuncRef.active->load()) {
                    GX_ASSERT(taskFuncRef.func);
                    taskFuncRef.func();
                }
            }
        }, tNameS.str());
        mThreads[i]->setPriority(mPriority);
    }
}

void TaskSystem::stopAndWait()
{
    if (!mIsRunning.load()) {
        return;
    }

    {
        GLockerGuard locker(mLock);
        mIsRunning.store(false);
        mTaskCond.notify_all();
    }
    for (auto &thread: mThreads) {
        thread->join();
    }
}

void TaskSystem::stop()
{
    if (!mIsRunning.load()) {
        return;
    }
    clearTask();
    stopAndWait();
}

bool TaskSystem::isRunning() const
{
    return mIsRunning.load();
}

void TaskSystem::setThreadPriority(ThreadPriority priority)
{
    mPriority = priority;
    if (!mThreads.empty()) {
        for (auto &thread: mThreads) {
            thread->setPriority(priority);
        }
    }
}

ThreadPriority TaskSystem::getThreadPriority() const
{
    return mPriority;
}

uint64_t TaskSystem::waitingTaskCount() const
{
    GLockerGuard locker(mLock);
    return (uint64_t) mTaskQueue.size();
}

TaskSystem::TaskFuncRef TaskSystem::pushTask(const TaskSystem::TaskFunc &task)
{
    GLockerGuard locker(mLock);
    TaskFuncRef taskRef{task, std::make_shared<std::atomic<bool>>(true)};
    mTaskQueue.push_back(taskRef);
    mTaskCond.notify_one();
    return taskRef;
}

TaskSystem::TaskFuncRef TaskSystem::pushTaskFront(const TaskSystem::TaskFunc &task)
{
    GLockerGuard locker(mLock);
    TaskFuncRef taskRef{task, std::make_shared<std::atomic<bool>>(true)};
    mTaskQueue.push_front(taskRef);
    mTaskCond.notify_one();
    return taskRef;
}

void TaskSystem::clearTask()
{
    GLockerGuard locker(mLock);
    mTaskQueue.clear();
}

GX_NS_END
