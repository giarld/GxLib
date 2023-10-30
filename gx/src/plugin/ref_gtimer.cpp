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

#include "ref_gx.h"

#include <gx/gtimer.h>


GX_NS_BEGIN

void refGTimer()
{
    Class<GTimerTask>("Gx", "GTimerTask", "Gx timer task.")
            .func("cancel", &GTimerTask::cancel);

    Class<GTimerScheduler>("Gx", "GTimerScheduler", "Gx timer scheduler.")
            .staticFunc("create", &GTimerScheduler::create)
            .staticFunc("makeGlobal", &GTimerScheduler::makeGlobal)
            .staticFunc("global", &GTimerScheduler::global)
            .func("run", &GTimerScheduler::run)
            .func("loop", &GTimerScheduler::loop)
            .func("start", &GTimerScheduler::start)
            .func("stop", &GTimerScheduler::stop)
            .func("isRunning", &GTimerScheduler::isRunning)
            .func("post", [](GTimerScheduler &self, const GAny& event, int64_t delay)->GTimerScheduler::GTimerTaskPtr {
                if (event.isFunction()) {
                    return self.post([event]() {
                        event();
                    }, delay);
                }
                return nullptr;
            });

    Class<GTimer>("Gx", "GTimer", "Gx timer.")
            .construct<>()
            .construct<GTimerSchedulerPtr>()
            .construct<GTimerSchedulerPtr, bool>()
            .inherit<GObject>()
            .func("timerEvent", [](GTimer &self, const GAny &eventFunc) {
                if (!eventFunc.isFunction()) {
                    throw GAnyException("Arg1 is not a function.");
                }
                self.timerEvent([eventFunc]() {
                    try {
                        eventFunc();
                    } catch (GAnyException &e) {
                        std::stringstream ss;
                        ss << "GTimer timerEvent call exception: " << e.what();
                        throw GAnyException(ss.str());
                    }
                });
            })
            .func("setOneShot", [](GTimer &self, bool oneShot) {
                self.setOneShot(oneShot);
            })
            .func("setOneShot", [](GTimer &self) {
                self.setOneShot();
            })
            .func("start", [](GTimer &self, int64_t interval) {
                self.start(interval);
            })
            .func("start", [](GTimer &self, int64_t delay, int64_t interval) {
                self.start(delay, interval);
            })
            .func("stop", &GTimer::stop);
}

GX_NS_END