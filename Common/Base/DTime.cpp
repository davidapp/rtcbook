//
//  DTime.cpp
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 07/09/2018 for Dream.
//  Copyright 2018. All rights reserved.
//

#include "DTime.h"
#include <time.h>

//iOS
#if defined(BUILD_FOR_IOS) && (BUILD_FOR_IOS==1)
#import <unistd.h>  //for sleep
#import <sys/time.h>//for gettimeofday and localtime_r
#import <mach/mach_time.h>
#endif

//Android
#if defined(BUILD_FOR_ANDROID) && (BUILD_FOR_ANDROID==1)
#import <unistd.h>  //for sleep
#import <sys/time.h>//for gettimeofday and localtime_r
#endif

//Mac
#if defined(BUILD_FOR_MAC) && (BUILD_FOR_MAC==1)
#import <unistd.h>  //for sleep
#import <sys/time.h>//for gettimeofday and localtime_r
#endif

//Linux
#if defined(BUILD_FOR_LINUX) && (BUILD_FOR_LINUX==1)
#include <unistd.h>  //for sleep
#include <sys/time.h>//for gettimeofday and localtime_r
#endif


//marcos
#define SECONDOFROUNDYEAR 31622400 //3600*24*366
#define SECONDOFYEAR      31536000 //3600*24*365
#define MAXSECONDOFDAY    86400	   //24*60*60

static DInt32 month_s[2][12] = { {31,28,31,30,31,30,31,31,30,31,30,31},
{31,29,31,30,31,30,31,31,30,31,30,31} };

#define MONTAB(year) \
((year) & 03 || (year) == 0 ? mos : lmos)
static const short lmos[] = { 0, 31, 60, 91, 121, 152,
    182, 213, 244, 274, 305, 335 };
static const short mos[] = { 0, 31, 59, 90, 120, 151,
    181, 212, 243, 273, 304, 334 };


/////////////////////////////////////////////////////////////////////////////
// DTime

DUInt32 DTime::GetUnixTimeStamp32()
{
    return (DUInt32)time(0);
}

DUInt64 DTime::GetUnixTimeStamp64()
{
    return (DUInt64)time(0);
}

DVoid DTime::GetCurrentLocalDTM(dtm* tm)
{
#if defined(BUILD_FOR_WINDOWS)
    SYSTEMTIME stLocal;
    ::GetLocalTime(&stLocal);
    if (tm)
    {
        tm->tm_year = stLocal.wYear;
        tm->tm_mon = stLocal.wMonth;
        tm->tm_mday = stLocal.wDay;
        tm->tm_hour = stLocal.wHour;
        tm->tm_min = stLocal.wMinute;
        tm->tm_sec = stLocal.wSecond;
    }
#else
    struct timeval stTime;
    gettimeofday(&stTime, NULL);
    struct tm stLocal;
    localtime_r(&stTime.tv_sec, &stLocal);
    if (tm)
    {
        tm->tm_year = stLocal.tm_year + 1900;
        tm->tm_mon = stLocal.tm_mon + 1;
        tm->tm_mday = stLocal.tm_mday;
        tm->tm_hour = stLocal.tm_hour;
        tm->tm_min = stLocal.tm_min;
        tm->tm_sec = stLocal.tm_sec;
    }
#endif
}

static int _Daysto(int year, int mon)
{
    /* compute extra days to start of month */
    int days;
    if (0 < year)        /* correct for leap year: 1801-2099 */
        days = (year - 1) / 4;
    else if (year <= -4)
        days = 1 + (4 - year) / 4;
    else
        days = 0;
    return (days + MONTAB(year)[mon]);
}

DVoid DTime::UnixTS2TM(DUInt64 secsarg, dtm* t)
{
    DUInt64 secs = secsarg;

    DInt32 days = (DInt32)(secs / 86400);
    //Year
    DInt32 i;
    DInt32 year;
    for (year = days / 365; days < (i = _Daysto(year, 0) + 365L * year); )
        --year;
    days -= i;
    t->tm_year = year + 1970;       //1970+diff

    //Month
    DInt32 mon;
    const short* pm = MONTAB(year);
    for (mon = 12; days < pm[--mon]; )
        ;
    t->tm_mon = mon + 1;                //[1-12]

    //Day
    t->tm_mday = days - pm[mon] + 1;    //[1-31]

    //Hour
    secs %= 86400;
    t->tm_hour = (DInt32)(secs / 3600); //[0-23]

    //Minute
    secs %= 3600;
    t->tm_min = (DInt32)(secs / 60);    //[0-59]

    //Second
    t->tm_sec = secs % 60;              //[0-59]
}

