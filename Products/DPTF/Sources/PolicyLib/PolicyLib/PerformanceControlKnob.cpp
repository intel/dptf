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

#include "PerformanceControlKnob.h"
using namespace std;


PerformanceControlKnob::PerformanceControlKnob(
    const PolicyServicesInterfaceContainer& policyServices,
    UIntN participantIndex,
    UIntN domainIndex,
    shared_ptr<PerformanceControlFacade> performanceControl,
    PerformanceControlType::Type controlType)
    : ControlKnobBase(policyServices, participantIndex, domainIndex),
    m_performanceControl(performanceControl),
    m_controlType(controlType)
{
}

PerformanceControlKnob::~PerformanceControlKnob(void)
{
}

void PerformanceControlKnob::limit()
{
    if (canLimit())
    {
        try
        {
            stringstream messageBefore;
            messageBefore << "Attempting to limit " << controlTypeToString(m_controlType) << "s.";
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, messageBefore.str(), getParticipantIndex(), getDomainIndex()));

            const PerformanceControlDynamicCaps& dynamicCapabilities = m_performanceControl->getDynamicCapabilities();
            UIntN lowerLimitIndex = dynamicCapabilities.getCurrentLowerLimitIndex();
            UIntN currentIndex = m_performanceControl->getStatus().getCurrentControlSetIndex();
            UIntN nextIndex = std::min(currentIndex + 1, lowerLimitIndex);
            m_performanceControl->setControl(nextIndex);

            stringstream messageAfter;
            messageAfter 
                << "Limited performance state to " 
                << nextIndex 
                << "(" << controlTypeToString(m_controlType) << ").";
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, messageAfter.str(), getParticipantIndex(), getDomainIndex()));
        }
        catch (std::exception& ex)
        {
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, ex.what(), getParticipantIndex(), getDomainIndex()));
            throw ex;
        }
    }
}

void PerformanceControlKnob::unlimit()
{
    if (canUnlimit())
    {
        try
        {
            stringstream messageBefore;
            messageBefore << "Attempting to unlimit " << controlTypeToString(m_controlType) << "s.";
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, messageBefore.str(), getParticipantIndex(), getDomainIndex()));

            const PerformanceControlDynamicCaps& dynamicCapabilities = m_performanceControl->getDynamicCapabilities();
            UIntN upperLimitIndex = dynamicCapabilities.getCurrentUpperLimitIndex();
            UIntN currentIndex = m_performanceControl->getStatus().getCurrentControlSetIndex();
            UIntN nextIndex = std::max(currentIndex - 1, upperLimitIndex);
            m_performanceControl->setControl(nextIndex);

            stringstream messageAfter;
            messageAfter 
                << "Unlimited performance state to " 
                << nextIndex 
                << "(" << controlTypeToString(m_controlType) << ").";
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, messageAfter.str(), getParticipantIndex(), getDomainIndex()));
        }
        catch (std::exception& ex)
        {
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, ex.what(), getParticipantIndex(), getDomainIndex()));
            throw ex;
        }
    }
}

Bool PerformanceControlKnob::canLimit()
{
    try
    {
        if (m_performanceControl->supportsPerformanceControls())
        {
            const PerformanceControlDynamicCaps& dynamicCapabilities = m_performanceControl->getDynamicCapabilities();
            UIntN lowerLimitIndex = dynamicCapabilities.getCurrentLowerLimitIndex();
            UIntN currentIndex = m_performanceControl->getStatus().getCurrentControlSetIndex();
            if (currentIndex >= lowerLimitIndex)
            {
                return false;
            }

            PerformanceControlType::Type nextControlType =
                m_performanceControl->getControls()[currentIndex + 1].getPerformanceControlType();
            return (nextControlType == m_controlType);
        }
        else
        {
            return false;
        }
    }
    catch (...)
    {
        return false;
    }
}

Bool PerformanceControlKnob::canUnlimit()
{
    try
    {
        if (m_performanceControl->supportsPerformanceControls())
        {
            const PerformanceControlDynamicCaps& dynamicCapabilities = m_performanceControl->getDynamicCapabilities();
            UIntN upperLimitIndex = dynamicCapabilities.getCurrentUpperLimitIndex();
            UIntN currentIndex = m_performanceControl->getStatus().getCurrentControlSetIndex();
            if (currentIndex <= upperLimitIndex)
            {
                return false;
            }

            PerformanceControlType::Type nextControlType =
                m_performanceControl->getControls()[currentIndex - 1].getPerformanceControlType();
            return (nextControlType == m_controlType);
        }
        else
        {
            return false;
        }
    }
    catch (...)
    {
        return false;
    }
}

string PerformanceControlKnob::controlTypeToString(PerformanceControlType::Type controlType)
{
    switch (controlType)
    {
        case PerformanceControlType::PerformanceState:
            return "P-State";
        case PerformanceControlType::ThrottleState:
            return "T-State";
        default:
            return "Unknown-Type";
    }
}

XmlNode* PerformanceControlKnob::getXml()
{
    XmlNode* status = XmlNode::createWrapperElement("performance_control_status");
    if (m_performanceControl->supportsPerformanceControls())
    {
        status->addChild(XmlNode::createDataElement("type", controlTypeToString(m_controlType)));
        auto dynamicCapabilities = m_performanceControl->getDynamicCapabilities();
        status->addChild(dynamicCapabilities.getXml());
        auto currentStatus = m_performanceControl->getStatus();
        status->addChild(currentStatus.getXml());
    }
    return status;
}