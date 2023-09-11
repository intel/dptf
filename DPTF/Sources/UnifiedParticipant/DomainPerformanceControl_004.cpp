/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "DomainPerformanceControl_004.h"
#include "XmlNode.h"

// OSTF Participant Performance Controls

DomainPerformanceControl_004::DomainPerformanceControl_004(
	UIntN participantIndex,
	UIntN domainIndex,
	const std::shared_ptr<ParticipantServicesInterface>& participantServicesInterface)
	: DomainPerformanceControlBase(participantIndex, domainIndex, participantServicesInterface)
	, m_capabilitiesLocked(false)
{
	DomainPerformanceControl_004::onClearCachedData();
	DomainPerformanceControl_004::capture();
}

DomainPerformanceControl_004::~DomainPerformanceControl_004()
{
	DomainPerformanceControl_004::restore();
}

PerformanceControlStaticCaps DomainPerformanceControl_004::getPerformanceControlStaticCaps(
	UIntN participantIndex,
	UIntN domainIndex)
{
	if (m_performanceControlStaticCaps.isInvalid())
	{
		m_performanceControlStaticCaps.set(createPerformanceControlStaticCaps());
	}
	return m_performanceControlStaticCaps.get();
}

PerformanceControlDynamicCaps DomainPerformanceControl_004::getPerformanceControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex)
{
	if (m_performanceControlDynamicCaps.isInvalid())
	{
		m_performanceControlDynamicCaps.set(createPerformanceControlDynamicCaps(domainIndex));
	}
	return m_performanceControlDynamicCaps.get();
}

PerformanceControlStatus DomainPerformanceControl_004::getPerformanceControlStatus(
	UIntN participantIndex,
	UIntN domainIndex)
{
	if (m_performanceControlStatus.isInvalid())
	{
		m_performanceControlStatus.set(createPerformanceControlStatus(domainIndex));
	}
	return m_performanceControlStatus.get();
}

PerformanceControlSet DomainPerformanceControl_004::getPerformanceControlSet(UIntN participantIndex, UIntN domainIndex)
{
	if (m_performanceControlSet.isInvalid())
	{
		m_performanceControlSet.set(createPerformanceControlSet(domainIndex));
	}
	return m_performanceControlSet.get();
}

void DomainPerformanceControl_004::setPerformanceControl(
	UIntN participantIndex,
	UIntN domainIndex,
	UIntN performanceControlIndex)
{
	PARTICIPANT_LOG_MESSAGE_DEBUG(
		{ return "Requesting Performance Control Index: " + std::to_string(performanceControlIndex); });

	if (performanceControlIndex == getCurrentPerformanceControlIndex(participantIndex, domainIndex))
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Requested limit = current limit.  Ignoring."; });

		return;
	}

	try
	{
		performanceControlIndex = snapIfPerformanceControlIndexIsOutOfBounds(domainIndex, performanceControlIndex);

		// Set control ID instead of the performance control index. However, store the index as the status for policies
		// to read.
		const auto performanceControlSet = getPerformanceControlSet(participantIndex, domainIndex);
		const auto controlIdRequested = performanceControlSet[performanceControlIndex].getControlId();
		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_PERF_PRESENT_CAPABILITY, controlIdRequested, domainIndex);

		PARTICIPANT_LOG_MESSAGE_DEBUG(
			{ return "Set Performance Control Index: " + std::to_string(performanceControlIndex); });
		// Refresh the status
		m_performanceControlStatus.set(PerformanceControlStatus(performanceControlIndex));
	}
	catch (...)
	{
		// eat any errors
	}
}

void DomainPerformanceControl_004::setPerformanceControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex,
	PerformanceControlDynamicCaps newCapabilities)
{
	auto upperLimitIndex = newCapabilities.getCurrentUpperLimitIndex();
	auto lowerLimitIndex = newCapabilities.getCurrentLowerLimitIndex();

	if (upperLimitIndex != Constants::Invalid && lowerLimitIndex != Constants::Invalid)
	{
		const auto controlSetSize = getPerformanceControlSet(participantIndex, domainIndex).getCount();
		const auto ppdl = getParticipantServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_PERF_PSTATE_DEPTH_LIMIT, domainIndex);
		const auto minIndex = (ppdl < controlSetSize) ? ppdl : controlSetSize - 1;
		if (upperLimitIndex >= controlSetSize)
		{
			upperLimitIndex = 0;
			PARTICIPANT_LOG_MESSAGE_WARNING(
				{ return "Limit index mismatch, setting upper limit to highest possible index."; });
		}
		if (upperLimitIndex > lowerLimitIndex || lowerLimitIndex > minIndex)
		{
			lowerLimitIndex = minIndex;
			PARTICIPANT_LOG_MESSAGE_WARNING(
				{ return "Limit index mismatch, setting lower limit to lowest possible index."; });
		}
	}

	m_performanceControlDynamicCaps.invalidate();
	m_performanceControlDynamicCaps.set(PerformanceControlDynamicCaps(lowerLimitIndex, upperLimitIndex));
}

void DomainPerformanceControl_004::setPerformanceCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
	m_capabilitiesLocked = lock;
}

UIntN DomainPerformanceControl_004::getCurrentPerformanceControlIndex(UIntN participantIndex, UIntN domainIndex)
{
	return getPerformanceControlStatus(participantIndex, domainIndex).getCurrentControlSetIndex();
}

