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

#include "DomainTemperatureInterface.h"
#include "ComponentExtendedInterface.h"
#include "ParticipantServicesInterface.h"

class DomainTemperature_001 final : public DomainTemperatureInterface,
    public ComponentExtendedInterface
{
public:

    DomainTemperature_001(ParticipantServicesInterface* participantServicesInterface);

    // DomainTemperatureInterface
    virtual TemperatureStatus getTemperatureStatus(UIntN participantIndex, UIntN domainIndex) override final;
    virtual TemperatureThresholds getTemperatureThresholds(UIntN participantIndex, UIntN domainIndex) override final;
    virtual void setTemperatureThresholds(UIntN participantIndex, UIntN domainIndex,
        const TemperatureThresholds& temperatureThresholds) override final;

    // ComponentExtendedInterface
    virtual void clearCachedData(void) override final;
    virtual XmlNode* getXml(UIntN domainIndex) override final;

private:

    ParticipantServicesInterface* m_participantServicesInterface;

    // hide the copy constructor and = operator
    DomainTemperature_001(const DomainTemperature_001& rhs);
    DomainTemperature_001& operator=(const DomainTemperature_001& rhs);

    Temperature getAuxTemperatureThreshold(UIntN domainIndex, UInt8 auxNumber);
    UIntN getHysteresis(UIntN domainIndex);
};