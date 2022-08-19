//
//  DTime.h
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 07/11/2018 for Dream.
//  Copyright 2018. All rights reserved.
//

#pragma once

#include "DConfig.h"
#include "DTypes.h"
#include "Base/DString.h"

typedef struct tagdtm {
    DInt32 tm_year;    /* years */
    DInt32 tm_mon;     /* months since January - [1,12] */
    DInt32 tm_mday;    /* day of the month - [1,31] */
    DInt32 tm_hour;    /* hours since midnight - [0,23] */
    DInt32 tm_sec;     /* seconds after the minute - [0,59] */
    DInt32 tm_min;     /* minutes after the hour - [0,59] */
    //DInt32 tm_wday;    /* days since Sunday - [0,6] */
    //DInt32 tm_yday;    /* days since January 1 - [0,365] */
    //DInt32 tm_isdst;   /* daylight savings time flag */
}dtm;

class DTimeSpan;

class DTime
{
public:
    static DUInt32 GetUnixTimeStamp32();		//second since 1970.1.1
    static DUInt64 GetUnixTimeStamp64();		//second since 1970.1.1
    static DVoid UnixTS2TM(DUInt64 ts, dtm* tm);
    static DVoid TM2UnixTS(dtm* tm, DUInt64* ts);

public:
    static DVoid GetCurrentLocalDTM(dtm* tm);

public:
    static DUInt32 GetTickCount32();            //in MILI second since system started, up to 49.7 days on Win32
    static DUInt64 GetTickCount64();            //in MILI second since system started
    static DVoid   SleepSec(DUInt32 second);

public:
    static DUInt64 GetFSTimeStamp64_Local();	//the number of 100-nanosecond intervals since January 1, 1601 (Local)
    static DUInt64 GetFSTimeStamp64_UTC();      //the number of 100-nanosecond intervals since January 1, 1601 (UTC)
    static DVoid FSTS2TM(DUInt64 ts, dtm* tm);
    static DVoid TM2FSTS(dtm* tm, DUInt64* ts);

public:
    static DVoid PrintTM(dtm* tm);
    static DStringA UnixTSToStr(DUInt64 ts);    //Date+Time+Week
    static DStringA UnixTSToNiceStr(DUInt64 ts);//As WeChat

    static DStringA GetCurrentDateTimeStr();
    static DStringA GetCurrentDateStr();
    static DStringA GetCurrentTimeStr();

public:
    static DStringA GetLogTime();

public:
    static DBool  IsRound(DInt32 year);
    static DInt32 GetYearDay(dtm* tm);
    static DInt32 GetWeekDay(dtm* tm);

public:
    DTime();
    explicit DTime(DUInt64 time);
    DTime(DInt32 nYear, DInt32 nMonth, DInt32 nDay, DInt32 nHour, DInt32 nMin, DInt32 nSec);
    DTime(const DTime& timeSrc);

    DTime& operator=(DUInt64 t);
    DTime& operator=(const DTime& timeSrc);
    DUInt64 GetTime() const;

public:
    DVoid PrintUnixTS();
    DVoid PrintFSTS();

public:
    friend class DTimeSpan;
    DTimeSpan operator-(DTime time) const;
    DTime operator-(DTimeSpan timeSpan) const;
    DTime operator+(DTimeSpan timeSpan) const;
    const DTime& operator+=(DTimeSpan timeSpan);
    const DTime& operator-=(DTimeSpan timeSpan);
    DBool operator==(DTime time) const;
    DBool operator!=(DTime time) const;
    DBool operator<(DTime time) const;
    DBool operator>(DTime time) const;
    DBool operator<=(DTime time) const;
    DBool operator>=(DTime time) const;

private:
    //Windows uses the number of 100-nanosecond intervals since January 1, 1601 (UTC)
    //But here we use second since 1970.1.1 (UTC)
    DUInt64 m_time;
};


/////////////////////////////////////////////////////////////////////////////
// DTimeSpan

class DTimeSpan
{
public:
    // Constructors
    DTimeSpan();
    DTimeSpan(DUInt64 time);
    DTimeSpan(DInt32 lDays, DInt32 nHours, DInt32 nMins, DInt32 nSecs);

    DTimeSpan(const DTimeSpan& timeSpanSrc);
    DTimeSpan& operator=(const DTimeSpan& timeSpanSrc);

    // Attributes
    DInt64 GetDays() const;
    DInt64 GetTotalHours() const;
    DInt32 GetHours() const;
    DInt64 GetTotalMinutes() const;
    DInt32 GetMinutes() const;
    DInt64 GetTotalSeconds() const;
    DInt32 GetSeconds() const;

    DUInt64 GetTimeSpan() const throw();

