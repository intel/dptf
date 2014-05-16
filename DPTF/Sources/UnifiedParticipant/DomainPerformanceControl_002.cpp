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

#include "DomainPerformanceControl_002.h"
#include "XmlNode.h"

// Processor Participant (CPU Domain) Performance Controls

DomainPerformanceControl_002::DomainPerformanceControl_002(ParticipantServicesInterface* participantServicesInterface) :
    m_participantServicesInterface(participantServicesInterface), m_tdpFrequencyLimitControlIndex(0),
    m_currentPerformanceControlIndex(Constants::Invalid),
    m_performanceControlDynamicCaps(nullptr), m_performanceControlSet(nullptr), m_performanceControlStaticCaps(nullptr)
{   
}

DomainPerformanceControl_002::~DomainPerformanceControl_002(void)
{
    DELETE_MEMORY_TC(m_performanceControlDynamicCaps);
    DELETE_MEMORY_TC(m_performanceControlSet);
    DELETE_MEMORY_TC(m_performanceControlStaticCaps);
}

PerformanceControlStaticCaps DomainPerformanceControl_002::getPerformanceControlStaticCaps(UIntN participantIndex,
    UIntN domainIndex)
{
    initializePerformanceControlStaticCapsIfNull();
    return *m_performanceControlStaticCaps; // This is hard-coded to FALSE in 7.0
}

PerformanceControlDynamicCaps DomainPerformanceControl_002::getPerformanceControlDynamicCaps(UIntN participantIndex,
    UIntN domainIndex)
{
    initializePerformanceControlDynamicCapsIfNull(domainIndex);
    return *m_performanceControlDynamicCaps;
}

PerformanceControlStatus DomainPerformanceControl_002::getPerformanceControlStatus(UIntN participantIndex,
    UIntN domainIndex)
{
    if (m_currentPerformanceControlIndex == Constants::Invalid)
    {
        throw dptf_exception("No performance control has been set.  No status available.");
    }
    return PerformanceControlStatus(m_currentPerformanceControlIndex);
}

PerformanceControlSet DomainPerformanceControl_002::getPerformanceControlSet(UIntN participantIndex, UIntN domainIndex)
{
    initializePerformanceControlSetIfNull(domainIndex);
    return *m_performanceControlSet;
}

void DomainPerformanceControl_002::setPerformanceControl(UIntN participantIndex, UIntN domainIndex,
    UIntN performanceControlIndex)
{
    initializePerformanceControlDynamicCapsIfNull(domainIndex);
    initializePerformanceControlSetIfNull(domainIndex);
    throwIfPerformanceControlIndexIsInvalid(performanceControlIndex);

    PerformanceControlType::Type targetType =
        (*m_performanceControlSet)[performanceControlIndex].getPerformanceControlType();
    switch (targetType)
    {
        case PerformanceControlType::PerformanceState:
            // Set T0
            if (!m_throttlingStateSet.empty())
            {
                m_participantServicesInterface->primitiveExecuteSetAsUInt32(
                    esif_primitive_type::SET_TSTATE_CURRENT,
                    m_throttlingStateSet.at(0).getControlId(),
                    domainIndex);
            }
            m_participantServicesInterface->primitiveExecuteSetAsUInt32(
                esif_primitive_type::SET_PERF_PRESENT_CAPABILITY,
                performanceControlIndex,
                domainIndex);
            break;
        case PerformanceControlType::ThrottleState:
            // Set Pn
            m_participantServicesInterface->primitiveExecuteSetAsUInt32(
                esif_primitive_type::SET_PERF_PRESENT_CAPABILITY,
                static_cast<UIntN>(m_performanceStateSet.size()) - 1,
                domainIndex);
            m_participantServicesInterface->primitiveExecuteSetAsUInt32(
                esif_primitive_type::SET_TSTATE_CURRENT,
                (*m_performanceControlSet)[performanceControlIndex].getControlId(),
                domainIndex);
            break;
        default:
            throw dptf_exception("Invalid performance state requested.");
            break;
    }

    m_currentPerformanceControlIndex = performanceControlIndex;
}

void DomainPerformanceControl_002::clearCachedData(void)
{
    DELETE_MEMORY_TC(m_performanceControlDynamicCaps);
    DELETE_MEMORY_TC(m_performanceControlSet);
    DELETE_MEMORY_TC(m_performanceControlStaticCaps);
    m_performanceStateSet.clear();
    m_throttlingStateSet.clear();
}

