/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#include "WIDptfParticipantActivityLoggingEnabled.h"
#include "PolicyManager.h"
#include "ParticipantManager.h"
#include "EsifServices.h"

WIDptfParticipantActivityLoggingEnabled::WIDptfParticipantActivityLoggingEnabled(
    DptfManagerInterface* dptfManager, UIntN participantIndex, UIntN domainIndex, UInt32 capabilityId) :
    DomainWorkItem(dptfManager, FrameworkEvent::Type::DptfParticipantActivityLoggingEnabled, participantIndex, domainIndex),
    m_capabilityId(capabilityId)
{
}

WIDptfParticipantActivityLoggingEnabled::~WIDptfParticipantActivityLoggingEnabled(void)
{
}

void WIDptfParticipantActivityLoggingEnabled::execute(void)
{
    WriteDomainWorkItemStartingInfoMessage();

    try
    {
        getParticipantPtr()->activityLoggingEnabled(getDomainIndex(), m_capabilityId);
    }
    catch (std::exception& ex)
    {
        WriteDomainWorkItemErrorMessage_Function("Participant::activityLoggingEnabled");
    }
}