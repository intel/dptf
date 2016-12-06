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
#include "DomainTemperatureBase.h"

class DomainTemperature_002 : public DomainTemperatureBase
{
public:

    DomainTemperature_002(UIntN participantIndex, UIntN domainIndex, 
        std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
    virtual ~DomainTemperature_002(void);

    // DomainTemperatureInterface
    virtual TemperatureStatus getTemperatureStatus(UIntN participantIndex, UIntN domainIndex) override;
    virtual DptfBuffer getCalibrationTable(UIntN participantIndex, UIntN domainIndex) override;
    virtual DptfBuffer getPollingTable(UIntN participantIndex, UIntN domainIndex) override;
    virtual Bool isVirtualTemperature(UIntN participantIndex, UIntN domainIndex) override;
    virtual void setVirtualTemperature(UIntN participantIndex, UIntN domainIndex, const Temperature& temperature) override;

    // ComponentExtendedInterface
    virtual void clearCachedData(void) override;
    virtual std::string getName(void) override;
    virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

private:

    // hide the copy constructor and = operator
    DomainTemperature_002(const DomainTemperature_002& rhs);
    DomainTemperature_002& operator=(const DomainTemperature_002& rhs);
    
    void createCalibrationTableBuffer(UIntN domainIndex);
    void createPollingTableBuffer(UIntN domainIndex);

    Temperature m_lastSetTemperature;
    DptfBuffer m_pollingTableBuffer;
    DptfBuffer m_calibrationTableBuffer;
};