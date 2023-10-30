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

#include "ref_gx.h"

#include "gx/gthread.h"
#include "gx/gmutex.h"
#include "gx/debug.h"

#include <future>


GX_NS_BEGIN

void refGThread()
{
    REF_ENUM(ThreadPriority, "Gx", "ThreadPriority");

    Class<GThread>("Gx", "GThread", "Gx thread")
            .inherit<GObject>()
            .construct<>()
            .construct<std::string>()
            .staticFunc(MetaFunction::Init, [](GAny &runnable) {
                if (runnable.isFunction()) {
                    return std::make_unique<GThread>(
                            [runnable]() {
                                try {
                                    runnable();
                                } catch (GAnyException &e) {
                                    GX_ASSERT_S(false, "GThread runnable error: %s.", e.what());
                                    LogE("GThread runnable error: %s.", e.what());
                                }
                            });
                }
                return std::make_unique<GThread>();
            }, "Constructor with runnable.")
            .staticFunc(MetaFunction::Init, [](GAny &runnable, const std::string &name) {
                if (runnable.isFunction()) {
                    return std::make_unique<GThread>(
                            [runnable]() {
                                try {
                                    runnable();
                                } catch (GAnyException &e) {
                                    GX_ASSERT_S(false, "GThread runnable error: %s.", e.what());
                                    LogE("GThread runnable error: %s.", e.what());
                                }
                            }, name);
                }
                return std::make_unique<GThread>();
            }, "Constructor with runnable and name.")
            .func("setRunnable", [](GThread &self, GAny &runnable) {
                if (runnable.isFunction()) {
                    self.setRunnable([runnable]() {
                        try {
                            runnable();
                        } catch (GAnyException &e) {
                            GX_ASSERT_S(false, "GThread runnable error: %s.", e.what());
                            LogE("GThread runnable error: %s.", e.what());
                        }
                    });
                } else if (runnable.is < std::function < void() >> ()) {
                    self.setRunnable(runnable.as < std::function < void() >> ());
                } else {
                    self.setRunnable(nullptr);
                }
            }, "Set runnable.")
            .func("start", &GThread::start, "Start thread.")
            .func("joinable", &GThread::joinable, "check thread joinable.")
            .func("join", &GThread::join, "Join thread.")
            .func("detach", &GThread::detach, "Detach thread.")
            .func("getId", [](GThread &self) {
                std::stringstream ss;
                ss << self.getId();
                return ss.str();
            }, "Get thread id.")
            .func("setName", &GThread::setName, "Set thread name.")
            .func("getName", &GThread::getName, "Get thread name.")
            .func("setPriority", &GThread::setPriority, "Set thread priority.")
            .func("getPriority", &GThread::getPriority, "Get thread priority.")
            .staticFunc("sleep", &GThread::sleep, "Sleep with second.")
            .staticFunc("mSleep", &GThread::mSleep, "Sleep with millisecond.")
            .staticFunc("nSleep", &GThread::nSleep, "Sleep with nanosecond.")
            .staticFunc("currentThreadId", []() {
                std::stringstream ss;
                ss << GThread::currentThreadId();
                return ss.str();
            }, "Get current thread id.")
            .func("hardwareConcurrency", []() {
                return (int32_t) GThread::hardwareConcurrency();
            }, "Get hardware concurrency thread count.");

    Class<GMutex>("Gx", "GMutex", "Gx thread mutex.")
            .construct<>()
            .func("lock", [](GMutex &self) {
                self.lock();
            })
            .func("tryLock", [](GMutex &self) {
                return self.try_lock();
            })
            .func("unlock", [](GMutex &self) {
                self.unlock();
            })
            .func("lock", [](GMutex &self, const GAny &func) {
                if (!func.isFunction()) {
                    return;
                }
                self.lock();
                func();
                self.unlock();
            });

    Class<GRecursiveMutex>("Gx", "GRecursiveMutex", "Gx thread recursive mutex.")
            .construct<>()
            .func("lock", [](GRecursiveMutex &self) {
                self.lock();
            })
            .func("tryLock", [](GRecursiveMutex &self) {
                return self.try_lock();
            })
            .func("unlock", [](GRecursiveMutex &self) {
                self.unlock();
            })
            .func("lock", [](GRecursiveMutex &self, const GAny &func) {
                if (!func.isFunction()) {
                    return;
                }
                self.lock();
                func();
                self.unlock();
            });

    Class<GSpinLock>("Gx", "GSpinLock", "Gx SpinLock.")
            .construct<>()
            .func("lock", [](GSpinLock &self) {
                self.lock();
            })
            .func("unlock", [](GSpinLock &self) {
                self.unlock();
            })
            .func("lock", [](GSpinLock &self, const GAny &func) {
                if (!func.isFunction()) {
                    return;
                }
                self.lock();
                func();
                self.unlock();
            });

    Class<GNoLock>("Gx", "GNoLock", "Gx NoLock.")
            .construct<>()
            .func("lock", [](GNoLock &self) {
                self.lock();
            })
            .func("tryLock", [](GNoLock &self) {
                return self.try_lock();
            })
            .func("unlock", [](GNoLock &self) {
                self.unlock();
            })
            .func("lock", [](GNoLock &self, const GAny &func) {
                if (!func.isFunction()) {
                    return;
                }
                self.lock();
                func();
                self.unlock();
            });

    Class<GRWLock>("Gx", "GRWLock", "Gx read/write lock.")
            .construct<>()
            .func("readLock", [](GRWLock &self) {
                self.readLock();
            })
            .func("readUnlock", [](GRWLock &self) {
                self.readUnlock();
            })
            .func("writeLock", [](GRWLock &self) {
                self.writeLock();
            })
            .func("writeUnlock", [](GRWLock &self) {
                self.writeUnlock();
            })
            .func("readLock", [](GRWLock &self, const GAny &func) {
                if (!func.isFunction()) {
                    return;
                }
                self.readLock();
                func();
                self.readUnlock();
            })
            .func("writeLock", [](GRWLock &self, const GAny &func) {
                if (!func.isFunction()) {
                    return;
                }
                self.writeLock();
                func();
                self.writeUnlock();
            });

    Class<GSpinRWLock>("Gx", "GSpinRWLock", "Gx spin read/write lock.")
            .construct<>()
            .func("readLock", [](GSpinRWLock &self) {
                self.readLock();
            })
            .func("readUnlock", [](GSpinRWLock &self) {
                self.readUnlock();
            })
            .func("writeLock", [](GSpinRWLock &self) {
                self.writeLock();
            })
            .func("writeUnlock", [](GSpinRWLock &self) {
                self.writeUnlock();
            })
            .func("readLock", [](GSpinRWLock &self, const GAny &func) {
                if (!func.isFunction()) {
                    return;
                }
                self.readLock();
                func();
                self.readUnlock();
            })
            .func("writeLock", [](GSpinRWLock &self, const GAny &func) {
                if (!func.isFunction()) {
                    return;
                }
                self.writeLock();
                func();
                self.writeUnlock();
            });
}

GX_NS_END
