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

#include "PassiveDomainControlStatus.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

PassiveDomainControlStatus::PassiveDomainControlStatus(DomainProxy& domain)
    : m_participantIndex(Constants::Invalid),
    m_domainIndex(Constants::Invalid),
    m_domainName(""),
    m_domainTemperature(Temperature::createInvalid()),
    m_domainPriority(Constants::Invalid),
    m_domainUtilization(Percentage::createInvalid())
{
    aquireDomainStatus(domain);
    addPowerStatus(domain);
    addPstateStatus(domain);
    addCoreStatus(domain);
    addTstateStatus(domain);
    addDisplayStatus(domain);
}

XmlNode* PassiveDomainControlStatus::getXml()
{
    XmlNode* domainControlStatus = XmlNode::createWrapperElement("domain_control_status");
    domainControlStatus->addChild(XmlNode::createDataElement("index", friendlyValue(m_domainIndex)));
    domainControlStatus->addChild(XmlNode::createDataElement("name", m_domainName));
    domainControlStatus->addChild(
        XmlNode::createDataElement("temperature", m_domainTemperature.toString()));
    domainControlStatus->addChild(m_domainUtilization.getXml("utilization"));
    domainControlStatus->addChild(
        XmlNode::createDataElement("priority",
            friendlyValue(m_domainPriority.getCurrentPriority())));
    XmlNode* controls = XmlNode::createWrapperElement("controls");
    for (auto control = m_controlStatus.begin(); control != m_controlStatus.end(); control++)
    {
        controls->addChild(control->getXml());
    }
    domainControlStatus->addChild(controls);
    return domainControlStatus;
}

void PassiveDomainControlStatus::addPstateStatus(DomainProxy& domain)
{
    PerformanceControlFacade& perfControl = domain.getPerformanceControl();
    if (perfControl.supportsPerformanceControls())
    {
        // get current p-state
        UIntN currentIndex(Constants::Invalid);
        try
        {
            currentIndex = perfControl.getStatus().getCurrentControlSetIndex();
        }
        catch (...)
        {
        }

        // get index of first t-state
        UIntN firstTstateIndex(Constants::Invalid);
        try
        {
            firstTstateIndex =
                indexOfFirstControlWithType(perfControl.getControls(), PerformanceControlType::ThrottleState);
        }
        catch (...)
        {
        }

        // get capabilities
        PerformanceControlDynamicCaps dynamicCapabilities(Constants::Invalid, Constants::Invalid);
        try
        {
            dynamicCapabilities = perfControl.getDynamicCapabilities();
        }
        catch (...)
        {
        }

        // get the upper limit index
        UIntN upperLimitIndex = dynamicCapabilities.getCurrentUpperLimitIndex();

        // get the lower limit index
        UIntN lowerLimitIndex;
        if (firstTstateIndex != Constants::Invalid)
        {
            lowerLimitIndex = std::min(firstTstateIndex - 1, dynamicCapabilities.getCurrentLowerLimitIndex());
        }
        else
        {
            lowerLimitIndex = dynamicCapabilities.getCurrentLowerLimitIndex();
        }

        // add the control status to the list
        m_controlStatus.push_back(
            ControlStatus(
                "P-States",
                upperLimitIndex,
                lowerLimitIndex,
                (currentIndex != Constants::Invalid) ? std::min(lowerLimitIndex, currentIndex) : Constants::Invalid));
    }
    else
    {
        // add an invalid control status if there are no p-states
        m_controlStatus.push_back(
            ControlStatus("P-States", Constants::Invalid, Constants::Invalid, Constants::Invalid));
    }
}

void PassiveDomainControlStatus::addTstateStatus(DomainProxy& domain)
{
    PerformanceControlFacade& perfControl = domain.getPerformanceControl();
    if (perfControl.supportsPerformanceControls())
    {
        try
        {
            PerformanceControlSet tstateControls = 
                filterControlSet(
                    perfControl.getControls(), 
                    perfControl.getDynamicCapabilities(), 
                    PerformanceControlType::ThrottleState);
            UIntN maxUnlimitedIndex = 0;
            UIntN maxLimitedIndex = tstateControls.getCount() == 0 ? 0 : tstateControls.getCount() - 1;
            IntN tstateIndexStart = 
                indexOfFirstControlWithType(perfControl.getControls(), PerformanceControlType::ThrottleState);
            IntN currentIndex = perfControl.getStatus().getCurrentControlSetIndex() - tstateIndexStart;
            currentIndex = std::max(0, currentIndex);
            if (maxUnlimitedIndex == maxLimitedIndex)
            {
                throw dptf_exception("No T-state controls are available.");
            }

            m_controlStatus.push_back(
                ControlStatus(
                "T-States",
                maxUnlimitedIndex,
                maxLimitedIndex,
                currentIndex));
        }
        catch (...)
        {
            m_controlStatus.push_back(
                ControlStatus("T-States", Constants::Invalid, Constants::Invalid, Constants::Invalid));
        }
    }
    else
    {
        m_controlStatus.push_back(
            ControlStatus("T-States", Constants::Invalid, Constants::Invalid, Constants::Invalid));
    }
}