DVoid DTime::TM2UnixTS(dtm* tm, DUInt64* ts)
{
    DUInt32 tSecond = tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;
    DInt32  nCount = 0;
    for (int i = 1970; i < tm->tm_year; ++i)
    {
        if (DTime::IsRound(i))
            ++nCount;
    }

    DUInt64 tTemp = (-1970 - (DUInt64)nCount) * SECONDOFYEAR + (DUInt64)nCount * SECONDOFROUNDYEAR;

    if (tm->tm_mon > 1)
    {
        if (DTime::IsRound(tm->tm_year))
        {
            for (int j = 0; j < tm->tm_mon - 1; ++j)
            {
                tTemp += (DUInt64)month_s[1][j] * MAXSECONDOFDAY;
            }
            tTemp += (DUInt64)(tm->tm_mday - 1) * MAXSECONDOFDAY + tSecond;
        }
        else
        {
            for (int j = 0; j < tm->tm_mon - 1; ++j)
            {
                tTemp += (DUInt64)month_s[0][j] * MAXSECONDOFDAY;
            }
            tTemp += (DUInt64)(tm->tm_mday - 1) * MAXSECONDOFDAY + tSecond;
        }
    }
    else
    {
        tTemp += (DUInt64)(tm->tm_mday - 1) * MAXSECONDOFDAY + tSecond;
    }

    *ts = tTemp;
}

DUInt64 DTime::GetFSTimeStamp64_Local()
{
#if defined(BUILD_FOR_WINDOWS)
    FILETIME ft = {}, ftLocal = {};
    ::GetSystemTimeAsFileTime(&ft);
    ::FileTimeToLocalFileTime(&ft, &ftLocal);
    DUInt64 nRet = ftLocal.dwHighDateTime;
    nRet <<= 32;
    nRet += ft.dwLowDateTime;
    return nRet;
#endif
    return 0;
}

DUInt64 DTime::GetFSTimeStamp64_UTC()
{
#if defined(BUILD_FOR_WINDOWS)
    FILETIME ft = {};
    ::GetSystemTimeAsFileTime(&ft);
    DUInt64 nRet = ft.dwHighDateTime;
    nRet <<= 32;
    nRet += ft.dwLowDateTime;
    return nRet;
#endif
    return 0;
}

DVoid DTime::FSTS2TM(DUInt64 ts, dtm* tm)
{
#if defined(BUILD_FOR_WINDOWS)
    SYSTEMTIME st = {};
    FILETIME ft = {};
    ft.dwHighDateTime = ts >> 32;
    ft.dwLowDateTime = (DUInt32)ts;
    ::FileTimeToSystemTime(&ft, &st);
    if (tm)
    {
        tm->tm_year = st.wYear;
        tm->tm_mon = st.wMonth;
        tm->tm_mday = st.wDay;
        tm->tm_hour = st.wHour;
        tm->tm_min = st.wMinute;
        tm->tm_sec = st.wSecond;
    }
#endif
}

DVoid DTime::TM2FSTS(dtm* tm, DUInt64* ts)
{
#if defined(BUILD_FOR_WINDOWS)
    if (tm == NULL)
    {
        *ts = 0;
        return;
    }

    SYSTEMTIME st = {};
    st.wYear = tm->tm_year;
    st.wMonth = tm->tm_mon;
    st.wDay = tm->tm_mday;
    st.wHour = tm->tm_hour;
    st.wMinute = tm->tm_min;
    st.wSecond = tm->tm_sec;
    st.wMilliseconds = 0;
    st.wDayOfWeek = 0;
    FILETIME ft = {};
    ::SystemTimeToFileTime(&st, &ft);
    *ts = ((DUInt64)ft.dwHighDateTime << 32) + ft.dwLowDateTime;
#endif
}

DVoid DTime::SleepSec(DUInt32 second)
{
#if defined(BUILD_FOR_WINDOWS)
    ::Sleep(second * 1000);
#else
    sleep(second);
#endif
}

DUInt32 DTime::GetTickCount32()
{
#if defined(BUILD_FOR_WINDOWS)
    DWARNING_SUPPRESS(28159)
        return ::GetTickCount();
    DWARNING_UNSUPPRESS()
#elif defined(BUILD_FOR_IOS) && (BUILD_FOR_IOS==1)
    static mach_timebase_info_data_t sTimebaseInfo;
    uint64_t machTime = mach_absolute_time();
    if (sTimebaseInfo.denom == 0)
    {
        (void)mach_timebase_info(&sTimebaseInfo);
    }
    // Convert the mach time to mili seconds
    DUInt32 millis = (DUInt32)((machTime / 1000000) * sTimebaseInfo.numer) / sTimebaseInfo.denom;
    return millis;
#elif defined(BUILD_FOR_ANDROID) && (BUILD_FOR_ANDROID == 1)
    //TODO
    return 0;
#endif
    return 0;
}

//in mili seconds
DUInt64 DTime::GetTickCount64()
{
#if defined(BUILD_FOR_WINDOWS)
    return ::GetTickCount64();
#elif defined(BUILD_FOR_IOS) && (BUILD_FOR_IOS==1)
    static mach_timebase_info_data_t sTimebaseInfo;
    uint64_t machTime = mach_absolute_time();
    if (sTimebaseInfo.denom == 0)
    {
        (void)mach_timebase_info(&sTimebaseInfo);
    }
    // Convert the mach time to mili seconds
    DUInt64 millis = (DUInt64)((machTime / 1000000) * sTimebaseInfo.numer) / sTimebaseInfo.denom;
    return millis;
#elif defined(BUILD_FOR_ANDROID) && (BUILD_FOR_ANDROID == 1)
    //TODO
    return 0;
#endif
    return 0;
}

