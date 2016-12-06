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

#pragma once

#include "Dptf.h"
#include "DomainPixelClockControlBase.h"

class DomainPixelClockControl_001 : public DomainPixelClockControlBase
{
public:

    DomainPixelClockControl_001(UIntN participantIndex, UIntN domainIndex, 
        std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
    virtual ~DomainPixelClockControl_001(void);

    // DomainPixelClockControlInterface
    virtual void setPixelClockControl(UIntN participantIndex, UIntN domainIndex,
        const PixelClockDataSet& pixelClockDataSet) override;

    // ParticipantActivityLoggingInterface
    virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

    // ComponentExtendedInterface
    virtual void clearCachedData(void) override;
    virtual std::string getName(void) override;
    virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

private:

    // hide the copy constructor and = operator
    DomainPixelClockControl_001(const DomainPixelClockControl_001& rhs);
    DomainPixelClockControl_001& operator=(const DomainPixelClockControl_001& rhs);
};