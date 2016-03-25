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

class DomainTemperature_001 : public DomainTemperatureBase
{
public:

    DomainTemperature_001(UIntN participantIndex, UIntN domainIndex, 
        ParticipantServicesInterface* participantServicesInterface);

    // DomainTemperatureInterface
    virtual TemperatureStatus getTemperatureStatus(UIntN participantIndex, UIntN domainIndex) override;
    virtual TemperatureThresholds getTemperatureThresholds(UIntN participantIndex, UIntN domainIndex) override;
    virtual void setTemperatureThresholds(UIntN participantIndex, UIntN domainIndex,
        const TemperatureThresholds& temperatureThresholds) override;
    virtual DptfBuffer getCalibrationTable(UIntN participantIndex, UIntN domainIndex) override;
    virtual DptfBuffer getPollingTable(UIntN participantIndex, UIntN domainIndex) override;
    virtual Bool isVirtualTemperature(UIntN participantIndex, UIntN domainIndex) override;
    virtual void setVirtualTemperature(UIntN participantIndex, UIntN domainIndex, const Temperature& temperature) override;

    // ParticipantActivityLoggingInterface
    virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

    // ComponentExtendedInterface
    virtual void clearCachedData(void) override;
    virtual std::string getName(void) override;
    virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

private:

    // hide the copy constructor and = operator
    DomainTemperature_001(const DomainTemperature_001& rhs);
    DomainTemperature_001& operator=(const DomainTemperature_001& rhs);

    Temperature getAuxTemperatureThreshold(UIntN domainIndex, UInt8 auxNumber);
    Temperature getHysteresis(UIntN domainIndex);
};