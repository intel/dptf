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

#include "PolicyServicesDomainTemperature.h"
#include "ParticipantManager.h"
#include "ManagerMessage.h"
#include "DptfManager.h"
#include "EsifServices.h"

PolicyServicesDomainTemperature::PolicyServicesDomainTemperature(DptfManager* dptfManager, UIntN policyIndex) :
    PolicyServices(dptfManager, policyIndex)
{
}

TemperatureStatus PolicyServicesDomainTemperature::getTemperatureStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->getTemperatureStatus(domainIndex);
}

TemperatureThresholds PolicyServicesDomainTemperature::getTemperatureThresholds(
    UIntN participantIndex, UIntN domainIndex)
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->getTemperatureThresholds(domainIndex);
}

void PolicyServicesDomainTemperature::setTemperatureThresholds(UIntN participantIndex,
    UIntN domainIndex, const TemperatureThresholds& temperatureThresholds)
{
    throwIfNotWorkItemThread();

#ifdef ONLY_LOG_TEMPERATURE_THRESHOLDS
    // Added to help debug issue with missing temperature threshold events
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF,
        "Policy is calling PolicyServicesDomainTemperature::setTemperatureThresholds().");
    message.addMessage("Aux0", temperatureThresholds.getAux0());
    message.addMessage("Aux1", temperatureThresholds.getAux1());
    message.setParticipantAndDomainIndex(participantIndex, domainIndex);
    message.setPolicyIndex(getPolicyIndex());
    getDptfManager()->getEsifServices()->writeMessageDebug(message, MessageCategory::TemperatureThresholds);
#endif

    getParticipantManager()->getParticipantPtr(participantIndex)->setTemperatureThresholds(
        domainIndex, getPolicyIndex(), temperatureThresholds);
}