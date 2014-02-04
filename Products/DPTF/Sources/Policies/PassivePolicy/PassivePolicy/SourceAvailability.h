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
#include "TimeInterface.h"
#include "PolicyServicesInterfaceContainer.h"
#include <memory>

// responsible for keeping track of when sources are available for limiting and unlimiting controls.
class dptf_export SourceAvailability
{
public:

    SourceAvailability(
        const PolicyServicesInterfaceContainer& policyServices,
        std::shared_ptr<TimeInterface> time);
    ~SourceAvailability();

    // source availability
    void setSourceAsBusy(UIntN source, UInt64 time);
    Bool isBusyNow(UIntN source);
    UInt64 getNextAvailableTime(UIntN source, UInt64 startTime);
    void remove(UIntN source);

    // update services
    void setTime(std::shared_ptr<TimeInterface> time);

    // status
    XmlNode* getXml() const;

private:

    // services
    std::shared_ptr<TimeInterface> m_time;
    PolicyServicesInterfaceContainer m_policyServices;

    // source availaiblity
    std::map<UIntN, UInt64> m_schedule;
};