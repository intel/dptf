/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
**
** Licensed under the Apache License, Version 2.0 (the "License"); you may not
** use this file except in compliance with the License.
**
** You may obtain a copy of the License at
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
** WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**
** See the License for the specific language governing permissions and
** limitations under the License.
**
******************************************************************************/

#pragma once

#include "Dptf.h"

class EsifTime final
{
public:

    // Creates a new instance of EsifTime initialized to the current time.
    EsifTime(void);

    // Creates a new instance of EsifTime based on the number of milliseconds passed in.
    EsifTime(UInt64 numMilliSeconds);

    // sets the internal structure to the current time.
    void refresh(void);

    // returns the internal time stamp (milliseconds)
    UInt64 getTimeStampInMilliSec(void) const;

    Bool operator==(const EsifTime& rhs) const;
    Bool operator!=(const EsifTime& rhs) const;
    Bool operator>(const EsifTime& rhs) const;
    Bool operator>=(const EsifTime& rhs) const;
    Bool operator<(const EsifTime& rhs) const;
    Bool operator<=(const EsifTime& rhs) const;

    EsifTime operator+(UInt64 numMilliSeconds) const;
    EsifTime operator-(UInt64 numMilliSeconds) const;
    EsifTime& operator+=(UInt64 numMilliSeconds);
    EsifTime& operator-=(UInt64 numMilliSeconds);

    UInt64 operator-(const EsifTime& rhs) const;

private:

    UInt64 m_timeStamp;                                             // stores num milliseconds since fixed point in time

    UInt64 getCurrentTime(void);
};