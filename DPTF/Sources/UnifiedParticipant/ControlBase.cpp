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

#include "ControlBase.h"

ControlBase::ControlBase(UIntN participantIndex, UIntN domainIndex, ParticipantServicesInterface* participantServices)
    : m_participantIndex(participantIndex), m_domainIndex(domainIndex), m_participantServices(participantServices),
    m_activityLoggingEnabled(false)
{

}

ControlBase::~ControlBase()
{

}

UIntN ControlBase::getParticipantIndex() const
{
    return m_participantIndex;
}

UIntN ControlBase::getDomainIndex() const
{
    return m_domainIndex;
}

Bool ControlBase::isActivityLoggingEnabled(void)
{
    return m_activityLoggingEnabled;
}

void ControlBase::enableActivityLogging(void)
{
    m_activityLoggingEnabled = true;
}

void ControlBase::disableActivityLogging(void)
{
    m_activityLoggingEnabled = false;
}

ParticipantServicesInterface* ControlBase::getParticipantServices() const
{
    return m_participantServices;
}