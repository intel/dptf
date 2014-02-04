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

#include "RadioFrequencyControlFacade.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

RadioFrequencyControlFacade::RadioFrequencyControlFacade(
    UIntN participantIndex,
    UIntN domainIndex,
    const DomainProperties& domainProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : m_participantIndex(participantIndex),
    m_domainIndex(domainIndex),
    m_domainProperties(domainProperties),
    m_policyServices(policyServices),
    m_controlsHaveBeenInitialized(false),
    m_rfProfileData(participantIndex, domainIndex, domainProperties, policyServices)
{
}

RadioFrequencyControlFacade::~RadioFrequencyControlFacade()
{
}

void RadioFrequencyControlFacade::initializeControlsIfNeeded()
{
    if (m_controlsHaveBeenInitialized == false)
    {
        m_controlsHaveBeenInitialized = true;
    }
}

Bool RadioFrequencyControlFacade::supportsControl()
{
    return m_domainProperties.implementsRfProfileControlInterface();
}

Bool RadioFrequencyControlFacade::supportsStatus()
{
    return m_domainProperties.implementsRfProfileStatusInterface();
}

RfProfileData RadioFrequencyControlFacade::getProfileData()
{
    throwIfStatusNotSupported();
    return m_rfProfileData.getProfileData();
}

void RadioFrequencyControlFacade::setOperatingFrequency(Frequency frequency)
{
    throwIfControlNotSupported();
    m_policyServices.domainRfProfileControl->setRfProfileCenterFrequency(m_participantIndex, m_domainIndex, frequency);
}

XmlNode* RadioFrequencyControlFacade::getXml()
{
    XmlNode* control = XmlNode::createWrapperElement("radio_frequency_control");
    control->addChild(XmlNode::createDataElement("supports_status_controls", supportsStatus() ? "true" : "false"));
    control->addChild(XmlNode::createDataElement("supports_set_controls", supportsControl() ? "true" : "false"));
    if (supportsStatus())
    {
        control->addChild(getProfileData().getXml());
    }
    return control;
}

void RadioFrequencyControlFacade::throwIfStatusNotSupported()
{
    if (supportsStatus() == false)
    {
        throw dptf_exception("Radio frequency status is not supported.");
    }
}

void RadioFrequencyControlFacade::throwIfControlNotSupported()
{
    if (supportsControl() == false)
    {
        throw dptf_exception("Radio frequency control is not supported.");
    }
}