DVoid DTime::PrintTM(dtm* tm)
{
    DPrintf("%04d/%02d/%02d %02d:%02d:%02d\n", tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
}

DStringA DTime::UnixTSToStr(DUInt64 ts)
{
    dtm tm;
    DTime::UnixTS2TM(ts, &tm);
    DStringA str;
    str.Format("%04d/%02d/%02d %02d:%02d:%02d", tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return str;
}

DStringA DTime::UnixTSToNiceStr(DUInt64 ts)
{
    dtm pm;
    DTime::UnixTS2TM(ts, &pm);
    DUInt64 now = GetUnixTimeStamp64();
    DUInt64 dt = now - ts;
    DStringA strRet;
    if (dt >= 7 * 24 * 3600)
    {
        strRet.Format("%04d/%02d/%02d", pm.tm_year, pm.tm_mon, pm.tm_mday);
    }
    else if (dt < 7 * 24 * 3600 && dt >= 2 * 24 * 3600)
    {
        //strRet.Format("week%d", pm.tm_wday);
    }
    else if (dt < 2 * 24 * 3600 && dt >= 24 * 3600)
    {
        strRet.Format("yesterday");
    }
    else
    {
        strRet.Format("%02d:%02d", pm.tm_hour, pm.tm_min);
    }
    return strRet;
}

DStringA DTime::GetCurrentDateTimeStr()
{
    dtm tm;
    DTime::GetCurrentLocalDTM(&tm);
    DStringA str;
    str.Format("%04d/%02d/%02d %02d:%02d:%02d", tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return str;
}

DStringA DTime::GetLogTime()
{
    dtm tm;
    DTime::GetCurrentLocalDTM(&tm);
    DStringA str;
    str.Format("%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
    return str;
}

DStringA DTime::GetCurrentDateStr()
{
    dtm tm;
    DTime::GetCurrentLocalDTM(&tm);
    DStringA str;
    str.Format("%04d/%02d/%02d", tm.tm_year, tm.tm_mon, tm.tm_mday);
    return str;
}

DStringA DTime::GetCurrentTimeStr()
{
    dtm tm;
    DTime::GetCurrentLocalDTM(&tm);
    DStringA str;
    str.Format("%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
    return str;
}

DBool DTime::IsRound(DInt32 year)
{
    if ((year % 100) && (year % 4 == 0)) return true;
    if ((year % 100 == 0) && (year % 400 == 0)) return true;
    return false;
}

DInt32 DTime::GetYearDay(dtm* tm)
{
    return 0;
}

DInt32 DTime::GetWeekDay(dtm* tm)
{
    return 0;
}


DTime::DTime()
{
    m_time = DTime::GetUnixTimeStamp64();
}

DTime::DTime(DUInt64 time)
{
    m_time = time;
}

DTime::DTime(DInt32 nYear, DInt32 nMonth, DInt32 nDay, DInt32 nHour, DInt32 nMin, DInt32 nSec)
{
    DAssert(nYear >= 1601);
    DAssert(nMonth >= 1 && nMonth <= 12);
    DAssert(nDay >= 1 && nDay <= 31);
    DAssert(nHour >= 0 && nHour <= 23);
    DAssert(nMin >= 0 && nMin <= 59);
    DAssert(nSec >= 0 && nSec <= 59);

    dtm tm;
    tm.tm_year = nYear;
    tm.tm_mon = nMonth;
    tm.tm_mday = nDay;
    tm.tm_hour = nHour;
    tm.tm_min = nMin;
    tm.tm_sec = nSec;

    DTime::TM2UnixTS(&tm, &m_time);
}

DTime::DTime(const DTime& timeSrc)
{
    m_time = timeSrc.GetTime();
}

DUInt64 DTime::GetTime() const
{
    return m_time;
}

DVoid DTime::PrintUnixTS()
{
    dtm tm;
    UnixTS2TM(m_time, &tm);
    DTime::PrintTM(&tm);
}

DVoid DTime::PrintFSTS()
{
    dtm tm;
    FSTS2TM(m_time, &tm);
    DTime::PrintTM(&tm);
}

/////////////////////////////////////////////////////////////////////////////
// DTimeSpan

DTimeSpan::DTimeSpan() : m_timeSpan(0)
{
}

DTimeSpan::DTimeSpan(DUInt64 time) : m_timeSpan(time)
{
}

DTimeSpan::DTimeSpan(DInt32 lDays, DInt32 nHours, DInt32 nMins, DInt32 nSecs)
{
    m_timeSpan = nSecs + 60 * (nMins + 60 * (nHours + (DUInt64)24 * lDays));
}

DInt64 DTimeSpan::GetDays() const
{
    return m_timeSpan / (24 * 3600);
}