void PassiveDomainControlStatus::addPowerStatus(DomainProxy& domain)
{
    PowerControlFacade& powerControl = domain.getPowerControl();
    if (powerControl.supportsPowerControls())
    {
        // get the min and max power limits
        Power min(Power::createInvalid());
        Power max(Power::createInvalid());
        try
        {
            UIntN pl1Index = powerControl.getPl1ControlSetIndex();
            PowerControlDynamicCaps pl1Capabilities = powerControl.getCapabilities()[pl1Index];
            max = pl1Capabilities.getMinPowerLimit();
            min = pl1Capabilities.getMaxPowerLimit();
        }
        catch (...)
        {
        }

        // get the current power limit
        Power current(Power::createInvalid());
        try
        {
            const PowerControlStatusSet& statusSet = powerControl.getControls();
            current = statusSet[powerControl.getPl1ControlSetIndex()].getCurrentPowerLimit();
        }
        catch (...)
        {
        }

        // add the control status to the list
        m_controlStatus.push_back(ControlStatus("Power", min, max, current));
    }
    else
    {
        // add an invalid control status to the list if the control is not supported
        m_controlStatus.push_back(
            ControlStatus("Power", Constants::Invalid, Constants::Invalid, Constants::Invalid));
    }
}

void PassiveDomainControlStatus::addDisplayStatus(DomainProxy& domain)
{
    DisplayControlFacade& displayControl = domain.getDisplayControl();
    if (displayControl.supportsDisplayControls())
    {
        // get the max limited and unlimited indexes
        UIntN maxUnlimitedIndex(Constants::Invalid);
        UIntN maxLimitedIndex(Constants::Invalid);
        try
        {
            maxUnlimitedIndex = displayControl.getCapabilities().getCurrentUpperLimit();
            maxLimitedIndex = displayControl.getCapabilities().getCurrentLowerLimit();
        }
        catch (...)
        {
        }

        // get the current limit index
        UIntN currentLimitIndex(Constants::Invalid);
        try
        {
            currentLimitIndex = displayControl.getStatus().getBrightnessLimitIndex();
        }
        catch (...)
        {
        }

        // add the control status to the list
        m_controlStatus.push_back(
            ControlStatus("Display", maxUnlimitedIndex, maxLimitedIndex, currentLimitIndex));
    }
    else
    {
        // add an invalid control status to the list
        m_controlStatus.push_back(
            ControlStatus("Display", Constants::Invalid, Constants::Invalid, Constants::Invalid));
    }
}

void PassiveDomainControlStatus::addCoreStatus(DomainProxy& domain)
{
    CoreControlFacade& coreControl = domain.getCoreControl();
    if (coreControl.supportsCoreControls() && coreControl.getPreferences().isLpoEnabled())
    {
        // get maximum processors
        UIntN maxProcessors(Constants::Invalid);
        try
        {
            maxProcessors = coreControl.getStaticCapabilities().getTotalLogicalProcessors();
        }
        catch (...)
        {
        }

        // get minimum processors
        UIntN minProcessors(Constants::Invalid);
        try
        {
            minProcessors = coreControl.getDynamicCapabilities().getMinActiveCores();
        }
        catch (...)
        {
        }

        // get current processors
        UIntN currentProcessors(Constants::Invalid);
        try
        {
            currentProcessors = coreControl.getStatus().getNumActiveLogicalProcessors();
        }
        catch (...)
        {
        }

        // add control status to the list
        m_controlStatus.push_back(
            ControlStatus("Core", maxProcessors, minProcessors, currentProcessors));
    }
    else
    {
        // add an invalid control status to the list if the control is not supported
        m_controlStatus.push_back(
            ControlStatus("Core", Constants::Invalid, Constants::Invalid, Constants::Invalid));
    }
}

UIntN PassiveDomainControlStatus::indexOfFirstControlWithType(
    const PerformanceControlSet& controlSet, PerformanceControlType::Type type) const
{
    for (UIntN controlIndex = 0; controlIndex < controlSet.getCount(); controlIndex++)
    {
        if (controlSet[controlIndex].getPerformanceControlType() == type)
        {
            return controlIndex;
        }
    }
    throw dptf_exception("Performance control set does not contain the specified type.");
}

UIntN PassiveDomainControlStatus::getNumberOfElementsOfControlType(
    const PerformanceControlSet& controlSet, PerformanceControlType::Type type) const
{
    UIntN count(0);
    for (UIntN controlIndex = 0; controlIndex < controlSet.getCount(); controlIndex++)
    {
        if (controlSet[controlIndex].getPerformanceControlType() == type)
        {
            count++;
        }
    }
    return count;
}

PerformanceControlSet PassiveDomainControlStatus::filterControlSet(
    const PerformanceControlSet& controlSet, 
    PerformanceControlDynamicCaps dynamicCapabilities, 
    PerformanceControlType::Type type) const
{
    std::vector<PerformanceControl> filteredControls;
    UIntN startIndex = dynamicCapabilities.getCurrentUpperLimitIndex();
    UIntN endIndex = dynamicCapabilities.getCurrentLowerLimitIndex();
    for (UIntN controlIndex = startIndex; controlIndex < std::min(endIndex + 1, controlSet.getCount()); controlIndex++)
    {
        if (controlSet[controlIndex].getPerformanceControlType() == type)
        {
            filteredControls.push_back(controlSet[controlIndex]);
        }
    }
    return PerformanceControlSet(filteredControls);
}

void PassiveDomainControlStatus::aquireDomainStatus(DomainProxy& domain)
{
    m_participantIndex = domain.getParticipantIndex();
    m_domainIndex = domain.getDomainIndex();
    m_domainName = domain.getDomainProperties().getName();
    try
    {
        m_domainTemperature = domain.getTemperatureProperty().getCurrentTemperature();
    }
    catch (...)
    {
    }
    try
    {
        m_domainPriority = domain.getDomainPriorityProperty().getDomainPriority();
    }
    catch (...)
    {
    }
    try
    {
        m_domainUtilization = domain.getUtilizationStatus();
    }
    catch (...)
    {
    }
}
