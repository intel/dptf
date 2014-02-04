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

#include "DomainPixelClockStatusInterface.h"
#include "ComponentExtendedInterface.h"
#include "ParticipantServicesInterface.h"

class DomainPixelClockStatus_001 final : public DomainPixelClockStatusInterface,
    public ComponentExtendedInterface
{
public:

    DomainPixelClockStatus_001(ParticipantServicesInterface* participantServicesInterface);
    ~DomainPixelClockStatus_001(void);

    // DomainPixelClockStatusInterface
    virtual PixelClockCapabilities getPixelClockCapabilities(UIntN participantIndex, UIntN domainIndex) override final;
    virtual PixelClockDataSet getPixelClockDataSet(UIntN participantIndex, UIntN domainIndex) override final;

    // ComponentExtendedInterface
    virtual void clearCachedData(void) override final;
    virtual XmlNode* getXml(UIntN domainIndex) override final;

private:

    // hide the copy constructor and = operator
    DomainPixelClockStatus_001(const DomainPixelClockStatus_001& rhs);
    DomainPixelClockStatus_001& operator=(const DomainPixelClockStatus_001& rhs);

    ParticipantServicesInterface* m_participantServicesInterface;
};