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

#include "CoreControlFacade.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

CoreControlFacade::CoreControlFacade(
    UIntN participantIndex,
    UIntN domainIndex,
    const DomainProperties& domainProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : m_policyServices(policyServices),
    m_participantIndex(participantIndex),
    m_domainIndex(domainIndex),
    m_domainProperties(domainProperties),
    m_capabilities(participantIndex, domainIndex, domainProperties, policyServices),
    m_preferences(participantIndex, domainIndex, domainProperties, policyServices),
    m_controlsHaveBeenInitialized(false),
    m_lastSetCoreControlStatus(0)
{
}

CoreControlFacade::~CoreControlFacade()
{
}

Bool CoreControlFacade::supportsCoreControls()
{
    return m_domainProperties.implementsCoreControlInterface() && getPreferences().isLpoEnabled();
}

void CoreControlFacade::initializeControlsIfNeeded()
{
    if (supportsCoreControls())
    {
        m_policyServices.messageLogging->writeMessageDebug(
            PolicyMessage(FLF, "Core control initialization started."));
        CoreControlDynamicCaps caps = getDynamicCapabilities();
        if (m_controlsHaveBeenInitialized == false)
        {
            setActiveCoreControl(CoreControlStatus(caps.getMaxActiveCores()));
            m_controlsHaveBeenInitialized = true;
        }
        else
        {
            UIntN maxActiveCores = caps.getMaxActiveCores();
            UIntN minActiveCores = caps.getMinActiveCores();
            UIntN lastSetActiveCores = m_lastSetCoreControlStatus.getNumActiveLogicalProcessors();
            if (lastSetActiveCores > maxActiveCores)
            {
                m_policyServices.messageLogging->writeMessageDebug(
                    PolicyMessage(FLF, "Adjusting active core limit to minimum allowed."));
                setActiveCoreControl(CoreControlStatus(maxActiveCores));
            }
            else if (lastSetActiveCores < minActiveCores)
            {
                m_policyServices.messageLogging->writeMessageDebug(
                    PolicyMessage(FLF, "Adjusting active core limit to minimum allowed."));
                setActiveCoreControl(CoreControlStatus(minActiveCores));
            }
        }
        m_policyServices.messageLogging->writeMessageDebug(
            PolicyMessage(FLF, "Core control initialization finished."));
    }
}

void CoreControlFacade::setControlsToMax()
{
    if (supportsCoreControls())
    {
        CoreControlDynamicCaps caps = getDynamicCapabilities();
        setActiveCoreControl(CoreControlStatus(caps.getMaxActiveCores()));
    }
}

void CoreControlFacade::setActiveCoreControl(CoreControlStatus status)
{
    if (supportsCoreControls())
    {
        m_policyServices.domainCoreControl->setActiveCoreControl(
            m_participantIndex, m_domainIndex, status);
        m_lastSetCoreControlStatus = status;
    }
    else
    {
        throw dptf_exception("Domain does not support the core control interface.");
    }
}

CoreControlStatus CoreControlFacade::getStatus()
{
    initializeControlsIfNeeded();
    return m_lastSetCoreControlStatus;
}

CoreControlDynamicCaps CoreControlFacade::getDynamicCapabilities()
{
    return m_capabilities.getDynamicCaps();
}

CoreControlStaticCaps CoreControlFacade::getStaticCapabilities()
{
    return m_capabilities.getStaticCaps();
}

CoreControlLpoPreference CoreControlFacade::getPreferences()
{
    return m_preferences.getPreferences();
}

void CoreControlFacade::refreshCapabilities()
{
    m_capabilities.refresh();
}

void CoreControlFacade::refreshPreferences()
{
    m_preferences.refresh();
}

void CoreControlFacade::setValueWithinCapabilities()
{
    auto controlValue = m_lastSetCoreControlStatus.getNumActiveLogicalProcessors();
    controlValue = std::min(m_capabilities.getStaticCaps().getTotalLogicalProcessors(), controlValue);
    controlValue = std::max(m_capabilities.getDynamicCaps().getMinActiveCores(), controlValue);
    controlValue = std::min(m_capabilities.getDynamicCaps().getMaxActiveCores(), controlValue);
    setActiveCoreControl(CoreControlStatus(controlValue));
}

void CoreControlFacade::throwIfControlNotSupported()
{
    if (supportsCoreControls() == false)
    {
        throw dptf_exception("Cannot perform core control action because core controls \
                             are not supported on the domain.");
    }
}