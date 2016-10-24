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

#include "DomainPerformanceControl_002.h"
#include "XmlNode.h"

// Processor Participant (CPU Domain) Performance Controls

DomainPerformanceControl_002::DomainPerformanceControl_002(UIntN participantIndex, UIntN domainIndex,
    ParticipantServicesInterface* participantServicesInterface) :
    DomainPerformanceControlBase(participantIndex, domainIndex, participantServicesInterface),
    m_tdpFrequencyLimitControlIndex(0),
    m_capabilitiesLocked(false)
{
    clearCachedData();
    capture();
}

DomainPerformanceControl_002::~DomainPerformanceControl_002(void)
{
    restore();
}

PerformanceControlStaticCaps DomainPerformanceControl_002::getPerformanceControlStaticCaps(UIntN participantIndex,
    UIntN domainIndex)
{
    if (m_performanceControlStaticCaps.isInvalid())
    {
        m_performanceControlStaticCaps.set(createPerformanceControlStaticCaps());
    }
    return m_performanceControlStaticCaps.get();
}

PerformanceControlDynamicCaps DomainPerformanceControl_002::getPerformanceControlDynamicCaps(UIntN participantIndex,
    UIntN domainIndex)
{
    if (m_performanceControlDynamicCaps.isInvalid())
    {
        m_performanceControlDynamicCaps.set(createPerformanceControlDynamicCaps(domainIndex));
    }
    return m_performanceControlDynamicCaps.get();
}

PerformanceControlStatus DomainPerformanceControl_002::getPerformanceControlStatus(UIntN participantIndex,
    UIntN domainIndex)
{
    if (m_performanceControlStatus.isInvalid())
    {
        m_performanceControlStatus.set(PerformanceControlStatus(Constants::Invalid));
    }
    return m_performanceControlStatus.get();
}

PerformanceControlSet DomainPerformanceControl_002::getPerformanceControlSet(UIntN participantIndex, UIntN domainIndex)
{
    if (m_performanceControlSet.isInvalid())
    {
        m_performanceControlSet.set(createCombinedPerformanceControlSet(domainIndex));
    }
    return m_performanceControlSet.get();
}

void DomainPerformanceControl_002::setPerformanceControl(UIntN participantIndex, UIntN domainIndex,
    UIntN performanceControlIndex)
{
    throwIfPerformanceControlIndexIsOutOfBounds(domainIndex, performanceControlIndex);

    auto performanceControlSet = getPerformanceControlSet(participantIndex, domainIndex);
    PerformanceControlType::Type targetType =
        (performanceControlSet)[performanceControlIndex].getPerformanceControlType();

    auto throttlingStateSet = getThrottlingStateSet(domainIndex);
    auto performanceStateSet = getPerformanceStateSet(domainIndex);

    switch (targetType)
    {
        case PerformanceControlType::PerformanceState:
            // Set T0
            if (throttlingStateSet.getCount() > 0)
            {
                getParticipantServices()->primitiveExecuteSetAsUInt32(
                    esif_primitive_type::SET_TSTATE_CURRENT,
                    throttlingStateSet[0].getControlId(),
                    domainIndex);
            }
            getParticipantServices()->primitiveExecuteSetAsUInt32(
                esif_primitive_type::SET_PERF_PRESENT_CAPABILITY,
                performanceControlIndex,
                domainIndex);
            break;
        case PerformanceControlType::ThrottleState:
            // Set Pn
            getParticipantServices()->primitiveExecuteSetAsUInt32(
                esif_primitive_type::SET_PERF_PRESENT_CAPABILITY,
                static_cast<UIntN>(performanceStateSet.getCount()) - 1,
                domainIndex);
            getParticipantServices()->primitiveExecuteSetAsUInt32(
                esif_primitive_type::SET_TSTATE_CURRENT,
                (performanceControlSet)[performanceControlIndex].getControlId(),
                domainIndex);
            break;
        default:
            throw dptf_exception("Invalid performance state requested.");
            break;
    }

    // Refresh the status
    m_performanceControlStatus.set(PerformanceControlStatus(performanceControlIndex));
}

