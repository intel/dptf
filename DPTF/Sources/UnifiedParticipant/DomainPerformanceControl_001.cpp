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

#include "DomainPerformanceControl_001.h"
#include "XmlNode.h"

// Generic Participant Performance Controls

DomainPerformanceControl_001::DomainPerformanceControl_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainPerformanceControlBase(participantIndex, domainIndex, participantServicesInterface)
	, m_capabilitiesLocked(false)
{
	onClearCachedData();
	capture();
}

DomainPerformanceControl_001::~DomainPerformanceControl_001(void)
{
	try
	{
		restore();
	}
	catch(...)
	{

	}
}

PerformanceControlStaticCaps DomainPerformanceControl_001::getPerformanceControlStaticCaps(
	UIntN participantIndex,
	UIntN domainIndex)
{
	if (m_performanceControlStaticCaps.isInvalid())
	{
		m_performanceControlStaticCaps.set(createPerformanceControlStaticCaps());
	}
	return m_performanceControlStaticCaps.get();
}

PerformanceControlDynamicCaps DomainPerformanceControl_001::getPerformanceControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex)
{
	if (m_performanceControlDynamicCaps.isInvalid())
	{
		m_performanceControlDynamicCaps.set(createPerformanceControlDynamicCaps(domainIndex));
	}
	return m_performanceControlDynamicCaps.get();
}

PerformanceControlStatus DomainPerformanceControl_001::getPerformanceControlStatus(
	UIntN participantIndex,
	UIntN domainIndex)
{
	if (m_performanceControlStatus.isInvalid())
	{
		m_performanceControlStatus.set(PerformanceControlStatus(Constants::Invalid));
	}
	return m_performanceControlStatus.get();
}

PerformanceControlSet DomainPerformanceControl_001::getPerformanceControlSet(UIntN participantIndex, UIntN domainIndex)
{
	if (m_performanceControlSet.isInvalid())
	{
		m_performanceControlSet.set(createPerformanceControlSet(domainIndex));
	}
	return m_performanceControlSet.get();
}

void DomainPerformanceControl_001::setPerformanceControl(
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
		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_PERF_PRESENT_CAPABILITY, performanceControlIndex, domainIndex);

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

void DomainPerformanceControl_001::setPerformanceControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex,
	PerformanceControlDynamicCaps newCapabilities)
{
	auto upperLimitIndex = newCapabilities.getCurrentUpperLimitIndex();
	auto lowerLimitIndex = newCapabilities.getCurrentLowerLimitIndex();

	if (upperLimitIndex != Constants::Invalid && lowerLimitIndex != Constants::Invalid)
	{
		auto controlSetSize = getPerformanceControlSet(participantIndex, domainIndex).getCount();
		auto ppdl = getParticipantServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_PERF_PSTATE_DEPTH_LIMIT, domainIndex);
		auto minIndex = (ppdl < controlSetSize) ? ppdl : controlSetSize - 1;
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
	getParticipantServices()->createEventPerformanceCapabilitiesChanged();
}

void DomainPerformanceControl_001::setPerformanceCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
	m_capabilitiesLocked = lock;
}

UIntN DomainPerformanceControl_001::getCurrentPerformanceControlIndex(UIntN participantIndex, UIntN domainIndex)
{
	return getPerformanceControlStatus(participantIndex, domainIndex).getCurrentControlSetIndex();
}

void DomainPerformanceControl_001::onClearCachedData(void)
{
	m_performanceControlSet.invalidate();
	m_performanceControlDynamicCaps.invalidate();
	m_performanceControlStaticCaps.invalidate();

	if (m_capabilitiesLocked == false)
	{
		try
		{
			DptfBuffer depthLimitBuffer = createResetPrimitiveTupleBinary(
				esif_primitive_type::SET_PERF_PSTATE_DEPTH_LIMIT, Constants::Esif::NoPersistInstance);
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
		}
		catch (...)
		{
			// best effort
			PARTICIPANT_LOG_MESSAGE_DEBUG(
				{ return "Failed to restore the initial performance control capabilities."; });
		}
	}
}

std::shared_ptr<XmlNode> DomainPerformanceControl_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("performance_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(getPerformanceControlStatus(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getPerformanceControlDynamicCaps(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getPerformanceControlSet(getParticipantIndex(), domainIndex).getXml());
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

	return root;
}

void DomainPerformanceControl_001::capture(void)
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

void DomainPerformanceControl_001::restore(void)
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
			PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to restore the initial performance control status. "; });
		}
	}
}

PerformanceControlStaticCaps DomainPerformanceControl_001::createPerformanceControlStaticCaps()
{
	return PerformanceControlStaticCaps(false); // This is hard-coded to FALSE in 7.0
}

PerformanceControlDynamicCaps DomainPerformanceControl_001::createPerformanceControlDynamicCaps(UIntN domainIndex)
{
	// Get dynamic caps. Set lower limit as min(ppdl value, last index) and
	// upper limit as first index.
	// If table is empty, these are set as invalid.
	UInt32 lowerLimitIndex;
	UInt32 upperLimitIndex;
	UInt32 ppdl;
	auto controlSetSize = getPerformanceControlSet(getParticipantIndex(), domainIndex).getCount();
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
	return PerformanceControlDynamicCaps(lowerLimitIndex, upperLimitIndex);
}

PerformanceControlSet DomainPerformanceControl_001::createPerformanceControlSet(UIntN domainIndex)
{
	// Build PPSS table
	DptfBuffer buffer = getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_PERF_SUPPORT_STATES, ESIF_DATA_BINARY, domainIndex);
	auto controlSet = PerformanceControlSet(PerformanceControlSet::createFromGenericPpss(buffer));

	if (controlSet.getCount() == 0)
	{
		throw dptf_exception("P-state set is empty.  Impossible if we support performance controls.");
	}

	return controlSet;
}

UIntN DomainPerformanceControl_001::snapIfPerformanceControlIndexIsOutOfBounds(
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

std::string DomainPerformanceControl_001::getName(void)
{
	return "Generic Participant Performance Control";
}

void DomainPerformanceControl_001::setPerfPreferenceMax(
	UIntN participantIndex,
	UIntN domainIndex,
	Percentage minMaxRatio)
{
	throw not_implemented();
}

void DomainPerformanceControl_001::setPerfPreferenceMin(
	UIntN participantIndex,
	UIntN domainIndex,
	Percentage minMaxRatio)
{
	throw not_implemented();
}
