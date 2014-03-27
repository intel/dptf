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

#include "DisplayControlKnob.h"

using namespace std;

DisplayControlKnob::DisplayControlKnob(
    const PolicyServicesInterfaceContainer& policyServices,
    std::shared_ptr<DisplayControlFacade> displayControl,
    UIntN participantIndex,
    UIntN domainIndex)
    : ControlKnobBase(policyServices, participantIndex, domainIndex),
    m_displayControl(displayControl),
    m_hasBeenLimited(false)
{
}

DisplayControlKnob::~DisplayControlKnob(void)
{
}

void DisplayControlKnob::limit(UIntN target)
{
    if (canLimit(target))
    {
        try
        {
            stringstream messageBefore;
            messageBefore << "Calculating request to limit display brightness.";
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, messageBefore.str(), getParticipantIndex(), getDomainIndex()));

            UIntN currentControlIndex = getTargetRequest(target);
            if (currentControlIndex == m_displayControl->getCapabilities().getCurrentUpperLimit())
            {
                currentControlIndex = m_displayControl->getStatus().getBrightnessLimitIndex();
            }
            UIntN lowerLimit = m_displayControl->getCapabilities().getCurrentLowerLimit();
            UIntN nextControlIndex = std::min(currentControlIndex + 1, lowerLimit);
            m_requests[target] = nextControlIndex;

            stringstream messageAfter;
            messageAfter << "Requesting to limit display brightness to" << nextControlIndex << ".";
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, messageAfter.str(), getParticipantIndex(), getDomainIndex()));
        }
        catch (std::exception& ex)
        {
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, ex.what(), getParticipantIndex(), getDomainIndex()));
            throw ex;
        }
    }
}

void DisplayControlKnob::unlimit(UIntN target)
{
    if (canUnlimit(target))
    {
        try
        {
            stringstream messageBefore;
            messageBefore << "Calculating request to unlimit display brightness.";
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, messageBefore.str(), getParticipantIndex(), getDomainIndex()));

            UIntN currentControlIndex = getTargetRequest(target);
            UIntN upperLimit = m_displayControl->getCapabilities().getCurrentUpperLimit();
            UIntN nextControlIndex = std::max(currentControlIndex - 1, upperLimit);
            m_requests[target] = nextControlIndex;

            stringstream messageAfter;
            messageAfter << "Requesting to unlimit display brightness to" << nextControlIndex << ".";
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, messageAfter.str(), getParticipantIndex(), getDomainIndex()));
        }
        catch (std::exception& ex)
        {
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, ex.what(), getParticipantIndex(), getDomainIndex()));
            throw ex;
        }
    }
}

Bool DisplayControlKnob::canLimit(UIntN target)
{
    try
    {
        if (m_displayControl->supportsDisplayControls())
        {
            UIntN lowerLimitIndex = m_displayControl->getCapabilities().getCurrentLowerLimit();
            UIntN currentLimitIndex = getTargetRequest(target);
            return (currentLimitIndex < lowerLimitIndex);
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

Bool DisplayControlKnob::canUnlimit(UIntN target)
{
    try
    {
        if (m_displayControl->supportsDisplayControls() && (m_hasBeenLimited == true))
        {
            UIntN upperLimitIndex = m_displayControl->getCapabilities().getCurrentUpperLimit();
            UIntN currentLimitIndex = getTargetRequest(target);
            return (currentLimitIndex > upperLimitIndex);
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

Bool DisplayControlKnob::commitSetting()
{
    try
    {
        if (m_displayControl->supportsDisplayControls())
        {
            UIntN nextIndex = snapToCapabilitiesBounds(findHighestDisplayIndexRequest());
            if (m_displayControl->getStatus().getBrightnessLimitIndex() != nextIndex)
            {
                stringstream messageBefore;
                messageBefore << "Attempting to change display brightness limit to " << nextIndex << ".";
                getPolicyServices().messageLogging->writeMessageDebug(
                    PolicyMessage(FLF, messageBefore.str(), getParticipantIndex(), getDomainIndex()));

                m_displayControl->setControl(nextIndex);
                m_hasBeenLimited = true;

                stringstream messageAfter;
                messageAfter << "Changed display brightness limit to " << nextIndex << ".";
                getPolicyServices().messageLogging->writeMessageDebug(
                    PolicyMessage(FLF, messageAfter.str(), getParticipantIndex(), getDomainIndex()));
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    catch (std::exception& ex)
    {
        getPolicyServices().messageLogging->writeMessageDebug(
            PolicyMessage(FLF, ex.what(), getParticipantIndex(), getDomainIndex()));
        throw ex;
    }
}

UIntN DisplayControlKnob::findHighestDisplayIndexRequest() const
{
    UIntN highestIndex(m_displayControl->getCapabilities().getCurrentUpperLimit());
    for (auto request = m_requests.begin(); request != m_requests.end(); request++)
    {
        if (request->second > highestIndex)
        {
            highestIndex = request->second;
        }
    }
    return highestIndex;
}

UIntN DisplayControlKnob::snapToCapabilitiesBounds(UIntN displayIndex)
{
    UIntN upperLimit(m_displayControl->getCapabilities().getCurrentUpperLimit());
    UIntN lowerLimit(m_displayControl->getCapabilities().getCurrentLowerLimit());
    if (displayIndex < upperLimit)
    {
        displayIndex = upperLimit;
    }
    if (displayIndex > lowerLimit)
    {
        displayIndex = lowerLimit;
    }
    return displayIndex;
}

UIntN DisplayControlKnob::getTargetRequest(UIntN target) const
{
    auto request = m_requests.find(target);
    if (request == m_requests.end())
    {
        return m_displayControl->getCapabilities().getCurrentUpperLimit();
    }
    else
    {
        return request->second;
    }
}

void DisplayControlKnob::clearRequestForTarget(UIntN target)
{
    auto targetRequest = m_requests.find(target);
    if (targetRequest != m_requests.end())
    {
        m_requests.erase(targetRequest);
    }
}

void DisplayControlKnob::clearAllRequests()
{
    m_requests.clear();
}

XmlNode* DisplayControlKnob::getXml()
{
    XmlNode* status = XmlNode::createWrapperElement("display_control_knob_status");
    if (m_displayControl->supportsDisplayControls())
    {
        auto displayStatus = m_displayControl->getStatus();
        status->addChild(displayStatus.getXml());
        auto displayCapabilities = m_displayControl->getStatus();
        status->addChild(displayCapabilities.getXml());
    }
    return status;
}