void DomainPerformanceControl_002::setPerformanceControlDynamicCaps(UIntN participantIndex, UIntN domainIndex, 
    PerformanceControlDynamicCaps newCapabilities)
{
    auto upperLimitIndex = newCapabilities.getCurrentUpperLimitIndex();
    auto lowerLimitIndex = newCapabilities.getCurrentLowerLimitIndex();

    auto throttlingStateSet = getThrottlingStateSet(domainIndex);
    auto performanceStateSet = getPerformanceStateSet(domainIndex);

    auto pstateLowerLimit = 0;
    auto tstateLowerLimit = 0;

    if (upperLimitIndex != Constants::Invalid && lowerLimitIndex != Constants::Invalid)
    {
        auto pstateSetSize = performanceStateSet.getCount();
        auto tstateSetSize = throttlingStateSet.getCount();
        auto combinedSet = getPerformanceControlSet(participantIndex, domainIndex);
        auto combinedSetSize = combinedSet.getCount();
        if (upperLimitIndex >= combinedSetSize)
        {
            throw dptf_exception("Upper Limit index is out of control set bounds.");
        }
        else if (upperLimitIndex > lowerLimitIndex || lowerLimitIndex >= combinedSetSize)
        {
            pstateLowerLimit = pstateSetSize - 1;
            tstateLowerLimit = tstateSetSize - 1;
            getParticipantServices()->writeMessageWarning(
                ParticipantMessage(FLF, "Limit index mismatch, setting lower limit to lowest possible index."));
        }
        else if (lowerLimitIndex > pstateSetSize - 1)
        {
            pstateLowerLimit = pstateSetSize - 1;
            if (isFirstTstateDeleted(domainIndex))
            {
                tstateLowerLimit = combinedSetSize - tstateSetSize;
            }
            else
            {
                tstateLowerLimit = combinedSetSize - tstateSetSize - 1;
            }
        }
    }

    m_performanceControlDynamicCaps.invalidate();

    getParticipantServices()->primitiveExecuteSetAsUInt32(
        esif_primitive_type::SET_PROC_PERF_PSTATE_DEPTH_LIMIT,
        pstateLowerLimit,
        domainIndex,
        Constants::Esif::NoPersistInstance);
    getParticipantServices()->primitiveExecuteSetAsUInt32(
        esif_primitive_type::SET_PROC_PERF_TSTATE_DEPTH_LIMIT,
        tstateLowerLimit,
        domainIndex,
        Constants::Esif::NoPersistInstance);

    // TODO: allow DPTF to change the MAX limit
    getParticipantServices()->writeMessageInfo(
        ParticipantMessage(FLF, "Currently DPTF cannot change the MAX limit."));
}

void DomainPerformanceControl_002::setPerformanceCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
    m_capabilitiesLocked = lock;
}

UIntN DomainPerformanceControl_002::getCurrentPerformanceControlIndex(UIntN participantIndex, UIntN domainIndex)
{
    return getPerformanceControlStatus(participantIndex, domainIndex).getCurrentControlSetIndex();
}

void DomainPerformanceControl_002::clearCachedData(void)
{
    m_performanceControlSet.invalidate();
    m_performanceControlDynamicCaps.invalidate();
    m_performanceControlStaticCaps.invalidate();
    m_performanceControlStatus.invalidate();
    m_performanceStateSet.invalidate();
    m_throttlingStateSet.invalidate();
    m_isFirstTstateDeleted.invalidate();

    if (m_capabilitiesLocked == false)
    {
        DptfBuffer depthLimitBuffer = createResetPrimitiveTupleBinary(
            esif_primitive_type::SET_PROC_PERF_PSTATE_DEPTH_LIMIT, Constants::Esif::NoPersistInstance);
        getParticipantServices()->primitiveExecuteSet(
            esif_primitive_type::SET_CONFIG_RESET, ESIF_DATA_BINARY,
            depthLimitBuffer.get(), depthLimitBuffer.size(), depthLimitBuffer.size(),
            0, Constants::Esif::NoInstance);

        depthLimitBuffer = createResetPrimitiveTupleBinary(
            esif_primitive_type::SET_PROC_PERF_TSTATE_DEPTH_LIMIT, Constants::Esif::NoPersistInstance);
        getParticipantServices()->primitiveExecuteSet(
            esif_primitive_type::SET_CONFIG_RESET, ESIF_DATA_BINARY,
            depthLimitBuffer.get(), depthLimitBuffer.size(), depthLimitBuffer.size(),
            0, Constants::Esif::NoInstance);
    }
}

