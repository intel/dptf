/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

DomainPerformanceControl_002::DomainPerformanceControl_002(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainPerformanceControlBase(participantIndex, domainIndex, participantServicesInterface)
	, m_tdpFrequencyLimitControlIndex(0)
	, m_capabilitiesLocked(false)
{
	onClearCachedData();
	capture();
}

DomainPerformanceControl_002::~DomainPerformanceControl_002(void)
{
	restore();
}

PerformanceControlStaticCaps DomainPerformanceControl_002::getPerformanceControlStaticCaps(
	UIntN participantIndex,
	UIntN domainIndex)
{
	if (m_performanceControlStaticCaps.isInvalid())
	{
		m_performanceControlStaticCaps.set(createPerformanceControlStaticCaps());
	}
	return m_performanceControlStaticCaps.get();
}

PerformanceControlDynamicCaps DomainPerformanceControl_002::getPerformanceControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex)
{
	if (m_performanceControlDynamicCaps.isInvalid())
	{
		m_performanceControlDynamicCaps.set(createPerformanceControlDynamicCaps(domainIndex));
	}
	return m_performanceControlDynamicCaps.get();
}

PerformanceControlStatus DomainPerformanceControl_002::getPerformanceControlStatus(
	UIntN participantIndex,
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

void DomainPerformanceControl_002::setPerformanceControl(
	UIntN participantIndex,
	UIntN domainIndex,
	UIntN performanceControlIndex)
{
	PARTICIPANT_LOG_MESSAGE_DEBUG(
		{ return "Requesting Performance Control Index: " + std::to_string(performanceControlIndex); });

	performanceControlIndex = snapIfPerformanceControlIndexIsOutOfBounds(domainIndex, performanceControlIndex);

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
				esif_primitive_type::SET_TSTATE_CURRENT, throttlingStateSet[0].getControlId(), domainIndex);
		}
		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_PERF_PRESENT_CAPABILITY, performanceControlIndex, domainIndex);
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

	PARTICIPANT_LOG_MESSAGE_DEBUG(
		{ return "Set Performance Control Index: " + std::to_string(performanceControlIndex); });
	// Refresh the status
	m_performanceControlStatus.set(PerformanceControlStatus(performanceControlIndex));
}