void DomainPerformanceControl_002::initializePerformanceControlStaticCapsIfNull()
{
    if (m_performanceControlStaticCaps == nullptr)
    {
        m_performanceControlStaticCaps = new PerformanceControlStaticCaps(false);
    }
}

void DomainPerformanceControl_002::initializePerformanceControlDynamicCapsIfNull(UIntN domainIndex)
{
    if (m_performanceControlDynamicCaps == nullptr)
    {
        // 7.0 Reference : ProcCpuPerfControl.c -> ProcCalPerfControlCapability

        // Get dynamic caps for P-states
        UIntN pStateUpperLimitIndex;
        UIntN pStateLowerLimitIndex;

        calculatePerformanceStateLimits(pStateUpperLimitIndex, pStateLowerLimitIndex, domainIndex);

        // Get dynamic caps for T-states
        UIntN tStateUpperLimitIndex = Constants::Invalid;
        UIntN tStateLowerLimitIndex = Constants::Invalid;
        if (!m_throttlingStateSet.empty())
        {
            calculateThrottlingStateLimits(tStateUpperLimitIndex, tStateLowerLimitIndex, domainIndex);
        }

        // Arbitrate dynamic caps
        // Upper Limit
        UIntN performanceUpperLimitIndex = Constants::Invalid;
        UIntN performanceLowerLimitIndex = Constants::Invalid;
        arbitratePerformanceStateLimits(pStateUpperLimitIndex, pStateLowerLimitIndex,
            tStateUpperLimitIndex, tStateLowerLimitIndex,
            performanceUpperLimitIndex, performanceLowerLimitIndex);
        m_performanceControlDynamicCaps = new PerformanceControlDynamicCaps(performanceLowerLimitIndex, 
            performanceUpperLimitIndex);
    }
}

void DomainPerformanceControl_002::calculatePerformanceStateLimits(UIntN& pStateUpperLimitIndex, UIntN& pStateLowerLimitIndex, UIntN domainIndex)
{
    // TODO: Revisit whether we should even call this to get the upper limit as opposed to just defaulting the upper
    // limit to 0 and arbitrate with ConfigTDP.  This primitive simply gives us the last set P-state index.  If 3rd
    // party tools set this or if we have throttled P-states and then crash and reload, our upper limit will be
    // whatever the last set P-state index was, which is wrong.
    initializePerformanceControlSetIfNull(domainIndex);
    pStateUpperLimitIndex =
        m_participantServicesInterface->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_PROC_PERF_PRESENT_CAPABILITY,
        domainIndex);

    try
    {
        // _PDL is an optional object
        pStateLowerLimitIndex =
            m_participantServicesInterface->primitiveExecuteGetAsUInt32(
            esif_primitive_type::GET_PROC_PERF_PSTATE_DEPTH_LIMIT,
            domainIndex);
    }
    catch (dptf_exception)
    {
        // _PDL wasn't supported. Default to P(n)
        pStateLowerLimitIndex = static_cast<UIntN>(m_performanceStateSet.size() - 1);
    }

    if (pStateUpperLimitIndex >= m_performanceStateSet.size())
    {
        throw dptf_exception("Retrieved upper P-state index limit is out of control set bounds. (P-States)");
    }

    if (pStateLowerLimitIndex >= m_performanceStateSet.size())
    {
        throw dptf_exception("Retrieved lower P-state index limit is out of control set bounds. (P-States)");
    }

    if (pStateUpperLimitIndex > pStateLowerLimitIndex)
    {
        // Update per review w/Vasu: If the limits are bad, ignore the lower limit.  Don't fail this control.
        m_participantServicesInterface->writeMessageWarning(
            ParticipantMessage(FLF, "Limit index mismatch, ignoring lower limit."));
        pStateLowerLimitIndex = static_cast<UIntN>(m_performanceStateSet.size() - 1);
    }
}