PerformanceControlStaticCaps DomainPerformanceControl_002::createPerformanceControlStaticCaps(void)
{
    return PerformanceControlStaticCaps(false); // This is hard-coded to FALSE in 7.0
}

PerformanceControlDynamicCaps DomainPerformanceControl_002::createPerformanceControlDynamicCaps(UIntN domainIndex)
{
    // 7.0 Reference : ProcCpuPerfControl.c -> ProcCalPerfControlCapability

    // Get dynamic caps for P-states
    UIntN pStateUpperLimitIndex;
    UIntN pStateLowerLimitIndex;

    calculatePerformanceStateLimits(pStateUpperLimitIndex, pStateLowerLimitIndex, domainIndex);

    // Get dynamic caps for T-states
    UIntN tStateUpperLimitIndex = Constants::Invalid;
    UIntN tStateLowerLimitIndex = Constants::Invalid;
    if (getThrottlingStateSet(domainIndex).getCount() > 0)
    {
        calculateThrottlingStateLimits(tStateUpperLimitIndex, tStateLowerLimitIndex, domainIndex);
    }

    // Arbitrate dynamic caps
    // Upper Limit
    UIntN performanceUpperLimitIndex = Constants::Invalid;
    UIntN performanceLowerLimitIndex = Constants::Invalid;
    arbitratePerformanceStateLimits(domainIndex, pStateUpperLimitIndex,
        pStateLowerLimitIndex, tStateUpperLimitIndex,
        tStateLowerLimitIndex, performanceUpperLimitIndex, performanceLowerLimitIndex);
    return PerformanceControlDynamicCaps(performanceLowerLimitIndex, performanceUpperLimitIndex);
}

void DomainPerformanceControl_002::calculatePerformanceStateLimits(UIntN& pStateUpperLimitIndex, 
    UIntN& pStateLowerLimitIndex, UIntN domainIndex)
{
    // TODO: Revisit whether we should even call this to get the upper limit as opposed to just defaulting the upper
    // limit to 0 and arbitrate with ConfigTDP.  This primitive simply gives us the last set P-state index.  If 3rd
    // party tools set this or if we have throttled P-states and then crash and reload, our upper limit will be
    // whatever the last set P-state index was, which is wrong.
    pStateUpperLimitIndex =
        getParticipantServices()->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_PROC_PERF_PRESENT_CAPABILITY,
        domainIndex);

    auto performanceStateSetSize = getPerformanceStateSet(domainIndex).getCount();
    try
    {
        // _PDL is an optional object
        pStateLowerLimitIndex =
            getParticipantServices()->primitiveExecuteGetAsUInt32(
            esif_primitive_type::GET_PROC_PERF_PSTATE_DEPTH_LIMIT,
            domainIndex);
    }
    catch (dptf_exception)
    {
        // _PDL wasn't supported. Default to P(n)
        pStateLowerLimitIndex = static_cast<UIntN>(performanceStateSetSize - 1);
    }

    if (pStateUpperLimitIndex >= performanceStateSetSize)
    {
        throw dptf_exception("Retrieved upper P-state index limit is out of control set bounds. (P-States)");
    }

    if (pStateLowerLimitIndex >= performanceStateSetSize)
    {
        throw dptf_exception("Retrieved lower P-state index limit is out of control set bounds. (P-States)");
    }

    if (pStateUpperLimitIndex > pStateLowerLimitIndex)
    {
        // If the limits are bad, ignore the lower limit.  Don't fail this control.
        getParticipantServices()->writeMessageWarning(
            ParticipantMessage(FLF, "Limit index mismatch, ignoring lower limit."));
        pStateLowerLimitIndex = static_cast<UIntN>(performanceStateSetSize - 1);
    }
}