void DomainPerformanceControl_002::setPerformanceControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex,
	PerformanceControlDynamicCaps newCapabilities)
{
	auto upperLimitIndex = newCapabilities.getCurrentUpperLimitIndex();
	auto lowerLimitIndex = newCapabilities.getCurrentLowerLimitIndex();

	auto throttlingStateSet = getThrottlingStateSet(domainIndex);
	auto performanceStateSet = getPerformanceStateSet(domainIndex);

	auto pstateUpperLimit = Constants::Invalid;
	auto tstateUpperLimit = Constants::Invalid;
	auto pstateLowerLimit = Constants::Invalid;
	auto tstateLowerLimit = Constants::Invalid;

	if (upperLimitIndex != Constants::Invalid && lowerLimitIndex != Constants::Invalid)
	{
		auto pstateSetSize = performanceStateSet.getCount();
		auto tstateSetSize = throttlingStateSet.getCount();
		auto combinedSet = getPerformanceControlSet(participantIndex, domainIndex);
		auto combinedSetSize = combinedSet.getCount();
		UIntN ppdl = getParticipantServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_PROC_PERF_PSTATE_DEPTH_LIMIT, domainIndex);
		auto minIndex = (ppdl < combinedSetSize) ? ppdl : combinedSetSize - 1;

		if (upperLimitIndex >= combinedSetSize)
		{
			pstateUpperLimit = 0;
			tstateUpperLimit = (tstateSetSize > 0) ? 0 : Constants::Invalid;
			PARTICIPANT_LOG_MESSAGE_WARNING(
				{ return "Limit index mismatch, setting upper limit to highest possible index."; });
		}
		else if (upperLimitIndex < ((ppdl < pstateSetSize) ? ppdl : pstateSetSize))
		{
			pstateUpperLimit = upperLimitIndex;
			tstateUpperLimit = (tstateSetSize > 0) ? 0 : Constants::Invalid;
		}
		else
		{
			pstateUpperLimit = 0;
			tstateUpperLimit = (tstateSetSize > 0)
								   ? (isFirstTstateDeleted(domainIndex) ? upperLimitIndex - pstateSetSize + 1
																		: upperLimitIndex - pstateSetSize)
								   : Constants::Invalid;
		}

		if (upperLimitIndex > lowerLimitIndex || lowerLimitIndex > minIndex)
		{
			// If requested lower limit is out of bounds or upper limit is higher than lower limit (shouldn't happen
			// when arbitrating) Set _PDL and _TDL to the bottom index of their sets
			pstateLowerLimit = minIndex;
			tstateLowerLimit = (tstateSetSize > 0) ? tstateSetSize - 1 : Constants::Invalid;
			PARTICIPANT_LOG_MESSAGE_WARNING(
				{ return "Limit index mismatch, setting lower limit to lowest possible index."; });
		}
		else if (lowerLimitIndex < ((ppdl < combinedSetSize) ? ppdl : combinedSetSize))
		{
			pstateLowerLimit = lowerLimitIndex;
			tstateLowerLimit = (tstateSetSize > 0) ? 0 : Constants::Invalid;
		}
		else
		{
			pstateLowerLimit = minIndex;
			tstateLowerLimit = (tstateSetSize > 0)
								   ? (isFirstTstateDeleted(domainIndex) ? combinedSetSize - pstateSetSize + 1
																		: combinedSetSize - pstateSetSize)
								   : Constants::Invalid;
		}
	}

	m_performanceControlDynamicCaps.invalidate();

	if (pstateLowerLimit != Constants::Invalid)
	{
		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_PROC_PERF_PSTATE_DEPTH_LIMIT,
			pstateLowerLimit,
			domainIndex,
			Constants::Esif::NoPersistInstance);
	}

	if (tstateLowerLimit != Constants::Invalid)
	{
		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_PROC_PERF_TSTATE_DEPTH_LIMIT,
			tstateLowerLimit,
			domainIndex,
			Constants::Esif::NoPersistInstance);
	}

	if (pstateUpperLimit != Constants::Invalid)
	{
		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_PARTICIPANT_MAX_PERF_STATE,
			pstateUpperLimit,
			domainIndex,
			Constants::Esif::NoPersistInstance);
	}

	if (tstateUpperLimit != Constants::Invalid)
	{
		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_PROC_MAX_THROTTLE_STATE,
			tstateUpperLimit,
			domainIndex,
			Constants::Esif::NoPersistInstance);
	}
}

void DomainPerformanceControl_002::setPerformanceCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
	m_capabilitiesLocked = lock;
}

UIntN DomainPerformanceControl_002::getCurrentPerformanceControlIndex(UIntN participantIndex, UIntN domainIndex)
{
	return getPerformanceControlStatus(participantIndex, domainIndex).getCurrentControlSetIndex();
}

