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

#include "ConfigTdpControlFacade.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

ConfigTdpControlFacade::ConfigTdpControlFacade(
    UIntN participantIndex,
    UIntN domainIndex,
    const DomainProperties& domainProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : m_participantIndex(participantIndex),
    m_domainIndex(domainIndex),
    m_domainProperties(domainProperties),
    m_policyServices(policyServices),
    m_controlsHaveBeenInitialized(false),
    m_configTdpCapabilities(participantIndex, domainIndex, domainProperties, policyServices),
    m_configTdpStatus(participantIndex, domainIndex, domainProperties, policyServices),
    m_configTdpControlSet(participantIndex, domainIndex, domainProperties, policyServices)
{
}

ConfigTdpControlFacade::~ConfigTdpControlFacade()
{
}

void ConfigTdpControlFacade::setControl(UIntN configTdpControlIndex)
{
    if (supportsConfigTdpControls())
    {
        const ConfigTdpControlDynamicCaps& capabilities = m_configTdpCapabilities.getDynamicCaps();
        UIntN arbitratedConfigTdpIndex;
        if ((configTdpControlIndex >= capabilities.getCurrentUpperLimitIndex()) && 
            (configTdpControlIndex <= capabilities.getCurrentLowerLimitIndex()))
        {
            arbitratedConfigTdpIndex = configTdpControlIndex;
        }
        else if (configTdpControlIndex < capabilities.getCurrentUpperLimitIndex())
        {
            arbitratedConfigTdpIndex = capabilities.getCurrentUpperLimitIndex();
        }
        else if (configTdpControlIndex > capabilities.getCurrentLowerLimitIndex())
        {
            arbitratedConfigTdpIndex = capabilities.getCurrentLowerLimitIndex();
        }
        else
        {
            arbitratedConfigTdpIndex = capabilities.getCurrentUpperLimitIndex();
        }

        m_policyServices.domainConfigTdpControl->setConfigTdpControl(
            m_participantIndex, m_domainIndex, arbitratedConfigTdpIndex);
        m_configTdpStatus.invalidate();
    }
    else
    {
        throw dptf_exception("Domain does not support the ConfigTDP control interface.");
    }
}

void ConfigTdpControlFacade::refreshCapabilities()
{
    m_configTdpCapabilities.refresh();
}

void ConfigTdpControlFacade::initializeControlsIfNeeded()
{
    if (supportsConfigTdpControls() && (m_controlsHaveBeenInitialized == false))
    {
        ConfigTdpControlDynamicCaps caps = getCapabilities();
        setControl(caps.getCurrentUpperLimitIndex());
        m_controlsHaveBeenInitialized = true;
    }
}

Bool ConfigTdpControlFacade::supportsConfigTdpControls()
{
    return m_domainProperties.implementsConfigTdpControlInterface();
}

ConfigTdpControlDynamicCaps ConfigTdpControlFacade::getCapabilities()
{
    return m_configTdpCapabilities.getDynamicCaps();
}

ConfigTdpControlStatus ConfigTdpControlFacade::getStatus()
{
    return m_configTdpStatus.getStatus();
}

ConfigTdpControlSet ConfigTdpControlFacade::getControlSet()
{
    return m_configTdpControlSet.getControlSet();
}

XmlNode* ConfigTdpControlFacade::getXml()
{
    XmlNode* status = XmlNode::createWrapperElement("control_config_tdp_level");
    if (supportsConfigTdpControls())
    {
        try
        {
            status->addChild(getStatus().getXml());
        }
        catch (...)
        {
            status->addChild(ConfigTdpControlStatus(Constants::Invalid).getXml());
        }
        
        try
        {
            status->addChild(getCapabilities().getXml());
        }
        catch (...)
        {
            status->addChild(ConfigTdpControlDynamicCaps(Constants::Invalid, Constants::Invalid).getXml());
        }

        try
        {
            status->addChild(getControlSet().getXml());
        }
        catch (...)
        {
            status->addChild(ConfigTdpControlSet(vector<ConfigTdpControl>(1, 
                ConfigTdpControl(Constants::Invalid, Constants::Invalid, Constants::Invalid, Constants::Invalid))).getXml());
        }
    }
    return status;
}