    // Operations
    DTimeSpan operator-(DTimeSpan timeSpan) const;
    DTimeSpan operator+(DTimeSpan timeSpan) const;
    const DTimeSpan& operator+=(DTimeSpan timeSpan);
    const DTimeSpan& operator-=(DTimeSpan timeSpan);
    DBool operator==(DTimeSpan timeSpan) const;
    DBool operator!=(DTimeSpan timeSpan) const;
    DBool operator<(DTimeSpan timeSpan) const;
    DBool operator>(DTimeSpan timeSpan) const;
    DBool operator<=(DTimeSpan timeSpan) const;
    DBool operator>=(DTimeSpan timeSpan) const;

    DString Format(DCWStr pFormat) const;

private:
    DUInt64 m_timeSpan;
    friend class DTime;
};


/////////////////////////////////////////////////////////////////////////////
//inline Functions

inline DTime& DTime::operator=(DUInt64 time)
{
    m_time = time;
    return(*this);
}

inline DTime& DTime::operator=(const DTime& timeSrc)
{
    DTime t = timeSrc;
    m_time = t.GetTime();
    return (*this);
}

inline const DTime& DTime::operator+=(DTimeSpan span)
{
    m_time += span.GetTimeSpan();

    return(*this);
}

inline const DTime& DTime::operator-=(DTimeSpan span)
{
    m_time -= span.GetTimeSpan();

    return(*this);
}

inline DTimeSpan DTime::operator-(DTime time) const
{
    return(DTimeSpan(m_time - time.m_time));
}

inline DTime DTime::operator-(DTimeSpan span) const
{
    return(DTime(m_time - span.GetTimeSpan()));
}

inline DTime DTime::operator+(DTimeSpan span) const
{
    return(DTime(m_time + span.GetTimeSpan()));
}

inline bool DTime::operator==(DTime time) const
{
    return(m_time == time.m_time);
}

inline bool DTime::operator!=(DTime time) const
{
    return(m_time != time.m_time);
}

inline bool DTime::operator<(DTime time) const
{
    return(m_time < time.m_time);
}

inline bool DTime::operator>(DTime time) const
{
    return(m_time > time.m_time);
}

inline bool DTime::operator<=(DTime time) const
{
    return(m_time <= time.m_time);
}

inline bool DTime::operator>=(DTime time) const
{
    return(m_time >= time.m_time);
}

inline DInt64 DTimeSpan::GetTotalHours() const
{
    return(m_timeSpan / 3600);
}

inline DInt32 DTimeSpan::GetHours() const
{
    return(DInt32(GetTotalHours() - (GetDays() * 24)));
}

inline DInt64 DTimeSpan::GetTotalMinutes() const
{
    return(m_timeSpan / 60);
}

inline DInt32 DTimeSpan::GetMinutes() const
{
    return(DInt32(GetTotalMinutes() - (GetTotalHours() * 60)));
}

inline DInt64 DTimeSpan::GetTotalSeconds() const
{
    return(m_timeSpan);
}

inline DInt32 DTimeSpan::GetSeconds() const
{
    return(DInt32(GetTotalSeconds() - (GetTotalMinutes() * 60)));
}

inline DUInt64 DTimeSpan::GetTimeSpan() const throw()
{
    return(m_timeSpan);
}

inline DTimeSpan DTimeSpan::operator+(DTimeSpan span) const
{
    return(DTimeSpan(m_timeSpan + span.m_timeSpan));
}

inline DTimeSpan DTimeSpan::operator-(DTimeSpan span) const
{
    return(DTimeSpan(m_timeSpan - span.m_timeSpan));
}

inline const DTimeSpan& DTimeSpan::operator+=(DTimeSpan span)
{
    m_timeSpan += span.m_timeSpan;
    return(*this);
}

inline const DTimeSpan& DTimeSpan::operator-=(DTimeSpan span)
{
    m_timeSpan -= span.m_timeSpan;
    return(*this);
}

inline bool DTimeSpan::operator==(DTimeSpan span) const
{
    return(m_timeSpan == span.m_timeSpan);
}

inline bool DTimeSpan::operator!=(DTimeSpan span) const
{
    return(m_timeSpan != span.m_timeSpan);
}

inline bool DTimeSpan::operator<(DTimeSpan span) const
{
    return(m_timeSpan < span.m_timeSpan);
}

inline bool DTimeSpan::operator>(DTimeSpan span) const
{
    return(m_timeSpan > span.m_timeSpan);
}

inline bool DTimeSpan::operator<=(DTimeSpan span) const
{
    return(m_timeSpan <= span.m_timeSpan);
}

inline bool DTimeSpan::operator>=(DTimeSpan span) const
{
    return(m_timeSpan >= span.m_timeSpan);
}
