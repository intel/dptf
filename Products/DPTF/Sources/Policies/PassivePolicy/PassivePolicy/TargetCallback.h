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

// represents a callback submitted for a target
class dptf_export TargetCallback
{
public:

    TargetCallback();
    TargetCallback(UInt64 callbackTime, UInt64 timeStamp, UInt64 callbackHandle);

    UInt64 getTimeDelta() const;
    UInt64 getTimeStamp() const;
    UInt64 getCallbackHandle() const;

private:

    UInt64 m_timeDelta;
    UInt64 m_timeStamp;
    UInt64 m_callbackHandle;
};