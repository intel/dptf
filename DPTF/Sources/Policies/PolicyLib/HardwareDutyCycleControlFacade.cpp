/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

#include "HardwareDutyCycleControlFacade.h"
using namespace std;


HardwareDutyCycleControlFacade::HardwareDutyCycleControlFacade(
    UIntN participantIndex, UIntN domainIndex, const DomainProperties& domainProperties,
    const ParticipantProperties& participantProperties, const PolicyServicesInterfaceContainer& policyServices) :
    m_policyServices(policyServices), m_domainProperties(domainProperties),
    m_participantProperties(participantProperties), m_participantIndex(participantIndex), m_domainIndex(domainIndex)
{

}

HardwareDutyCycleControlFacade::~HardwareDutyCycleControlFacade(void)
{

}

DptfBuffer HardwareDutyCycleControlFacade::getHardwareDutyCycleUtilizationSet() const
{
    return m_policyServices.domainHardwareDutyCycleControl->getHardwareDutyCycleUtilizationSet(
        m_participantIndex, m_domainIndex);
}

Bool HardwareDutyCycleControlFacade::isEnabledByPlatform() const
{
    return m_policyServices.domainHardwareDutyCycleControl->isEnabledByPlatform(
        m_participantIndex, m_domainIndex);
}

Bool HardwareDutyCycleControlFacade::isSupportedByPlatform() const
{
    return m_policyServices.domainHardwareDutyCycleControl->isSupportedByPlatform(
        m_participantIndex, m_domainIndex);
}

Bool HardwareDutyCycleControlFacade::isEnabledByOperatingSystem() const
{
    return m_policyServices.domainHardwareDutyCycleControl->isEnabledByOperatingSystem(
        m_participantIndex, m_domainIndex);
}

Bool HardwareDutyCycleControlFacade::isSupportedByOperatingSystem() const
{
    return m_policyServices.domainHardwareDutyCycleControl->isSupportedByOperatingSystem(
        m_participantIndex, m_domainIndex);
}

Bool HardwareDutyCycleControlFacade::isHdcOobEnabled() const
{
    return m_policyServices.domainHardwareDutyCycleControl->isHdcOobEnabled(
        m_participantIndex, m_domainIndex);
}

void HardwareDutyCycleControlFacade::setHdcOobEnable(const UInt8& hdcOobEnable)
{
    return m_policyServices.domainHardwareDutyCycleControl->setHdcOobEnable(
        m_participantIndex, m_domainIndex, hdcOobEnable);
}

void HardwareDutyCycleControlFacade::setHardwareDutyCycle(const Percentage& dutyCycle)
{
    m_policyServices.domainHardwareDutyCycleControl->setHardwareDutyCycle(
        m_participantIndex, m_domainIndex, dutyCycle);
}

Percentage HardwareDutyCycleControlFacade::getHardwareDutyCycle() const
{
    return m_policyServices.domainHardwareDutyCycleControl->getHardwareDutyCycle(
        m_participantIndex, m_domainIndex);
}

Bool HardwareDutyCycleControlFacade::supportsHdcControls() const
{
    return m_domainProperties.implementsHardwareDutyCycleControlInterface();
}
