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

#include "DomainPerformanceControl_003.h"
#include "XmlNode.h"

DomainPerformanceControl_003::DomainPerformanceControl_003(UIntN participantIndex, UIntN domainIndex,
    std::shared_ptr<ParticipantServicesInterface> participantServicesInterface) :
    DomainPerformanceControlBase(participantIndex, domainIndex, participantServicesInterface),
    m_capabilitiesLocked(false)
{
    clearCachedData();
    capture();
}

DomainPerformanceControl_003::~DomainPerformanceControl_003(void)
{
    restore();
}

PerformanceControlStaticCaps DomainPerformanceControl_003::getPerformanceControlStaticCaps(UIntN participantIndex,
    UIntN domainIndex)
{
    if (m_performanceControlStaticCaps.isInvalid())
    {
        m_performanceControlStaticCaps.set(createPerformanceControlStaticCaps());
    }
    return m_performanceControlStaticCaps.get();
}

PerformanceControlDynamicCaps DomainPerformanceControl_003::getPerformanceControlDynamicCaps(UIntN participantIndex,
    UIntN domainIndex)
{
    if (m_performanceControlDynamicCaps.isInvalid())
    {
        m_performanceControlDynamicCaps.set(createPerformanceControlDynamicCaps(domainIndex));
    }
    return m_performanceControlDynamicCaps.get();
}

PerformanceControlStatus DomainPerformanceControl_003::getPerformanceControlStatus(UIntN participantIndex,
    UIntN domainIndex)
{
    if (m_performanceControlStatus.isInvalid())
    {
        m_performanceControlStatus.set(PerformanceControlStatus(Constants::Invalid));
    }
    return m_performanceControlStatus.get();
}

PerformanceControlSet DomainPerformanceControl_003::getPerformanceControlSet(UIntN participantIndex, UIntN domainIndex)
{
    if (m_performanceControlSet.isInvalid())
    {
        m_performanceControlSet.set(createPerformanceControlSet(domainIndex));
    }
    return m_performanceControlSet.get();
}

void DomainPerformanceControl_003::setPerformanceControl(UIntN participantIndex, UIntN domainIndex,
    UIntN performanceControlIndex)
{
    throwIfPerformanceControlIndexIsOutOfBounds(participantIndex, performanceControlIndex);
    getParticipantServices()->primitiveExecuteSetAsUInt32(
        esif_primitive_type::SET_PERF_PRESENT_CAPABILITY,
        performanceControlIndex,
        domainIndex);

    // Refresh the status
    m_performanceControlStatus.set(PerformanceControlStatus(performanceControlIndex));
}

void DomainPerformanceControl_003::setPerformanceControlDynamicCaps(UIntN participantIndex, UIntN domainIndex, 
    PerformanceControlDynamicCaps newCapabilities)
{
    auto upperLimitIndex = newCapabilities.getCurrentUpperLimitIndex();
    auto lowerLimitIndex = newCapabilities.getCurrentLowerLimitIndex();

    if (upperLimitIndex == Constants::Invalid && lowerLimitIndex == Constants::Invalid)
    {
        if (m_capabilitiesLocked == false)
        {
            m_performanceControlDynamicCaps.invalidate();
        }

        return;
    }

    auto size = getPerformanceControlSet(participantIndex, domainIndex).getCount();
    if (upperLimitIndex >= size)
    {
        throw dptf_exception("Upper Limit index is out of control set bounds.");
    }
    else if (upperLimitIndex > lowerLimitIndex || lowerLimitIndex >= size)
    {
        lowerLimitIndex = size - 1;
        getParticipantServices()->writeMessageWarning(
            ParticipantMessage(FLF, "Limit index mismatch, setting lower limit to lowest possible index."));
    }

    m_performanceControlDynamicCaps.invalidate();
    m_performanceControlDynamicCaps.set(PerformanceControlDynamicCaps(lowerLimitIndex, upperLimitIndex));
}

void DomainPerformanceControl_003::setPerformanceCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
    m_capabilitiesLocked = lock;
}

UIntN DomainPerformanceControl_003::getCurrentPerformanceControlIndex(UIntN participantIndex, UIntN domainIndex)
{
    return getPerformanceControlStatus(participantIndex, domainIndex).getCurrentControlSetIndex();
}

void DomainPerformanceControl_003::clearCachedData(void)
{
    if (m_capabilitiesLocked == false)
    {
        m_performanceControlDynamicCaps.invalidate();
    }

    m_performanceControlStaticCaps.invalidate();
    m_performanceControlSet.invalidate();
    m_performanceControlStatus.invalidate();
}