void DomainPerformanceControl_002::onClearCachedData(void)
{
	m_performanceControlSet.invalidate();
	m_performanceControlDynamicCaps.invalidate();
	m_performanceControlStaticCaps.invalidate();
	m_performanceStateSet.invalidate();
	m_throttlingStateSet.invalidate();
	m_isFirstTstateDeleted.invalidate();

	if (m_capabilitiesLocked == false)
	{
		try
		{
			DptfBuffer depthLimitBuffer = createResetPrimitiveTupleBinary(
				esif_primitive_type::SET_PROC_PERF_PSTATE_DEPTH_LIMIT, Constants::Esif::NoPersistInstance);
			getParticipantServices()->primitiveExecuteSet(
				esif_primitive_type::SET_CONFIG_RESET,
				ESIF_DATA_BINARY,
				depthLimitBuffer.get(),
				depthLimitBuffer.size(),
				depthLimitBuffer.size(),
				0,
				Constants::Esif::NoInstance);

			depthLimitBuffer = createResetPrimitiveTupleBinary(
				esif_primitive_type::SET_PROC_PERF_TSTATE_DEPTH_LIMIT, Constants::Esif::NoPersistInstance);
			getParticipantServices()->primitiveExecuteSet(
				esif_primitive_type::SET_CONFIG_RESET,
				ESIF_DATA_BINARY,
				depthLimitBuffer.get(),
				depthLimitBuffer.size(),
				depthLimitBuffer.size(),
				0,
				Constants::Esif::NoInstance);

			DptfBuffer upperLimitBuffer = createResetPrimitiveTupleBinary(
				esif_primitive_type::SET_PARTICIPANT_MAX_PERF_STATE, Constants::Esif::NoPersistInstance);
			getParticipantServices()->primitiveExecuteSet(
				esif_primitive_type::SET_CONFIG_RESET,
				ESIF_DATA_BINARY,
				upperLimitBuffer.get(),
				upperLimitBuffer.size(),
				upperLimitBuffer.size(),
				0,
				Constants::Esif::NoInstance);

			upperLimitBuffer = createResetPrimitiveTupleBinary(
				esif_primitive_type::SET_PROC_MAX_THROTTLE_STATE, Constants::Esif::NoPersistInstance);
			getParticipantServices()->primitiveExecuteSet(
				esif_primitive_type::SET_CONFIG_RESET,
				ESIF_DATA_BINARY,
				upperLimitBuffer.get(),
				upperLimitBuffer.size(),
				upperLimitBuffer.size(),
				0,
				Constants::Esif::NoInstance);
		}
		catch (...)
		{
			// best effort
			PARTICIPANT_LOG_MESSAGE_DEBUG(
				{ return "Failed to restore the initial performance control capabilities. "; });
		}
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
	UIntN overallUpperLimitIndex = Constants::Invalid;
	UIntN overallLowerLimitIndex = Constants::Invalid;
	arbitratePerformanceStateLimits(
		domainIndex,
		pStateUpperLimitIndex,
		pStateLowerLimitIndex,
		tStateUpperLimitIndex,
		tStateLowerLimitIndex,
		overallUpperLimitIndex,
		overallLowerLimitIndex);
	return PerformanceControlDynamicCaps(overallLowerLimitIndex, overallUpperLimitIndex);
}

void DomainPerformanceControl_002::calculatePerformanceStateLimits(
	UIntN& pStateUpperLimitIndex,
	UIntN& pStateLowerLimitIndex,
	UIntN domainIndex)
{
	// Set lower limit as min(ppdl value, last index) and
	// upper limit as first index.
	// If table is empty, these are set as invalid.
	UInt32 ppdl;
	auto performanceStateSetSize = getPerformanceStateSet(domainIndex).getCount();
	if (performanceStateSetSize == 0)
	{
		pStateLowerLimitIndex = Constants::Invalid;
		pStateUpperLimitIndex = Constants::Invalid;
	}
	else
	{
		try
		{
			ppdl = getParticipantServices()->primitiveExecuteGetAsUInt32(
				esif_primitive_type::GET_PROC_PERF_PSTATE_DEPTH_LIMIT, domainIndex);
		}
		catch (...)
		{
			ppdl = performanceStateSetSize - 1;
			getParticipantServices()->primitiveExecuteSetAsUInt32(
				esif_primitive_type::SET_PROC_PERF_PSTATE_DEPTH_LIMIT, ppdl, domainIndex);
		}
		pStateUpperLimitIndex = 0;
		pStateLowerLimitIndex =
			static_cast<UIntN>((ppdl >= pStateUpperLimitIndex && ppdl < performanceStateSetSize) ? ppdl : performanceStateSetSize - 1);
	}
}

void DomainPerformanceControl_002::calculateThrottlingStateLimits(
	UIntN& tStateUpperLimitIndex,
	UIntN& tStateLowerLimitIndex,
	UIntN domainIndex)
{
	// Required object if T-states are supported
	try
	{
		tStateUpperLimitIndex = getParticipantServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_PROC_PERF_THROTTLE_PRESENT_CAPABILITY, domainIndex);
	}
	catch (dptf_exception&)
	{
		PARTICIPANT_LOG_MESSAGE_WARNING({ return "Bad upper T-state limit."; });
		tStateUpperLimitIndex = 0;
	}

	auto throttlingStateSetSize = getThrottlingStateSet(domainIndex).getCount();
	try
	{
		// _TDL is an optional object
		tStateLowerLimitIndex = getParticipantServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_PROC_PERF_TSTATE_DEPTH_LIMIT, domainIndex);
	}
	catch (dptf_exception&)
	{
		// Optional object.  Default value is T(n)
		tStateLowerLimitIndex = static_cast<UIntN>(throttlingStateSetSize - 1);
	}

	if (tStateUpperLimitIndex >= throttlingStateSetSize)
	{
		tStateUpperLimitIndex = 0;
		PARTICIPANT_LOG_MESSAGE_WARNING({
			return "Retrieved upper T-state index limit is out of control set bounds, ignoring upper index limit.";
		});
	}

	if (tStateLowerLimitIndex >= throttlingStateSetSize || tStateUpperLimitIndex > tStateLowerLimitIndex)
	{
		// If the limits are bad, ignore the lower limit.  Don't fail this control.
		tStateLowerLimitIndex = static_cast<UIntN>(throttlingStateSetSize - 1);
		PARTICIPANT_LOG_MESSAGE_WARNING(
			{ return "T-state limit indexes are mismatched, ignoring lower index limit."; });
	}
}

