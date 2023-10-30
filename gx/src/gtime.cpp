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

#include "gx/gtime.h"

#include <chrono>
#include <ratio>
#include <utility>


GX_NS_BEGIN

using TimeSec = std::chrono::duration<GTime::TimeType>;
using TimeSecD = std::chrono::duration<double>;
using TimeMilliSec = std::chrono::duration<GTime::TimeType, std::milli>;
using TimeMicroSec = std::chrono::duration<GTime::TimeType, std::micro>;
using TimeNanoSec = std::chrono::duration<GTime::TimeType, std::nano>;
using TimeMinute = std::chrono::duration<GTime::TimeType, std::ratio<60, 1>>;
using TimeHour = std::chrono::duration<GTime::TimeType, std::ratio<3600, 1>>;
using TimeDay = std::chrono::duration<GTime::TimeType, std::ratio<86400, 1>>;

GTime::GTime(gx::GTime::Type type, GTime::TimeType nanosecond)
        : mType(type),
          mTime(nanosecond)
{
    if (mTime < 0) {
        update();
    }
}

GTime::~GTime() = default;

GTime::GTime(const gx::GTime &b) = default;

GTime::GTime(GTime &&b) noexcept
        :
        mType(b.mType),
        mTime(b.mTime)
{
    b.mTime = 0;
}

GTime &GTime::operator=(const GTime &b) = default;

GTime &GTime::operator=(GTime &&b) noexcept
{
    if (this != &b) {
        swap(b);
        b.mTime = 0;
    }
    return *this;
}

void GTime::swap(GTime &b)
{
    std::swap(b.mType, this->mType);
    std::swap(b.mTime, this->mTime);
}

void GTime::resetToSystemClock()
{
    mType = GTime::SystemClock;
    update();
}

void GTime::resetToSteadyClock()
{
    mType = GTime::SteadyClock;
    update();
}