void DomainPerformanceControl_002::calculateThrottlingStateLimits(UIntN& tStateUpperLimitIndex, UIntN& tStateLowerLimitIndex, UIntN domainIndex)
{
    // Required object if T-states are supported
    try
    {
        tStateUpperLimitIndex =
            m_participantServicesInterface->primitiveExecuteGetAsUInt32(
            esif_primitive_type::GET_PROC_PERF_THROTTLE_PRESENT_CAPABILITY,
            domainIndex);
    }
    catch (dptf_exception)
    {
        m_participantServicesInterface->writeMessageWarning(
            ParticipantMessage(FLF, "Bad upper T-state limit."));
        tStateUpperLimitIndex = 0;
    }

    try
    {
        // _TDL is an optional object
        tStateLowerLimitIndex =
            m_participantServicesInterface->primitiveExecuteGetAsUInt32(
            esif_primitive_type::GET_PROC_PERF_TSTATE_DEPTH_LIMIT,
            domainIndex);
    }
    catch (dptf_exception)
    {
        // Optional object.  Default value is T(n)
        tStateLowerLimitIndex = static_cast<UIntN>(m_throttlingStateSet.size() - 1);
    }

    if (tStateUpperLimitIndex >= m_throttlingStateSet.size())
    {
        throw dptf_exception("Retrieved upper T-state index limit out of control set bounds. (T-States)");
    }

    if (tStateLowerLimitIndex >= m_throttlingStateSet.size())
    {
        throw dptf_exception("Retrieved lower T-state index limit is out of control set bounds. (T-States)");
    }

    if (tStateUpperLimitIndex > tStateLowerLimitIndex)
    {
        // Update per review w/Vasu: If the limits are bad, ignore the lower limit.  Don't fail this control.
        m_participantServicesInterface->writeMessageWarning(
            ParticipantMessage(FLF, "Limit index mismatch, ignoring lower limit."));
        tStateLowerLimitIndex = static_cast<UIntN>(m_throttlingStateSet.size() - 1);
    }
}

void DomainPerformanceControl_002::arbitratePerformanceStateLimits(
    UIntN pStateUpperLimitIndex, UIntN pStateLowerLimitIndex,
    UIntN tStateUpperLimitIndex, UIntN tStateLowerLimitIndex,
    UIntN& performanceUpperLimitIndex, UIntN& performanceLowerLimitIndex)
{
    if (pStateUpperLimitIndex == (m_performanceStateSet.size() - 1) &&
        pStateLowerLimitIndex == (m_performanceStateSet.size() - 1))
    {
        if (!m_throttlingStateSet.empty())
        {
            performanceUpperLimitIndex = static_cast<UIntN>(m_performanceStateSet.size() + tStateUpperLimitIndex);
        }
        else
        {
            performanceUpperLimitIndex = pStateUpperLimitIndex;
        }
    }
    else
    {
        // Ignore PPC/PDL if TPC is non-zero, ignore P-states
        if (tStateUpperLimitIndex != 0)
        {
            m_participantServicesInterface->writeMessageDebug(
                ParticipantMessage(FLF, "P-states are being ignored because of the T-State upper dynamic cap."));
            performanceUpperLimitIndex = static_cast<UIntN>(m_performanceStateSet.size() + tStateUpperLimitIndex);
        }
        performanceUpperLimitIndex = pStateUpperLimitIndex;
    }
    performanceUpperLimitIndex = std::max(performanceUpperLimitIndex, m_tdpFrequencyLimitControlIndex);

    //  Lower Limit
    if (pStateLowerLimitIndex == (m_performanceStateSet.size() - 1))
    {
        // Lower P is P(n)
        if (m_throttlingStateSet.empty())
        {
            performanceLowerLimitIndex = pStateLowerLimitIndex;
        }
        else
        {
            performanceLowerLimitIndex = static_cast<UIntN>(m_performanceStateSet.size() + tStateLowerLimitIndex);
        }
    }
    else
    {
        //Lower P is NOT P(n), ignore T-states
        performanceLowerLimitIndex = pStateLowerLimitIndex;
        if (!m_throttlingStateSet.empty())
        {
            m_participantServicesInterface->writeMessageWarning(
                ParticipantMessage(FLF, "T-states are being ignored because of the P-State lower dynamic cap."));
        }
    }
}