void DomainPerformanceControl_004::onClearCachedData()
{
	m_performanceControlSet.invalidate();
	m_performanceControlDynamicCaps.invalidate();
	m_performanceControlStaticCaps.invalidate();

	if (m_capabilitiesLocked == false)
	{
		try
		{
			const DptfBuffer depthLimitBuffer = createResetPrimitiveTupleBinary(
				esif_primitive_type::SET_PERF_PSTATE_DEPTH_LIMIT, Constants::Esif::NoPersistInstance);
			getParticipantServices()->primitiveExecuteSet(
				esif_primitive_type::SET_CONFIG_RESET,
				ESIF_DATA_BINARY,
				depthLimitBuffer.get(),
				depthLimitBuffer.size(),
				depthLimitBuffer.size(),
				0,
				Constants::Esif::NoInstance);

			const DptfBuffer upperLimitBuffer = createResetPrimitiveTupleBinary(
				esif_primitive_type::SET_PARTICIPANT_MAX_PERF_STATE, Constants::Esif::NoPersistInstance);
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
				{ return "Failed to restore the initial performance control capabilities."; });
		}
	}
}

std::shared_ptr<XmlNode> DomainPerformanceControl_004::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("performance_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(getPerformanceControlStatus(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getPerformanceControlDynamicCaps(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getPerformanceControlSet(getParticipantIndex(), domainIndex).getXml());
	root->addChild(XmlNode::createDataElement("control_knob_version", "004"));

	return root;
}

void DomainPerformanceControl_004::capture()
{
	try
	{
		m_initialStatus.set(getPerformanceControlDynamicCaps(getParticipantIndex(), getDomainIndex()));
	}
	catch (dptf_exception& ex)
	{
		m_initialStatus.invalidate();
		PARTICIPANT_LOG_MESSAGE_WARNING_EX(
			{ return "Failed to get the initial performance control dynamic capabilities. " + ex.getDescription(); });
	}
}

void DomainPerformanceControl_004::restore()
{
	try
	{
		if (m_initialStatus.isValid())
		{
			const auto upperLimitIndex = m_initialStatus.get().getCurrentUpperLimitIndex();
			const auto perfControlSet = getPerformanceControlSet(getParticipantIndex(), getDomainIndex());
			getParticipantServices()->primitiveExecuteSetAsUInt32(
				esif_primitive_type::SET_PERF_PRESENT_CAPABILITY,
				perfControlSet[upperLimitIndex].getControlId(),
				getDomainIndex());
		}
	}
	catch (...)
	{
		// best effort
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to restore the initial performance control status. "; });
	}
}

PerformanceControlStaticCaps DomainPerformanceControl_004::createPerformanceControlStaticCaps()
{
	return {false}; // This is hard-coded to FALSE in 7.0
}

PerformanceControlStatus DomainPerformanceControl_004::createPerformanceControlStatus(UIntN domainIndex)
{
	UInt32 activeIndex = Constants::Invalid;
	try
	{
		const auto perfControlSet = getPerformanceControlSet(getParticipantIndex(), domainIndex);
		const auto currentControlId = getParticipantServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_PARTICIPANT_PERF_PRESENT_CAPABILITY, domainIndex);

		for (UInt32 index = 0; index < perfControlSet.getCount(); ++index) {
			auto entry = perfControlSet[index];
			if (currentControlId == entry.getControlId()) {
				activeIndex = index;
				break;
			}
		}

	}
	catch (...) 
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to load the initial performance control status. "; });
	}

	return {activeIndex};
}

PerformanceControlDynamicCaps DomainPerformanceControl_004::createPerformanceControlDynamicCaps(UIntN domainIndex)
{
	// Get dynamic caps. Set lower limit as min(ppdl value, last index) and
	// upper limit as first index.
	// If table is empty, these are set as invalid.
	UInt32 lowerLimitIndex;
	UInt32 upperLimitIndex;
	UInt32 ppdl;
	const auto controlSetSize = getPerformanceControlSet(getParticipantIndex(), domainIndex).getCount();
	if (controlSetSize == 0)
	{
		lowerLimitIndex = Constants::Invalid;
		upperLimitIndex = Constants::Invalid;
	}
	else
	{
		try
		{
			ppdl = getParticipantServices()->primitiveExecuteGetAsUInt32(
				esif_primitive_type::GET_PERF_PSTATE_DEPTH_LIMIT, domainIndex);
		}
		catch (...)
		{
			ppdl = controlSetSize - 1;
			getParticipantServices()->primitiveExecuteSetAsUInt32(
				esif_primitive_type::SET_PERF_PSTATE_DEPTH_LIMIT, ppdl, domainIndex);
		}
		upperLimitIndex = 0;
		lowerLimitIndex = (ppdl >= upperLimitIndex && ppdl < controlSetSize) ? ppdl : controlSetSize - 1;
	}
	return {lowerLimitIndex, upperLimitIndex};
}

PerformanceControlSet DomainPerformanceControl_004::createPerformanceControlSet(UIntN domainIndex) const
{
	// Build PPSS table
	const DptfBuffer buffer = getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_PERF_SUPPORT_STATES, ESIF_DATA_BINARY, domainIndex);
	auto controlSet = PerformanceControlSet(PerformanceControlSet::createFromGenericPpss(buffer));

	if (controlSet.getCount() == 0)
	{
		throw dptf_exception("P-state set is empty.  Impossible if we support performance controls.");
	}

	return controlSet;
}

UIntN DomainPerformanceControl_004::snapIfPerformanceControlIndexIsOutOfBounds(
	UIntN domainIndex,
	UIntN performanceControlIndex)
{
	const auto caps = getPerformanceControlDynamicCaps(getParticipantIndex(), domainIndex);
	const auto capsUpperLimitIndex = caps.getCurrentUpperLimitIndex();
	const auto capsLowerLimitIndex = caps.getCurrentLowerLimitIndex();
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

std::string DomainPerformanceControl_004::getName()
{
	return "Control ID Performance Control";
}