void DomainPerformanceControl_002::calculateThrottlingStateLimits(UIntN& tStateUpperLimitIndex, 
    UIntN& tStateLowerLimitIndex, UIntN domainIndex)
{
    // Required object if T-states are supported
    try
    {
        tStateUpperLimitIndex =
            getParticipantServices()->primitiveExecuteGetAsUInt32(
            esif_primitive_type::GET_PROC_PERF_THROTTLE_PRESENT_CAPABILITY,
            domainIndex);
    }
    catch (dptf_exception)
    {
        getParticipantServices()->writeMessageWarning(
            ParticipantMessage(FLF, "Bad upper T-state limit."));
        tStateUpperLimitIndex = 0;
    }

    auto throttlingStateSetSize = getThrottlingStateSet(domainIndex).getCount();
    try
    {
        // _TDL is an optional object
        tStateLowerLimitIndex =
            getParticipantServices()->primitiveExecuteGetAsUInt32(
            esif_primitive_type::GET_PROC_PERF_TSTATE_DEPTH_LIMIT,
            domainIndex);
    }
    catch (dptf_exception)
    {
        // Optional object.  Default value is T(n)
        tStateLowerLimitIndex = static_cast<UIntN>(throttlingStateSetSize - 1);
    }

    if (isFirstTstateDeleted(domainIndex) && tStateLowerLimitIndex != 0)
    {
        tStateLowerLimitIndex--;
    }

    if (tStateUpperLimitIndex >= throttlingStateSetSize)
    {
        throw dptf_exception("Retrieved upper T-state index limit out of control set bounds. (T-States)");
    }

    if (tStateLowerLimitIndex >= throttlingStateSetSize)
    {
        throw dptf_exception("Retrieved lower T-state index limit is out of control set bounds. (T-States)");
    }

    if (tStateUpperLimitIndex > tStateLowerLimitIndex)
    {
        // If the limits are bad, ignore the lower limit.  Don't fail this control.
        getParticipantServices()->writeMessageWarning(
            ParticipantMessage(FLF, "Limit index mismatch, ignoring lower limit."));
        tStateLowerLimitIndex = static_cast<UIntN>(throttlingStateSetSize - 1);
    }
}

void DomainPerformanceControl_002::arbitratePerformanceStateLimits(
    UIntN domainIndex, UIntN pStateUpperLimitIndex, UIntN pStateLowerLimitIndex,
    UIntN tStateUpperLimitIndex, UIntN tStateLowerLimitIndex,
    UIntN& performanceUpperLimitIndex, UIntN& performanceLowerLimitIndex)
{
    auto performanceStateSetSize = getPerformanceStateSet(domainIndex).getCount();
    auto throttlingStateSetSize = getThrottlingStateSet(domainIndex).getCount();

    if (pStateUpperLimitIndex == (performanceStateSetSize - 1) &&
        pStateLowerLimitIndex == (performanceStateSetSize - 1))
    {
        if (throttlingStateSetSize > 0)
        {
            performanceUpperLimitIndex = static_cast<UIntN>(performanceStateSetSize + tStateUpperLimitIndex);
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
            getParticipantServices()->writeMessageDebug(
                ParticipantMessage(FLF, "P-states are being ignored because of the T-State upper dynamic cap."));
            performanceUpperLimitIndex = static_cast<UIntN>(performanceStateSetSize + tStateUpperLimitIndex);
        }
        performanceUpperLimitIndex = pStateUpperLimitIndex;
    }
    performanceUpperLimitIndex = std::max(performanceUpperLimitIndex, m_tdpFrequencyLimitControlIndex);
    getParticipantServices()->writeMessageDebug(
        ParticipantMessage(FLF, "Performance upper limit index is: " + StlOverride::to_string(performanceUpperLimitIndex)));

    //  Lower Limit
    if (pStateLowerLimitIndex == (performanceStateSetSize - 1))
    {
        // Lower P is P(n)
        if (throttlingStateSetSize == 0)
        {
            performanceLowerLimitIndex = pStateLowerLimitIndex;
        }
        else
        {
            performanceLowerLimitIndex = static_cast<UIntN>(performanceStateSetSize + tStateLowerLimitIndex);
        }
    }
    else
    {
        //Lower P is NOT P(n), ignore T-states
        performanceLowerLimitIndex = pStateLowerLimitIndex;
        if (throttlingStateSetSize > 0)
        {
            getParticipantServices()->writeMessageWarning(
                ParticipantMessage(FLF, "T-states are being ignored because of the P-State lower dynamic cap."));
        }
    }
    getParticipantServices()->writeMessageDebug(
        ParticipantMessage(FLF, "Performance lower limit index is: " + StlOverride::to_string(performanceLowerLimitIndex)));
}

