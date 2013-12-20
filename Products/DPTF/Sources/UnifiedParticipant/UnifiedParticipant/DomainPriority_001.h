/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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

#include "DomainPriorityInterface.h"
#include "ComponentExtendedInterface.h"
#include "ParticipantServicesInterface.h"

class DomainPriority_001 final : public DomainPriorityInterface,
    public ComponentExtendedInterface
{
public:

    DomainPriority_001(ParticipantServicesInterface* participantServicesInterface);

    // DomainPriorityInterface
    virtual DomainPriority getDomainPriority(UIntN participantIndex, UIntN domainIndex) override final;

    // ComponentExtendedInterface
    virtual void clearCachedData(void) override final;
    virtual XmlNode* getXml(UIntN domainIndex) override final;

private:

    ParticipantServicesInterface* m_participantServicesInterface;
    Bool m_cacheDataCleared;
    DomainPriority m_currentPriority;

    void updateCacheIfCleared(UIntN domainIndex);
    void updateCache(UIntN domainIndex);
};