void DomainPerformanceControl_002::arbitratePerformanceStateLimits(
	UIntN domainIndex,
	UIntN pStateUpperLimitIndex,
	UIntN pStateLowerLimitIndex,
	UIntN tStateUpperLimitIndex,
	UIntN tStateLowerLimitIndex,
	UIntN& overallUpperLimitIndex,
	UIntN& overallLowerLimitIndex)
{
	auto performanceStateSetSize = getPerformanceStateSet(domainIndex).getCount();
	auto throttlingStateSetSize = getThrottlingStateSet(domainIndex).getCount();

	overallUpperLimitIndex = pStateUpperLimitIndex;
	if ((pStateUpperLimitIndex >= performanceStateSetSize - 1 || tStateUpperLimitIndex != 0)
		&& throttlingStateSetSize > 0) // If _PPC is P(n) or _TPC is non-zero, ignore P-States
	{
		overallUpperLimitIndex = isFirstTstateDeleted(domainIndex) ? performanceStateSetSize + tStateUpperLimitIndex - 1
																   : performanceStateSetSize + tStateUpperLimitIndex;
	}

	overallUpperLimitIndex = std::max(overallUpperLimitIndex, m_tdpFrequencyLimitControlIndex);
	PARTICIPANT_LOG_MESSAGE_DEBUG(
		{ return "Performance upper limit index is: " + std::to_string(overallUpperLimitIndex); });

	overallLowerLimitIndex = pStateLowerLimitIndex;
	if (pStateLowerLimitIndex >= performanceStateSetSize - 1 && throttlingStateSetSize > 0) // _PDL is P(n)
	{
		overallLowerLimitIndex = isFirstTstateDeleted(domainIndex) ? performanceStateSetSize + tStateLowerLimitIndex - 1
																   : performanceStateSetSize + tStateLowerLimitIndex;
	}

	PARTICIPANT_LOG_MESSAGE_DEBUG(
		{ return "Performance lower limit index is: " + std::to_string(overallLowerLimitIndex); });
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

			// Removing the first Tstate if the last Pstate and first Tstate are same.
			if (throttlingStateSet.getCount() > 0)
			{
				auto lastPstateIndex = performanceStateSet.getCount() - 1;
				if (performanceStateSet[lastPstateIndex].getControlAbsoluteValue()
					== throttlingStateSet[0].getControlAbsoluteValue())
				{
					m_isFirstTstateDeleted.set(true);
				}
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

	PARTICIPANT_LOG_MESSAGE_DEBUG({
		std::stringstream message;
		message << "Performance controls created."
				<< "Total Entries: " << combinedStateSet.getCount() << ", "
				<< "P-State Count: " << performanceStateSet.getCount() << ", "
				<< "T-State Count: " << throttlingStateSet.getCount();
		return message.str();
	});

	return PerformanceControlSet(combinedStateSet);
}

UIntN DomainPerformanceControl_002::snapIfPerformanceControlIndexIsOutOfBounds(
	UIntN domainIndex,
	UIntN performanceControlIndex)
{
	auto caps = getPerformanceControlDynamicCaps(getParticipantIndex(), domainIndex);
	auto capsUpperLimitIndex = caps.getCurrentUpperLimitIndex();
	auto capsLowerLimitIndex = caps.getCurrentLowerLimitIndex();
	if (performanceControlIndex < capsUpperLimitIndex)
	{
		PARTICIPANT_LOG_MESSAGE_WARNING(
			{ return "Performance control index < upper limit index. Snapping to upper limit index."; });
		performanceControlIndex = capsUpperLimitIndex;
	}
	else if (performanceControlIndex > capsLowerLimitIndex)
	{
		PARTICIPANT_LOG_MESSAGE_WARNING(
			{ return "Performance control index > lower limit index. Snapping to lower limit index."; });
		performanceControlIndex = capsLowerLimitIndex;
	}
	return performanceControlIndex;
}

std::shared_ptr<XmlNode> DomainPerformanceControl_002::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("performance_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(getPerformanceControlStatus(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getPerformanceControlDynamicCaps(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getPerformanceControlSet(getParticipantIndex(), domainIndex).getXml());
	root->addChild(XmlNode::createDataElement("control_knob_version", "002"));

	return root;
}

void DomainPerformanceControl_002::capture(void)
{
	try
	{
		m_initialStatus.set(getPerformanceControlDynamicCaps(getParticipantIndex(), getDomainIndex()));
		PARTICIPANT_LOG_MESSAGE_DEBUG({
			std::stringstream message;
			message << "Initial performance capabilities are captured. MIN = "
					<< std::to_string(m_initialStatus.get().getCurrentLowerLimitIndex())
					<< " & MAX = " << std::to_string(m_initialStatus.get().getCurrentUpperLimitIndex());
			return message.str();
		});
	}
	catch (dptf_exception& ex)
	{
		m_initialStatus.invalidate();
		PARTICIPANT_LOG_MESSAGE_WARNING_EX({
			return "Failed to get the initial processor performance control dynamic capabilities. "
				   + ex.getDescription();
		});
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
			PARTICIPANT_LOG_MESSAGE_DEBUG(
				{ return "Restoring... P-state = P" + std::to_string(restoreIndex) + " & T-state = T0."; });

			auto throttlingStateSet = getThrottlingStateSet(domainIndex);
			// Set T0
			if (throttlingStateSet.getCount() > 0)
			{
				getParticipantServices()->primitiveExecuteSetAsUInt32(
					esif_primitive_type::SET_TSTATE_CURRENT, throttlingStateSet[0].getControlId(), domainIndex);
			}
			getParticipantServices()->primitiveExecuteSetAsUInt32(
				esif_primitive_type::SET_PERF_PRESENT_CAPABILITY, restoreIndex, domainIndex);
		}
		catch (...)
		{
			// best effort
			PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to restore the initial performance control status. "; });
		}
	}
}

void DomainPerformanceControl_002::updateBasedOnConfigTdpInformation(
	UIntN participantIndex,
	UIntN domainIndex,
	ConfigTdpControlSet configTdpControlSet,
	ConfigTdpControlStatus configTdpControlStatus)
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
	return "Processor Participant (CPU Domain) Performance Control";
}