PerformanceControlSet DomainPerformanceControl_003::createPerformanceControlSet(UIntN domainIndex)
{
    // Build GFX performance table
    DptfBuffer buffer = getParticipantServices()->primitiveExecuteGet(
        esif_primitive_type::GET_PERF_SUPPORT_STATES, ESIF_DATA_BINARY, domainIndex);
    auto controlSet = PerformanceControlSet(PerformanceControlSet::createFromProcessorGfxPstates(buffer));
    if (controlSet.getCount() == 0)
    {
        throw dptf_exception("GFX P-state set is empty. Impossible if we support performance controls.");
    }

    return controlSet;
}

PerformanceControlDynamicCaps DomainPerformanceControl_003::createPerformanceControlDynamicCaps(UIntN domainIndex)
{
    auto controlSetSize = getPerformanceControlSet(getParticipantIndex(), domainIndex).getCount();
    return PerformanceControlDynamicCaps(controlSetSize - 1, 0);
}

void DomainPerformanceControl_003::throwIfPerformanceControlIndexIsOutOfBounds(UIntN domainIndex, UIntN performanceControlIndex)
{
    auto controlSetSize = getPerformanceControlSet(getParticipantIndex(), domainIndex).getCount();
    if (performanceControlIndex >= controlSetSize)
    {
        std::stringstream infoMessage;

        infoMessage << "Control index out of control set bounds." << std::endl
                    << "Desired Index : " << performanceControlIndex << std::endl
                    << "PerformanceControlSet size :" << controlSetSize << std::endl;

        throw dptf_exception(infoMessage.str());
    }

    auto caps = getPerformanceControlDynamicCaps(getParticipantIndex(), domainIndex);
    if (performanceControlIndex < caps.getCurrentUpperLimitIndex() ||
        performanceControlIndex > caps.getCurrentLowerLimitIndex())
    {
        std::stringstream infoMessage;

        infoMessage << "Got a performance control index that was outside the allowable range." << std::endl
                    << "Desired Index : " << performanceControlIndex << std::endl
                    << "Upper Limit Index : " << caps.getCurrentUpperLimitIndex() << std::endl
                    << "Lower Limit Index : " << caps.getCurrentLowerLimitIndex() << std::endl;

        throw dptf_exception(infoMessage.str());
    }
}

PerformanceControlStaticCaps DomainPerformanceControl_003::createPerformanceControlStaticCaps(void)
{
    return PerformanceControlStaticCaps(false); // This is hard-coded to FALSE in 7.0
}

std::shared_ptr<XmlNode> DomainPerformanceControl_003::getXml(UIntN domainIndex)
{
    auto root = XmlNode::createWrapperElement("performance_control");
    root->addChild(getPerformanceControlStatus(getParticipantIndex(), domainIndex).getXml());
    root->addChild(getPerformanceControlDynamicCaps(getParticipantIndex(), domainIndex).getXml());
    root->addChild(getPerformanceControlStaticCaps(getParticipantIndex(), domainIndex).getXml());
    root->addChild(getPerformanceControlSet(getParticipantIndex(), domainIndex).getXml());
    root->addChild(XmlNode::createDataElement("control_knob_version", "003"));

    return root;
}

void DomainPerformanceControl_003::capture(void)
{
    try
    {
        m_initialStatus.set(getPerformanceControlDynamicCaps(getParticipantIndex(), getDomainIndex()));
    }
    catch (dptf_exception& e)
    {
        m_initialStatus.invalidate();
        std::string warningMsg = e.what();
        getParticipantServices()->writeMessageWarning(ParticipantMessage(
            FLF, "Failed to get the initial graphics performance control dynamic capabilities. " + warningMsg));
    }
}

void DomainPerformanceControl_003::restore(void)
{
    if (m_initialStatus.isValid())
    {
        try
        {
            getParticipantServices()->primitiveExecuteSetAsUInt32(
                esif_primitive_type::SET_PERF_PRESENT_CAPABILITY,
                m_initialStatus.get().getCurrentUpperLimitIndex(),
                getDomainIndex());
        }
        catch (...)
        {
            // best effort
            getParticipantServices()->writeMessageDebug(ParticipantMessage(
                FLF, "Failed to restore the initial performance control status. "));
        }
    }
}

void DomainPerformanceControl_003::updateBasedOnConfigTdpInformation(UIntN participantIndex, UIntN domainIndex,
    ConfigTdpControlSet configTdpControlSet, ConfigTdpControlStatus configTdpControlStatus)
{
    throw not_implemented();
}

std::string DomainPerformanceControl_003::getName(void)
{
    return "Performance Control (Version 3)";
}