PerformanceControlSet DomainPerformanceControl_002::createPerformanceStateSet(UIntN domainIndex)
{
    DptfBuffer pstateBuffer = getParticipantServices()->primitiveExecuteGet(
        esif_primitive_type::GET_PROC_PERF_SUPPORT_STATES, ESIF_DATA_BINARY, domainIndex);
    auto performanceStateSet = PerformanceControlSet::createFromProcessorPss(pstateBuffer);
    if (performanceStateSet.getCount() == 0)
    {
        throw dptf_exception("P-state set is empty.  Impossible if we support performance controls.");
    }

    return performanceStateSet;
}

PerformanceControlSet DomainPerformanceControl_002::getPerformanceStateSet(UIntN domainIndex)
{
    if (m_performanceStateSet.isInvalid())
    {
        m_performanceStateSet.set(createPerformanceStateSet(domainIndex));
    }
    return m_performanceStateSet.get();
}

PerformanceControlSet DomainPerformanceControl_002::getThrottlingStateSet(UIntN domainIndex)
{
    if (m_throttlingStateSet.isInvalid())
    {
        m_throttlingStateSet.set(createThrottlingStateSet(domainIndex));
    }
    return m_throttlingStateSet.get();
}

PerformanceControlSet DomainPerformanceControl_002::createThrottlingStateSet(UIntN domainIndex)
{
    PerformanceControlSet performanceStateSet = getPerformanceStateSet(domainIndex);
    PerformanceControlSet throttlingStateSet;
    try
    {
        DptfBuffer tstateBuffer = getParticipantServices()->primitiveExecuteGet(
            esif_primitive_type::GET_TSTATES, ESIF_DATA_BINARY, domainIndex);
        throttlingStateSet = PerformanceControlSet::createFromProcessorTss(
            performanceStateSet[performanceStateSet.getCount() - 1], tstateBuffer);
    }
    catch (...)
    {
        // T-States aren't supported.
        throttlingStateSet = PerformanceControlSet();
    }
    return throttlingStateSet;
}

Bool DomainPerformanceControl_002::isFirstTstateDeleted(UIntN domainIndex)
{
    if (m_isFirstTstateDeleted.isInvalid())
    {
        m_isFirstTstateDeleted.set(false);
        PerformanceControlSet performanceStateSet = getPerformanceStateSet(domainIndex);
        PerformanceControlSet throttlingStateSet;

        // Get T-States
        if (performanceStateSet.getCount() > 0)
        {
            throttlingStateSet = getThrottlingStateSet(domainIndex);
        }

        // Removing the first Tstate if the last Pstate and first Tstate are same.
        if (performanceStateSet.getCount() > 0 && throttlingStateSet.getCount() > 0)
        {
            auto lastPstateIndex = performanceStateSet.getCount() - 1;
            if (performanceStateSet[lastPstateIndex].getControlAbsoluteValue() ==
                throttlingStateSet[0].getControlAbsoluteValue())
            {
                m_isFirstTstateDeleted.set(true);
            }
        }
    }

    return m_isFirstTstateDeleted.get();
}

