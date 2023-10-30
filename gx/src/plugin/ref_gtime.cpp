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

#include "gx/gtime.h"


GX_NS_BEGIN

void refGTime()
{
    Class<GTime>("Gx", "GTime", "Gx time")
            .construct<>()
            .construct<GTime::Type>()
            .construct<GTime::Type, int64_t>()
            .construct<GTime &>()
            .inherit<GObject>()
            .defEnum("Type", {
                    {"SystemClock", GTime::Type::SystemClock},
                    {"SteadyClock", GTime::Type::SteadyClock}
            }, "Time type, SystemClock: System clock, follow the time set by the system; "
               "SteadyClock: Steady clock, the time points of this clock cannot decrease as physical time moves "
               "forward and the time between ticks of this clock is constant.")
            .func("swap", &GTime::swap, "Swap two GTime objects.")
            .func("resetToSystemClock", &GTime::resetToSystemClock,
                 "Reset the clock to the system clock and update it to the latest time.")
            .func("resetToSteadyClock", &GTime::resetToSteadyClock,
                 "Reset the clock to a steady clock and update it to the latest time.")
            .func("update", &GTime::update,"Update the clock to the latest time of the corresponding type.")
            .func("reset", &GTime::reset, "Reset time to 0.")
            .func("millisecond", &GTime::millisecond, "Get milliseconds of time.")
            .func("nanosecond", &GTime::nanosecond, "Get nanoseconds of time.")
            .func("microsecond", &GTime::microsecond, "Get microseconds of time.")
            .func("second", &GTime::second, "Get seconds of time.")
            .func("secondD", &GTime::secondD, "Gets the number of seconds of time, in double numbers.")
            .func("minute", &GTime::minute, "Get minutes of time.")
            .func("hour", &GTime::hour, "Get hour of time.")
            .func("day", &GTime::day, "Get day of time.")
            .func("milliSecsTo", &GTime::milliSecsTo, "Calculate the millisecond difference between two times.")
            .func("microSecsTo", &GTime::microSecsTo, "Calculate the microsecond difference between two times.")
            .func("nanoSecsTo", &GTime::nanoSecsTo, "Calculate the nanosecond difference between two times.")
            .func("secsTo", &GTime::secsTo, "Calculate the second difference between two times.")
            .func("secsDTo", &GTime::secsDTo, "Calculate the second(double) difference between two times.")
            .func("addMilliSecs", &GTime::addMilliSecs, "Increase the specified number of milliseconds.")
            .func("addSecs", &GTime::addSecs, "Increase the specified number of seconds.")
            .func("addMicroSecs", &GTime::addMicroSecs, "Increase the specified number of microseconds.")
            .func("addNanoSecs", &GTime::addNanoSecs, "Increase the specified number of nanoseconds.")
            .func("toString", [](const GTime &t, const std::string &format) {
                return t.toString(format);
            }, "Format time is a string, only for SystemClock. "
               "arg1: format.")
            .func("toString", [](const GTime &t, const std::string &format, bool utc) {
                return t.toString(format, utc);
            }, "Format time is a string, only for SystemClock. "
               "arg1: format; arg2: utc.")
            .func(MetaFunction::Subtraction, [](const GTime &t1, const GTime &t2) {
                return t1 - t2;
            }, "Calculate the difference between two times, the two times must be of the same type.")
            .func(MetaFunction::Addition, [](const GTime &t1, const GTime &t2) {
                return t1 + t2;
            }, "To calculate the sum of two times, the two times must be of the same type.")
            .func(MetaFunction::EqualTo, &GTime::operator==)
            .func(MetaFunction::LessThan, &GTime::operator<)
            .staticFunc("currentSystemTime", &GTime::currentSystemTime, "Get current system time.")
            .staticFunc("currentSteadyTime", &GTime::currentSteadyTime, "Get current steady time.");
}

GX_NS_END
