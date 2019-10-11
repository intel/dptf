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

#include "DomainActiveControl_001.h"
#include "XmlNode.h"
#include "ParticipantLogger.h"

DomainActiveControl_001::DomainActiveControl_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainActiveControlBase(participantIndex, domainIndex, participantServicesInterface)
	, m_capabilitiesLocked(false)
{
	onClearCachedData();
	capture();
}

DomainActiveControl_001::~DomainActiveControl_001(void)
{
	restore();
}

DptfBuffer DomainActiveControl_001::getActiveControlStaticCaps(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_FAN_INFORMATION, ESIF_DATA_BINARY, domainIndex);
}

DptfBuffer DomainActiveControl_001::getActiveControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		return getParticipantServices()->primitiveExecuteGet(
			esif_primitive_type::GET_FAN_CAPABILITIES, ESIF_DATA_BINARY, domainIndex);
	}
	catch (...)
	{
		Percentage minSpeed = Percentage::fromWholeNumber(0);
		Percentage maxSpeed = Percentage::fromWholeNumber(100);
		return ActiveControlDynamicCaps(minSpeed, maxSpeed).toFcdcBinary();
	}
}

DptfBuffer DomainActiveControl_001::getActiveControlStatus(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_FAN_STATUS, ESIF_DATA_BINARY, domainIndex);
}

DptfBuffer DomainActiveControl_001::getActiveControlSet(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_FAN_PERFORMANCE_STATES, ESIF_DATA_BINARY, domainIndex);
}

void DomainActiveControl_001::setActiveControl(UIntN participantIndex, UIntN domainIndex, const Percentage& fanSpeed)
{
	throwIfFineGrainedControlIsNotSupported(participantIndex, domainIndex);

	// FIXME: For now SET_FAN_LEVEL doesn't follow the normal rule for percentages.  For this we pass in a whole number
	// which would be 90 for 90%.  This is an exception and should be corrected in the future.
	if (fanSpeed.isValid() == false)
	{
		throw dptf_exception("Fan speed percentage is not valid.");
	}
	UInt32 convertedFanSpeedPercentage = fanSpeed.toWholeNumber();
	getParticipantServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_FAN_LEVEL, convertedFanSpeedPercentage, domainIndex);
}

void DomainActiveControl_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			throwIfFineGrainedControlIsNotSupported(participantIndex, domainIndex);

			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL;
			capability.size = sizeof(capability);

			// Control ID and speed polled in ESIF; so not updated here
			auto dynamicCapabilities =
				ActiveControlDynamicCaps::createFromFcdc(getActiveControlDynamicCaps(participantIndex, domainIndex));
			capability.data.activeControl.lowerLimit = dynamicCapabilities.getMinFanSpeed().toWholeNumber();
			capability.data.activeControl.upperLimit = dynamicCapabilities.getMaxFanSpeed().toWholeNumber();

			getParticipantServices()->sendDptfEvent(
				ParticipantEvent::DptfParticipantControlAction,
				domainIndex,
				Capability::getEsifDataFromCapabilityData(&capability));

			PARTICIPANT_LOG_MESSAGE_INFO({
				std::stringstream message;
				message << "Published activity for participant " << getParticipantIndex() << ", "
						<< "domain " << getName() << " "
						<< "("
						<< "Active Control"
						<< ")";
				return message.str();
			});
		}
	}
	catch (...)
	{
		// skip if there are any issue in sending log data
		// a participant capability bit can be set and the corresponding version can be > 0 as well
		// but the participant may not implement that capability.
		// For e.g, skin sensor participant need not implement performance control even though
		// that capability is set in dsp
	}
}

void DomainActiveControl_001::onClearCachedData(void)
{
	if (m_capabilitiesLocked == false)
	{
		try
		{
			DptfBuffer capabilitiesBuffer = createResetPrimitiveTupleBinary(
				esif_primitive_type::SET_FAN_CAPABILITIES, Constants::Esif::NoPersistInstance);
			getParticipantServices()->primitiveExecuteSet(
				esif_primitive_type::SET_CONFIG_RESET,
				ESIF_DATA_BINARY,
				capabilitiesBuffer.get(),
				capabilitiesBuffer.size(),
				capabilitiesBuffer.size(),
				0,
				Constants::Esif::NoInstance);
		}
		catch (...)
		{
			// best effort
			PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to restore the initial active control capabilities. "; });
		}
	}
}

std::shared_ptr<XmlNode> DomainActiveControl_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("active_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	auto status = ActiveControlStatus::createFromFst(getActiveControlStatus(getParticipantIndex(), domainIndex));
	root->addChild(status.getXml());
	auto staticCaps =
		ActiveControlStaticCaps::createFromFif(getActiveControlStaticCaps(getParticipantIndex(), domainIndex));
	root->addChild(staticCaps.getXml());
	auto dynamicCaps =
		ActiveControlDynamicCaps::createFromFcdc(getActiveControlDynamicCaps(getParticipantIndex(), domainIndex));
	root->addChild(dynamicCaps.getXml());
	auto controlSet = ActiveControlSet::createFromFps(getActiveControlSet(getParticipantIndex(), domainIndex));
	root->addChild(controlSet.getXml());
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));
	return root;
}

void DomainActiveControl_001::capture(void)
{
	try
	{
		auto status =
			ActiveControlStatus::createFromFst(getActiveControlStatus(getParticipantIndex(), getDomainIndex()));
		m_initialStatus.set(status);
	}
	catch (dptf_exception& ex)
	{
		m_initialStatus.invalidate();
		PARTICIPANT_LOG_MESSAGE_WARNING_EX(
			{ return "Failed to get the initial active control status. " + ex.getDescription(); });
	}
}

void DomainActiveControl_001::restore(void)
{
	if (m_initialStatus.isValid())
	{
		try
		{
			getParticipantServices()->primitiveExecuteSetAsUInt32(
				esif_primitive_type::SET_FAN_LEVEL, m_initialStatus.get().getCurrentControlId(), getDomainIndex());
		}
		catch (...)
		{
			// best effort
			PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to restore the initial active control status. "; });
		}
	}
}

ActiveControlSet DomainActiveControl_001::createActiveControlSet(UIntN domainIndex)
{
	DptfBuffer buffer = getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_FAN_PERFORMANCE_STATES, ESIF_DATA_BINARY, domainIndex);
	return ActiveControlSet::createFromFps(buffer);
}

void DomainActiveControl_001::throwIfFineGrainedControlIsNotSupported(UIntN participantIndex, UIntN domainIndex)
{
	auto staticCaps = ActiveControlStaticCaps::createFromFif(getActiveControlStaticCaps(participantIndex, domainIndex));
	if (staticCaps.supportsFineGrainedControl() == false)
	{
		throw dptf_exception("Fine grain control is not supported.");
	}
}

std::string DomainActiveControl_001::getName(void)
{
	return "Active Control";
}

void DomainActiveControl_001::setFanCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
	m_capabilitiesLocked = lock;
}

void DomainActiveControl_001::setActiveControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex,
	ActiveControlDynamicCaps newCapabilities)
{
	DptfBuffer buffer = newCapabilities.toFcdcBinary();
	getParticipantServices()->primitiveExecuteSet(
		esif_primitive_type::SET_FAN_CAPABILITIES,
		ESIF_DATA_BINARY,
		buffer.get(),
		buffer.size(),
		buffer.size(),
		domainIndex,
		Constants::Esif::NoPersistInstance);
}
