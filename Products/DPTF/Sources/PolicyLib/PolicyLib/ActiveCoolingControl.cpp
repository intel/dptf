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

#include "ActiveCoolingControl.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

ActiveCoolingControl::ActiveCoolingControl(
    UIntN participantIndex,
    UIntN domainIndex,
    const DomainProperties& domainProperties,
    const ParticipantProperties& participantProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : m_policyServices(policyServices),
    m_participantIndex(participantIndex),
    m_domainIndex(domainIndex),
    m_domainProperties(domainProperties),
    m_participantProperties(participantProperties),
    m_staticCaps(participantIndex, domainIndex, domainProperties, policyServices),
    m_lastFanSpeedRequest(Percentage::createInvalid()),
    m_lastFanSpeedRequestIndex(Constants::Invalid)
{
}

ActiveCoolingControl::~ActiveCoolingControl(void)
{
}

Bool ActiveCoolingControl::supportsActiveCoolingControls()
{
    return m_domainProperties.implementsActiveControlInterface();
}

Bool ActiveCoolingControl::supportsFineGrainControl()
{
    if (supportsActiveCoolingControls())
    {
        return m_staticCaps.getCapabilities().supportsFineGrainedControl();
    }
    return false;
}

void ActiveCoolingControl::requestFanSpeedPercentage(UIntN requestorIndex, const Percentage& fanSpeed)
{
    if (supportsFineGrainControl())
    {
        updateFanSpeedRequestTable(requestorIndex, fanSpeed);
        Percentage highestFanSpeed = chooseHighestFanSpeedRequest();
        if ((m_lastFanSpeedRequest.isValid() == false) || (highestFanSpeed != m_lastFanSpeedRequest))
        {
            m_policyServices.domainActiveControl->setActiveControl(
                m_participantIndex, m_domainIndex, highestFanSpeed);
            m_lastFanSpeedRequest = highestFanSpeed;
        }
    }
}

void ActiveCoolingControl::requestActiveControlIndex(UIntN requestorIndex, UIntN activeControlIndex)
{
    if (supportsActiveCoolingControls() && (supportsFineGrainControl() == false))
    {
        updateActiveControlRequestTable(requestorIndex, activeControlIndex);
        UIntN highestActiveControlIndex = chooseHighestActiveControlIndex();
        if (highestActiveControlIndex != m_lastFanSpeedRequestIndex)
        {
            m_policyServices.domainActiveControl->setActiveControl(
                m_participantIndex,
                m_domainIndex,
                highestActiveControlIndex);
            m_lastFanSpeedRequestIndex = highestActiveControlIndex;
        }
    }
}

void ActiveCoolingControl::updateFanSpeedRequestTable(UIntN requestorIndex, const Percentage& fanSpeed)
{
    if (m_fanSpeedRequestTable.count(requestorIndex))
    {
        m_fanSpeedRequestTable.at(requestorIndex) = fanSpeed;
    }
    else
    {
        m_fanSpeedRequestTable.insert(pair<UIntN, Percentage>(requestorIndex, fanSpeed));
    }
}

Percentage ActiveCoolingControl::chooseHighestFanSpeedRequest()
{
    Percentage highestFanSpeed(0.0);
    for (auto request = m_fanSpeedRequestTable.begin(); request != m_fanSpeedRequestTable.end(); request++)
    {
        if (request->second > highestFanSpeed)
        {
            highestFanSpeed = request->second;
        }
    }
    return highestFanSpeed;
}

void ActiveCoolingControl::updateActiveControlRequestTable(UIntN requestorIndex, UIntN activeControlIndex)
{
    if (m_activeControlRequestTable.count(requestorIndex))
    {
        m_activeControlRequestTable.at(requestorIndex) = activeControlIndex;
    }
    else
    {
        m_activeControlRequestTable.insert(pair<UIntN, UIntN>(requestorIndex, activeControlIndex));
    }
}

UIntN ActiveCoolingControl::chooseHighestActiveControlIndex()
{
    UIntN highestActiveControlIndex = ActiveRelationshipTableEntry::FanOffIndex;
    for (auto request = m_activeControlRequestTable.begin(); request != m_activeControlRequestTable.end(); request++)
    {
        if (request->second < highestActiveControlIndex)
        {
            highestActiveControlIndex = request->second;
        }
    }
    return highestActiveControlIndex;
}

void ActiveCoolingControl::forceFanOff(void)
{
    if (supportsActiveCoolingControls())
    {
        if (supportsFineGrainControl())
        {
            m_policyServices.domainActiveControl->setActiveControl(
                m_participantIndex, m_domainIndex, Percentage(0.0));
            m_lastFanSpeedRequest = Percentage(0.0);
        }
        else
        {
            m_policyServices.domainActiveControl->setActiveControl(
                m_participantIndex, m_domainIndex, ActiveRelationshipTableEntry::FanOffIndex);
            m_lastFanSpeedRequestIndex = ActiveRelationshipTableEntry::FanOffIndex;
        }
    }
}

XmlNode* ActiveCoolingControl::getXml()
{
    XmlNode* status = XmlNode::createWrapperElement("active_cooling_control");
    status->addChild(XmlNode::createDataElement("participant_index", friendlyValue(m_participantIndex)));
    status->addChild(XmlNode::createDataElement("domain_index", friendlyValue(m_domainIndex)));
    status->addChild(XmlNode::createDataElement("name", m_participantProperties.getAcpiInfo().getAcpiScope()));
    if (supportsFineGrainControl())
    {
        status->addChild(XmlNode::createDataElement("speed", chooseHighestFanSpeedRequest().toString()));
    }
    else
    {
        status->addChild(XmlNode::createDataElement("speed", friendlyValue(chooseHighestActiveControlIndex())));
    }
    status->addChild(XmlNode::createDataElement("fine_grain", friendlyValue(supportsFineGrainControl())));
    return status;
}