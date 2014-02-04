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

void DisplayControlKnob::limit()
{
    if (canLimit())
    {
        try
        {
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Attempting to limit display brightness.", 
                getParticipantIndex(), getDomainIndex()));

            UIntN currentControlIndex = m_displayControl->getStatus().getBrightnessLimitIndex();
            UIntN lowerLimit = m_displayControl->getCapabilities().getCurrentLowerLimit();
            UIntN nextControlIndex = std::min(currentControlIndex + 1, lowerLimit);
            m_displayControl->setControl(nextControlIndex);
            m_hasBeenLimited = true;

            stringstream message;
            message << "Limited display brightness to control index" << nextControlIndex << ".";
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, message.str(), getParticipantIndex(), getDomainIndex()));
        }
        catch (std::exception& ex)
        {
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, ex.what(), getParticipantIndex(), getDomainIndex()));
            throw ex;
        }
    }
}

void DisplayControlKnob::unlimit()
{
    if (canUnlimit())
    {
        try
        {
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Attempting to unlimit display brightness.", 
                getParticipantIndex(), getDomainIndex()));

            UIntN currentControlIndex = m_displayControl->getStatus().getBrightnessLimitIndex();
            UIntN upperLimit = m_displayControl->getCapabilities().getCurrentUpperLimit();
            UIntN nextControlIndex = std::max(currentControlIndex - 1, upperLimit);
            m_displayControl->setControl(nextControlIndex);

            stringstream message;
            message << "Unlimited display brightness to control index " << nextControlIndex << ".";
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, message.str(), getParticipantIndex(), getDomainIndex()));
        }
        catch (std::exception& ex)
        {
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, ex.what(), getParticipantIndex(), getDomainIndex()));
            throw ex;
        }
    }
}

Bool DisplayControlKnob::canLimit()
{
    try
    {
        if (m_displayControl->supportsDisplayControls())
        {
            UIntN lowerLimitIndex = m_displayControl->getCapabilities().getCurrentLowerLimit();
            UIntN currentLimitIndex = m_displayControl->getStatus().getBrightnessLimitIndex();
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

Bool DisplayControlKnob::canUnlimit()
{
    try
    {
        if (m_displayControl->supportsDisplayControls() && (m_hasBeenLimited == true))
        {
            UIntN upperLimitIndex = m_displayControl->getCapabilities().getCurrentUpperLimit();
            UIntN currentLimitIndex = m_displayControl->getStatus().getBrightnessLimitIndex();
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