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

#include "gx/task_system.h"


GX_NS_BEGIN

void refTaskSystem()
{
    Class<TaskSystem>("Gx", "TaskSystem",
                      "Gx task system, A multithreaded task system that supports synchronous waiting for task results.")
            .construct<>("Default constructor, Number of threads created according to the number of CPU cores.")
            .construct<int32_t>("Constructor, The parameter is the number of threads.")
            .construct<int32_t, std::string>("Constructor, The parameter is the number of threads.")
            .func("threadCount", &TaskSystem::threadCount, "Get number of worker threads.")
            .func("start", &TaskSystem::start, "Start the TaskSystem after calling this function.")
            .func("stopAndWait", &TaskSystem::stopAndWait,
                 "Stop the TaskSystem after all tasks in the task queue are completed.")
            .func("stop", &TaskSystem::stop,
                 "Clear the task queue and stop the TaskSystem. Unexecuted tasks will not be executed.")
            .func("isRunning", &TaskSystem::isRunning, "Check whether the TaskSystem is running.")
            .func("setThreadPriority", &TaskSystem::setThreadPriority, "Set thread priority.")
            .func("getThreadPriority", &TaskSystem::getThreadPriority, "Get thread priority.")
            .func("submit", [](TaskSystem &self, GAny &runnable) {
                if (runnable.isFunction()) {
                    auto task = std::make_unique<TaskSystem::Task<GAny>>(
                            std::move(self.submit([runnable]() {
                                try {
                                    return runnable();
                                } catch (GAnyException &e) {
                                    GX_ASSERT_S(false, "TaskSystem runnable error: %s.", e.what());
                                    LogE("TaskSystem runnable error: %s.", e.what());
                                    return GAny::undefined();
                                }
                            })));
                    return GAny(std::move(task));
                }
                return GAny::undefined();
            }, "Submit a task to the task queue. Arg1 is a nonparametric function. "
               "GAnyFuture will be returned if the task is submitted successfully, and undefined if it fails.")
            .func("submit", [](TaskSystem &self, GAny &runnable, const GAny &params) {
                if (runnable.isFunction()) {
                    auto task = std::make_unique<TaskSystem::Task<GAny>>(
                            std::move(self.submit([runnable, params]() {
                                try {
                                    return runnable(params);
                                } catch (GAnyException &e) {
                                    GX_ASSERT_S(false, "TaskSystem runnable error: %s.", e.what());
                                    LogE("TaskSystem runnable error: %s.", e.what());
                                    return GAny::undefined();
                                }
                            })));
                    return GAny(std::move(task));
                }
                return GAny::undefined();
            }, "Submit a task to the task queue. Arg1 is a function with a GAny parameter and arg2 is a task parameter."
               " It will be passed in as a parameter of arg1.")
            .func("submitFront", [](TaskSystem &self, GAny &runnable) {
                if (runnable.isFunction()) {
                    auto task = std::make_unique<TaskSystem::Task<GAny>>(
                            std::move(self.submitFront([runnable]() {
                                try {
                                    return runnable();
                                } catch (GAnyException &e) {
                                    GX_ASSERT_S(false, "TaskSystem runnable error: %s.", e.what());
                                    LogE("TaskSystem runnable error: %s.", e.what());
                                    return GAny::undefined();
                                }
                            })));
                    return GAny(std::move(task));
                }
                return GAny::undefined();
            }, "Submit a task to the task queue and insert it in the front of the task queue.")
            .func("submitFront", [](TaskSystem &self, GAny &runnable, const GAny &params) {
                if (runnable.isFunction()) {
                    auto task = std::make_unique<TaskSystem::Task<GAny>>(
                            std::move(self.submitFront([runnable, params]() {
                                try {
                                    return runnable(params);
                                } catch (GAnyException &e) {
                                    GX_ASSERT_S(false, "TaskSystem runnable error: %s.", e.what());
                                    LogE("TaskSystem runnable error: %s.", e.what());
                                    return GAny::undefined();
                                }
                            })));
                    return GAny(std::move(task));
                }
                return GAny::undefined();
            }, "Submit a task to the task queue and insert it in the front of the task queue."
               " Arg2 is the task parameter and will be passed in as arg1 parameter.")
            .func("waitingTaskCount", &TaskSystem::waitingTaskCount,
                 "Get the count of tasks waiting.");

    Class<TaskSystem::Task<GAny>>("Gx", "Task", "Task results of TaskSystem.")
            .func("get",
                 [](TaskSystem::Task<GAny> &self) {
                     return self.get();
                 }, "Wait for the task result, and wait until the task is completed and returned.")
            .func("wait",
                 [](TaskSystem::Task<GAny> &self) {
                     self.wait();
                 }, "Block waiting for task to complete.")
            .func("waitFor",
                 [](TaskSystem::Task<GAny> &self, int64_t ms) {
                     return self.waitFor(ms);
                 },
                 "Blocking and waiting for task completion within a certain time, "
                 "if the task is completed within the specified time, true will be returned; otherwise, "
                 "false will be returned.")
            .func("cancel",
                 [](TaskSystem::Task<GAny> &self) {
                     self.cancel();
                 }, "Cancel task.")
            .func("isValid",
                 [](TaskSystem::Task<GAny> &self) {
                     return self.isValid();
                 }, "Whether the task is valid. If it is canceled or got, the task will be invalid.")
            .func("subscribe", [](TaskSystem::Task<GAny> &self, const GAny &action) {
                if (action.isFunction()) {
                    self.subscribe([action](const GAny &ret) {
                        action(ret);
                    });
                }
            }, "Subscribe to tasks and return results by specifying the scheduler thread. "
               "arg1: action, function(GAny ret).")
            .func("subscribe", [](TaskSystem::Task<GAny> &self, const GAny &action, const GTimerSchedulerPtr &scheduler) {
                if (action.isFunction()) {
                    self.subscribe([action](const GAny &ret) {
                        action(ret);
                    }, scheduler);
                }
            }, "Subscribe to tasks and return results by specifying the scheduler thread. "
               "arg1: action, function(GAny ret); arg2: scheduler.");
}

GX_NS_END