void GTime::update()
{
    mTime = mType == GTime::SystemClock
            ? std::chrono::duration_cast<TimeNanoSec>((std::chrono::system_clock::now()).time_since_epoch()).count()
            : std::chrono::duration_cast<TimeNanoSec>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

void GTime::reset()
{
    mTime = 0;
}

GTime::TimeType GTime::millisecond() const
{
    return std::chrono::duration_cast<TimeMilliSec>(TimeNanoSec(mTime)).count();
}

GTime::TimeType GTime::nanosecond() const
{
    return mTime;
}

GTime::TimeType GTime::microsecond() const
{
    return std::chrono::duration_cast<TimeMicroSec>(TimeNanoSec(mTime)).count();
}

GTime::TimeType GTime::second() const
{
    return std::chrono::duration_cast<TimeSec>(TimeNanoSec(mTime)).count();
}

double GTime::secondD() const
{
    return std::chrono::duration_cast<TimeSecD>(TimeNanoSec(mTime)).count();
}

GTime::TimeType GTime::minute() const
{
    return std::chrono::duration_cast<TimeMinute>(TimeNanoSec(mTime)).count();
}

GTime::TimeType GTime::hour() const
{
    return std::chrono::duration_cast<TimeHour>(TimeNanoSec(mTime)).count();
}

GTime::TimeType GTime::day() const
{
    return std::chrono::duration_cast<TimeDay>(TimeNanoSec(mTime)).count();
}

GTime::TimeType GTime::milliSecsTo(const GTime &other) const
{
    return this->millisecond() - other.millisecond();
}

GTime::TimeType GTime::microSecsTo(const GTime &other) const
{
    return this->microsecond() - other.microsecond();
}

GTime::TimeType GTime::nanoSecsTo(const GTime &other) const
{
    return this->nanosecond() - other.nanosecond();
}

GTime::TimeType GTime::secsTo(const GTime &other) const
{
    return this->second() - other.second();
}

double GTime::secsDTo(const GTime &other) const
{
    return this->secondD() - other.secondD();
}

void GTime::addMilliSecs(GTime::TimeType ms)
{
    this->mTime += (ms * 1000000L);
}

void GTime::addSecs(GTime::TimeType s)
{
    this->mTime += (s * 1000000000L);
}

void GTime::addMicroSecs(GTime::TimeType ms)
{
    this->mTime += (ms * 1000L);
}

void GTime::addNanoSecs(GTime::TimeType ns)
{
    this->mTime += ns;
}

std::string GTime::toString(const std::string &format, bool utc) const
{
    TimeType msecs = this->millisecond();
    time_t tTime = msecs / 1000;
    struct tm tmv{};
    if (utc) {
#if GX_PLATFORM_WINDOWS
        ::gmtime_s(&tmv, &tTime);
#else
        ::gmtime_r(&tTime, &tmv);
#endif
    } else {
#if GX_PLATFORM_WINDOWS
        ::localtime_s(&tmv, &tTime);
#else
        ::localtime_r(&tTime, &tmv);
#endif
    }
    int64_t onlyMs = msecs % 1000;
    GString formatMs = GString::toString(onlyMs);
    if (onlyMs / 10 == 0) {
        formatMs.insert(0, "0");
    }
    if (onlyMs / 100 == 0) {
        formatMs.insert(0, "0");
    }
    GString _format = format;
    _format = _format.replace("yyyy", GString::toString(tmv.tm_year + 1900))
                 .replace("yy", ((tmv.tm_year + 1900) % 100) / 10 ? GString::toString((tmv.tm_year + 1900) % 100) :
                                GString("0") + GString::toString((tmv.tm_year + 1900) % 100))
                 .replace("MM", tmv.tm_mon >= 9 ?
                                GString::toString(tmv.tm_mon + 1) : GString("0") +
                                                                    GString::toString(tmv.tm_mon + 1))
                 .replace("M", GString::toString(tmv.tm_mon + 1))
                 .replace("dd", tmv.tm_mday / 10 ?
                                GString::toString(tmv.tm_mday) : GString("0") + GString::toString(tmv.tm_mday))
                 .replace("d", GString::toString(tmv.tm_mday))
                 .replace("HH", tmv.tm_hour / 10 ?
                                GString::toString(tmv.tm_hour) : GString("0") + GString::toString(tmv.tm_hour))
                 .replace("H", GString::toString(tmv.tm_hour))
                 .replace("hh", (tmv.tm_hour % 12) / 10 ?
                                GString::toString(tmv.tm_hour % 12) : GString("0")
                                                                      + GString::toString(tmv.tm_hour % 12))
                 .replace("h", GString::toString(tmv.tm_hour % 12))
                 .replace("mm", tmv.tm_min / 10 ?
                                GString::toString(tmv.tm_min) : GString("0") + GString::toString(tmv.tm_min))
                 .replace("m", GString::toString(tmv.tm_min))
                 .replace("ss", tmv.tm_sec / 10 ?
                                GString::toString(tmv.tm_sec) : GString("0") + GString::toString(tmv.tm_sec))
                 .replace("s", GString::toString(tmv.tm_sec))
                 .replace("zzz", formatMs)
                 .replace("z", GString::toString(onlyMs))
                 .replace("ap", tmv.tm_hour < 12 ? "am" : "pm")
                 .replace("AP", tmv.tm_hour < 12 ? "AM" : "PM");
    return _format.toStdString();
}

bool gx::GTime::operator==(const gx::GTime &rh) const
{
    return mTime == rh.mTime;
}

bool GTime::operator!=(const GTime &rh) const
{
    return !(*this == rh);
}

bool gx::GTime::operator<(const gx::GTime &rh) const
{
    return mTime < rh.mTime;
}

bool GTime::operator>(const GTime &rh) const
{
    return mTime > rh.mTime;
}

bool GTime::operator<=(const GTime &rh) const
{
    return mTime <= rh.mTime;
}

bool GTime::operator>=(const GTime &rh) const
{
    return mTime >= rh.mTime;
}

GTime GTime::currentSystemTime()
{
    GTime t = GTime(GTime::SystemClock);
    return t;
}

GTime GTime::currentSteadyTime()
{
    GTime t = GTime(GTime::SteadyClock);
    return t;
}

GX_NS_END