PerformanceControlSet DomainPerformanceControl_002::createCombinedPerformanceControlSet(UIntN domainIndex)
{
    PerformanceControlSet performanceStateSet = getPerformanceStateSet(domainIndex);
    PerformanceControlSet throttlingStateSet;

    // Get T-States
    if (performanceStateSet.getCount() > 0)
    {
        throttlingStateSet = getThrottlingStateSet(domainIndex);
    }

    // Create all encompassing set (P and T states)
    //    P-States
    PerformanceControlSet combinedStateSet(performanceStateSet);

    //    T-States
    auto tStateStartIndex = isFirstTstateDeleted(domainIndex) ? 1 : 0;
    combinedStateSet.append(throttlingStateSet, tStateStartIndex);
        
    ParticipantMessage message = ParticipantMessage(FLF, "Performance controls created.");
    message.addMessage("Total Entries", combinedStateSet.getCount());
    message.addMessage("P-State Count", performanceStateSet.getCount());
    message.addMessage("T-State Count", throttlingStateSet.getCount());
    getParticipantServices()->writeMessageDebug(message);

    return PerformanceControlSet(combinedStateSet);
}

void DomainPerformanceControl_002::throwIfPerformanceControlIndexIsOutOfBounds(UIntN domainIndex, UIntN performanceControlIndex)
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

std::shared_ptr<XmlNode> DomainPerformanceControl_002::getXml(UIntN domainIndex)
{
    auto root = XmlNode::createWrapperElement("performance_control");
    root->addChild(getPerformanceControlStatus(getParticipantIndex(), domainIndex).getXml());
    root->addChild(getPerformanceControlDynamicCaps(getParticipantIndex(), domainIndex).getXml());
    root->addChild(getPerformanceControlStaticCaps(getParticipantIndex(), domainIndex).getXml());
    root->addChild(getPerformanceControlSet(getParticipantIndex(), domainIndex).getXml());
    root->addChild(XmlNode::createDataElement("control_knob_version", "002"));

    return root;
}

void DomainPerformanceControl_002::capture(void)
{
    try
    {
        m_initialStatus.set(getPerformanceControlDynamicCaps(getParticipantIndex(), getDomainIndex()));
        getParticipantServices()->writeMessageDebug(ParticipantMessage(
            FLF, "Initial performance capabilities are captured. MIN = " + 
            StlOverride::to_string(m_initialStatus.get().getCurrentLowerLimitIndex()) + 
            " & MAX = " + StlOverride::to_string(m_initialStatus.get().getCurrentUpperLimitIndex())));
    }
    catch (dptf_exception& e)
    {
        m_initialStatus.invalidate();
        std::string warningMsg = e.what();
        getParticipantServices()->writeMessageWarning(ParticipantMessage(
            FLF, "Failed to get the initial processor performance control dynamic capabilities. " + warningMsg));
    }
}

void DomainPerformanceControl_002::restore(void)
{
    if (m_initialStatus.isValid())
    {
        try
        {
            auto restoreIndex = m_initialStatus.get().getCurrentUpperLimitIndex();
            auto domainIndex = getDomainIndex();
            getParticipantServices()->writeMessageDebug(ParticipantMessage(
                FLF, "Restoring... P-state = P" + StlOverride::to_string(restoreIndex) + " & T-state = T0."));

            auto throttlingStateSet = getThrottlingStateSet(domainIndex);
            // Set T0
            if (throttlingStateSet.getCount() > 0)
            {
                getParticipantServices()->primitiveExecuteSetAsUInt32(
                    esif_primitive_type::SET_TSTATE_CURRENT,
                    throttlingStateSet[0].getControlId(),
                    domainIndex);
            }
            getParticipantServices()->primitiveExecuteSetAsUInt32(
                esif_primitive_type::SET_PERF_PRESENT_CAPABILITY,
                restoreIndex,
                domainIndex);
        }
        catch (...)
        {
            // best effort
            getParticipantServices()->writeMessageDebug(ParticipantMessage(
                FLF, "Failed to restore the initial performance control status. "));
        }
    }
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

    m_performanceControlDynamicCaps.invalidate();
}

std::string DomainPerformanceControl_002::getName(void)
{
    return "Performance Control (Version 2)";
}
