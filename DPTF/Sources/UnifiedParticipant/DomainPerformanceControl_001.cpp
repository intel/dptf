/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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
	clearCachedData();
	capture();
}

DomainPerformanceControl_001::~DomainPerformanceControl_001(void)
{
	restore();
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
	if (performanceControlIndex == getCurrentPerformanceControlIndex(participantIndex, domainIndex))
	{
		getParticipantServices()->writeMessageDebug(
			ParticipantMessage(FLF, "Requested limit = current limit.  Ignoring."));
		return;
	}

	try
	{
		throwIfPerformanceControlIndexIsOutOfBounds(domainIndex, performanceControlIndex);
		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_PERF_PRESENT_CAPABILITY, performanceControlIndex, domainIndex);

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
		if (upperLimitIndex >= controlSetSize)
		{
			throw dptf_exception("Upper Limit index is out of control set bounds.");
		}
		else if (upperLimitIndex > lowerLimitIndex || lowerLimitIndex >= controlSetSize)
		{
			lowerLimitIndex = controlSetSize - 1;
			getParticipantServices()->writeMessageWarning(
				ParticipantMessage(FLF, "Limit index mismatch, setting lower limit to lowest possible index."));
		}
	}

	m_performanceControlDynamicCaps.invalidate();

	if (lowerLimitIndex != Constants::Invalid)
	{
		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_PERF_PSTATE_DEPTH_LIMIT,
			lowerLimitIndex,
			domainIndex,
			Constants::Esif::NoPersistInstance);
	}

	if (upperLimitIndex != Constants::Invalid)
	{
		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_PARTICIPANT_MAX_PERF_STATE,
			upperLimitIndex,
			domainIndex,
			Constants::Esif::NoPersistInstance);
	}
}

void DomainPerformanceControl_001::setPerformanceCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
	m_capabilitiesLocked = lock;
}

UIntN DomainPerformanceControl_001::getCurrentPerformanceControlIndex(UIntN participantIndex, UIntN domainIndex)
{
	return getPerformanceControlStatus(participantIndex, domainIndex).getCurrentControlSetIndex();
}

void DomainPerformanceControl_001::clearCachedData(void)
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
			getParticipantServices()->writeMessageDebug(
				ParticipantMessage(FLF, "Failed to restore the initial performance control capabilities."));
		}
	}
}

std::shared_ptr<XmlNode> DomainPerformanceControl_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("performance_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(getPerformanceControlStatus(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getPerformanceControlDynamicCaps(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getPerformanceControlStaticCaps(getParticipantIndex(), domainIndex).getXml());
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
	catch (dptf_exception& e)
	{
		m_initialStatus.invalidate();
		std::string warningMsg = e.what();
		getParticipantServices()->writeMessageWarning(ParticipantMessage(
			FLF, "Failed to get the initial performance control dynamic capabilities. " + warningMsg));
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
			getParticipantServices()->writeMessageDebug(
				ParticipantMessage(FLF, "Failed to restore the initial performance control status. "));
		}
	}
}

void DomainPerformanceControl_001::updateBasedOnConfigTdpInformation(
	UIntN participantIndex,
	UIntN domainIndex,
	ConfigTdpControlSet configTdpControlSet,
	ConfigTdpControlStatus configTdpControlStatus)
{
	throw not_implemented();
}

PerformanceControlStaticCaps DomainPerformanceControl_001::createPerformanceControlStaticCaps()
{
	return PerformanceControlStaticCaps(false); // This is hard-coded to FALSE in 7.0
}

PerformanceControlDynamicCaps DomainPerformanceControl_001::createPerformanceControlDynamicCaps(UIntN domainIndex)
{
	// Get dynamic caps
	UInt32 lowerLimitIndex;
	UInt32 upperLimitIndex;
	auto controlSetSize = getPerformanceControlSet(getParticipantIndex(), domainIndex).getCount();

	try
	{
		lowerLimitIndex = getParticipantServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_PERF_PSTATE_DEPTH_LIMIT, domainIndex);
	}
	catch (...)
	{
		// If PPDL is not supported, default to Pn.
		lowerLimitIndex = controlSetSize - 1;
	}

	try
	{
		// If PPPC is not supported, default to P0
		upperLimitIndex = getParticipantServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_PARTICIPANT_PERF_PRESENT_CAPABILITY, domainIndex);
	}
	catch (...)
	{
		upperLimitIndex = 0;
	}

	if (upperLimitIndex >= controlSetSize)
	{
		upperLimitIndex = controlSetSize - 1;
		lowerLimitIndex = upperLimitIndex;
		getParticipantServices()->writeMessageWarning(
			ParticipantMessage(FLF, "Retrieved upper limit control index out of control set bounds., ignoring upper limit."));
	}

	if (lowerLimitIndex >= controlSetSize || upperLimitIndex > lowerLimitIndex)
	{
		lowerLimitIndex = controlSetSize - 1;
		getParticipantServices()->writeMessageWarning(
			ParticipantMessage(FLF, "Performance state limit indexes are mismatched, ignoring lower limit."));
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

void DomainPerformanceControl_001::throwIfPerformanceControlIndexIsOutOfBounds(
	UIntN domainIndex,
	UIntN performanceControlIndex)
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
	if (performanceControlIndex < caps.getCurrentUpperLimitIndex()
		|| performanceControlIndex > caps.getCurrentLowerLimitIndex())
	{
		std::stringstream infoMessage;

		infoMessage << "Got a performance control index that was outside the allowable range." << std::endl
			<< "Desired Index : " << performanceControlIndex << std::endl
			<< "Upper Limit Index : " << caps.getCurrentUpperLimitIndex() << std::endl
			<< "Lower Limit Index : " << caps.getCurrentLowerLimitIndex() << std::endl;

		throw dptf_exception(infoMessage.str());
	}
}

std::string DomainPerformanceControl_001::getName(void)
{
	return "Generic Participant Performance Control";
}