void DomainPerformanceControl_002::initializePerformanceControlSetIfNull(UIntN domainIndex)
{
    if (m_performanceControlSet == nullptr)
    {
        UInt32 dataLength = 0;
        DptfMemory binaryData(Constants::DefaultBufferSize);

        m_participantServicesInterface->primitiveExecuteGet(
            esif_primitive_type::GET_PROC_PERF_SUPPORT_STATES,
            ESIF_DATA_BINARY,
            binaryData,
            binaryData.getSize(),
            &dataLength,
            domainIndex);

        try
        {
            m_performanceStateSet = BinaryParse::processorPssObject(dataLength, binaryData);
            if (m_performanceStateSet.empty())
            {
                throw dptf_exception("P-state set is empty.  Impossible if we support performance controls.");
            }
        }
        catch (dptf_exception& ex)
        {
            m_participantServicesInterface->writeMessageWarning(ParticipantMessage(FLF, ex.what()));
        }

        binaryData.deallocate();

        // Get T-States
        if (m_performanceStateSet.empty() == false)
        {
            try
            {
                binaryData.allocate(Constants::DefaultBufferSize, true);
                m_participantServicesInterface->primitiveExecuteGet(
                    esif_primitive_type::GET_TSTATES,
                    ESIF_DATA_BINARY,
                    binaryData,
                    binaryData.getSize(),
                    &dataLength,
                    domainIndex);
                m_throttlingStateSet = BinaryParse::processorTssObject(m_performanceStateSet.back(), dataLength, binaryData);
                binaryData.deallocate();
            }
            catch (...)
            {
                // T-States aren't supported.
                binaryData.deallocate();
            }
        }

        // Create all encompassing set (P and T states)
        //    P-States
        std::vector<PerformanceControl> combinedStateSet(m_performanceStateSet);

        //    T-States
        combinedStateSet.insert(combinedStateSet.end(),
            m_throttlingStateSet.begin(), m_throttlingStateSet.end());

        ParticipantMessage message = ParticipantMessage(FLF, "Performance controls created.");
        message.addMessage("Total Entries", combinedStateSet.size());
        message.addMessage("P-State Count", m_performanceStateSet.size());
        message.addMessage("T-State Count", m_throttlingStateSet.size());
        m_participantServicesInterface->writeMessageDebug(message);

        m_performanceControlSet = new PerformanceControlSet(combinedStateSet);
    }
}

void DomainPerformanceControl_002::throwIfPerformanceControlIndexIsInvalid(UIntN performanceControlIndex)
{
    if (performanceControlIndex >= m_performanceControlSet->getCount())
    {
        std::stringstream infoMessage;

        infoMessage << "Control index out of control set bounds." << std::endl
                    << "Desired Index : " << performanceControlIndex << std::endl
                    << "PerformanceControlSet size :" << m_performanceControlSet->getCount() << std::endl;

        throw dptf_exception(infoMessage.str());
    }

    if (performanceControlIndex < m_performanceControlDynamicCaps->getCurrentUpperLimitIndex() ||
        performanceControlIndex > m_performanceControlDynamicCaps->getCurrentLowerLimitIndex())
    {
        std::stringstream infoMessage;

        infoMessage << "Got a performance control index that was outside the allowable range." << std::endl
                    << "Desired Index : " << performanceControlIndex << std::endl
                    << "Upper Limit Index : " << m_performanceControlDynamicCaps->getCurrentUpperLimitIndex() << std::endl
                    << "Lower Limit Index : " << m_performanceControlDynamicCaps->getCurrentLowerLimitIndex() << std::endl;

        throw dptf_exception(infoMessage.str());
    }
}

XmlNode* DomainPerformanceControl_002::getXml(UIntN domainIndex)
{
    initializePerformanceControlStaticCapsIfNull();
    initializePerformanceControlDynamicCapsIfNull(domainIndex);
    initializePerformanceControlSetIfNull(domainIndex);

    XmlNode* root = XmlNode::createWrapperElement("performance_control");
    root->addChild(PerformanceControlStatus(m_currentPerformanceControlIndex).getXml());
    root->addChild(m_performanceControlDynamicCaps->getXml());
    root->addChild(m_performanceControlStaticCaps->getXml());
    root->addChild(m_performanceControlSet->getXml());
    root->addChild(XmlNode::createDataElement("control_knob_version", "002"));

    return root;
}

void DomainPerformanceControl_002::updateBasedOnConfigTdpInformation(UIntN participantIndex, UIntN domainIndex,
    ConfigTdpControlSet configTdpControlSet, ConfigTdpControlStatus configTdpControlStatus)
{
    UInt64 tdpFrequencyLimit = configTdpControlSet[configTdpControlStatus.getCurrentControlIndex()].getTdpFrequency();
    PerformanceControlSet controlSet = getPerformanceControlSet(participantIndex, domainIndex);
    for (UIntN controlIndex = 0; controlIndex < controlSet.getCount(); controlIndex++)
    {
        if (tdpFrequencyLimit >= controlSet[controlIndex].getControlAbsoluteValue())
        {
            m_tdpFrequencyLimitControlIndex = controlIndex;
            break;
        }
    }

    DELETE_MEMORY_TC(m_performanceControlDynamicCaps);
}
