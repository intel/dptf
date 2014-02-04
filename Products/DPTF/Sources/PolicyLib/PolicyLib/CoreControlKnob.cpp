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

#include "CoreControlKnob.h"
#include <math.h>
using namespace std;

CoreControlKnob::CoreControlKnob(
    const PolicyServicesInterfaceContainer& policyServices,
    UIntN participantIndex,
    UIntN domainIndex,
    shared_ptr<CoreControlFacade> coreControl,
    shared_ptr<PerformanceControlFacade> performanceControl)
    : ControlKnobBase(policyServices, participantIndex, domainIndex),
    m_coreControl(coreControl),
    m_performanceControl(performanceControl)
{
}

CoreControlKnob::~CoreControlKnob(void)
{
}

void CoreControlKnob::limit()
{
    if (canLimit())
    {
        try
        {
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Attempting to limit cores.", 
                getParticipantIndex(), getDomainIndex()));

            Percentage stepSize = m_coreControl->getPreferences().getStepSize();
            UIntN totalCores = m_coreControl->getStaticCapabilities().getTotalLogicalProcessors();
            UIntN stepAmount = calculateStepAmount(stepSize, totalCores);
            UIntN currentActiveCores = m_coreControl->getStatus().getNumActiveLogicalProcessors();
            UIntN minActiveCores = m_coreControl->getDynamicCapabilities().getMinActiveCores();
            UIntN nextActiveCores = std::max(currentActiveCores - stepAmount, minActiveCores);
            m_coreControl->setControl(CoreControlStatus(nextActiveCores));

            stringstream message;
            message << "Limited cores to " << nextActiveCores << ".";
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, message.str(), getParticipantIndex(), getDomainIndex()));
        }
        catch (std::exception& ex)
        {
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, ex.what(), getParticipantIndex(), getDomainIndex()));
            throw ex;
        }
    }
}

void CoreControlKnob::unlimit()
{
    if (canUnlimit())
    {
        try
        {
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Attempting to unlimit cores.", 
                getParticipantIndex(), getDomainIndex()));

            Percentage stepSize = m_coreControl->getPreferences().getStepSize();
            UIntN totalCores = m_coreControl->getStaticCapabilities().getTotalLogicalProcessors();
            UIntN stepAmount = calculateStepAmount(stepSize, totalCores);
            UIntN currentActiveCores = m_coreControl->getStatus().getNumActiveLogicalProcessors();
            UIntN maxActiveCores = m_coreControl->getDynamicCapabilities().getMaxActiveCores();
            UIntN nextActiveCores = std::min(currentActiveCores + stepAmount, maxActiveCores);
            m_coreControl->setControl(CoreControlStatus(nextActiveCores));

            stringstream message;
            message << "Unlimited cores to " << nextActiveCores << ".";
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, message.str(), getParticipantIndex(), getDomainIndex()));
        }
        catch (std::exception& ex)
        {
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, ex.what(), getParticipantIndex(), getDomainIndex()));
            throw ex;
        }
    }
}

Bool CoreControlKnob::canLimit()
{
    try
    {
        if ((m_coreControl->supportsCoreControls() == false) ||
            (m_coreControl->getPreferences().isLpoEnabled() == false))
        {
            return false;
        }

        UIntN startPerformanceIndex = m_coreControl->getPreferences().getStartPState();
        UIntN currentPerformanceIndex;
        if (m_performanceControl->supportsPerformanceControls())
        {
            currentPerformanceIndex = m_performanceControl->getStatus().getCurrentControlSetIndex();
        }
        else
        {
            currentPerformanceIndex = startPerformanceIndex;
        }

        UIntN numActiveCores = m_coreControl->getStatus().getNumActiveLogicalProcessors();
        UIntN minActiveCores = m_coreControl->getDynamicCapabilities().getMinActiveCores();
        return ((currentPerformanceIndex >= startPerformanceIndex) && (numActiveCores > minActiveCores));
    }
    catch (...)
    {
        return false;
    }
}

Bool CoreControlKnob::canUnlimit()
{
    try
    {
        if ((m_coreControl->supportsCoreControls() == false) ||
            (m_coreControl->getPreferences().isLpoEnabled() == false))
        {
            return false;
        }

        UIntN numActiveCores = m_coreControl->getStatus().getNumActiveLogicalProcessors();
        UIntN maxActiveCores = m_coreControl->getDynamicCapabilities().getMaxActiveCores();
        return (numActiveCores < maxActiveCores);
    }
    catch (...)
    {
        return false;
    }
}

UIntN CoreControlKnob::calculateStepAmount(Percentage stepSize, UIntN totalAvailableCores)
{
    double result = stepSize * (double)totalAvailableCores;
    return (UIntN)ceil(result);
}

XmlNode* CoreControlKnob::getXml()
{
    XmlNode* status = XmlNode::createWrapperElement("core_control_knob_status");
    if (m_coreControl->supportsCoreControls())
    {
        status->addChild(m_coreControl->getStaticCapabilities().getXml());
        status->addChild(m_coreControl->getStatus().getXml());
    }
    return status;
}