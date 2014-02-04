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

#include "DisplayControlFacade.h"
using namespace std;

DisplayControlFacade::DisplayControlFacade(
    UIntN participantIndex,
    UIntN domainIndex,
    const DomainProperties& domainProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : m_participantIndex(participantIndex),
    m_domainIndex(domainIndex),
    m_domainProperties(domainProperties),
    m_policyServices(policyServices),
    m_displayControlSetProperty(participantIndex, domainIndex, domainProperties, policyServices),
    m_displayControlStatusProperty(participantIndex, domainIndex, domainProperties, policyServices),
    m_displayControlCapabilitiesProperty(participantIndex, domainIndex, domainProperties, policyServices)
{
}

DisplayControlFacade::~DisplayControlFacade()
{
}

Bool DisplayControlFacade::supportsDisplayControls()
{
    return m_domainProperties.implementsDisplayControlInterface();
}

const DisplayControlStatus& DisplayControlFacade::getStatus()
{
    return m_displayControlStatusProperty.getStatus();
}

void DisplayControlFacade::setControl(UIntN displayControlIndex)
{
    if (supportsDisplayControls())
    {
        m_policyServices.domainDisplayControl->setDisplayControl(
            m_participantIndex, m_domainIndex, displayControlIndex, true);
        m_displayControlStatusProperty.invalidate();
    }
    else
    {
        throw dptf_exception("Domain does not support the display control interface.");
    }
}

const DisplayControlSet& DisplayControlFacade::getControls()
{
    return m_displayControlSetProperty.getControls();
}

void DisplayControlFacade::invalidateControlSet()
{
    m_displayControlSetProperty.invalidate();
    m_displayControlStatusProperty.invalidate();
}

void DisplayControlFacade::refreshCapabilities()
{
    m_displayControlCapabilitiesProperty.refresh();
}

const DisplayControlDynamicCaps& DisplayControlFacade::getCapabilities()
{
    return m_displayControlCapabilitiesProperty.getCapabilities();
}

void DisplayControlFacade::invalidateStatus()
{
    m_displayControlStatusProperty.invalidate();
}