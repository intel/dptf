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

#include "DomainActiveControl_001.h"
#include "XmlNode.h"

DomainActiveControl_001::DomainActiveControl_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainActiveControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	clearCachedData();
	capture();
}

DomainActiveControl_001::~DomainActiveControl_001(void)
{
	restore();
}

ActiveControlStaticCaps DomainActiveControl_001::getActiveControlStaticCaps(UIntN participantIndex, UIntN domainIndex)
{
	if (m_activeControlStaticCaps.isInvalid())
	{
		m_activeControlStaticCaps.set(createActiveControlStaticCaps(domainIndex));
	}

	return m_activeControlStaticCaps.get();
}

ActiveControlDynamicCaps DomainActiveControl_001::getActiveControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
	if (m_activeControlDynamicCaps.isInvalid())
	{
		m_activeControlDynamicCaps.set(createActiveControlDynamicCaps(domainIndex));
	}

	return m_activeControlDynamicCaps.get();
}

ActiveControlStatus DomainActiveControl_001::getActiveControlStatus(UIntN participantIndex, UIntN domainIndex)
{
	m_activeControlStatus.set(createActiveControlStatus(domainIndex));

	return m_activeControlStatus.get();
}

ActiveControlSet DomainActiveControl_001::getActiveControlSet(UIntN participantIndex, UIntN domainIndex)
{
	if (m_activeControlSet.isInvalid())
	{
		m_activeControlSet.set(createActiveControlSet(domainIndex));
	}

	return m_activeControlSet.get();
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
			UInt32 controlId = getActiveControlStatus(participantIndex, domainIndex).getCurrentControlId();

			if (controlId == Constants::Invalid)
			{
				controlId = 0;
			}
			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL;
			capability.size = sizeof(capability);
			capability.data.activeControl.controlId = controlId;
			capability.data.activeControl.speed =
				getActiveControlStatus(participantIndex, domainIndex).getCurrentSpeed();

			getParticipantServices()->sendDptfEvent(
				ParticipantEvent::DptfParticipantControlAction,
				domainIndex,
				Capability::getEsifDataFromCapabilityData(&capability));

			std::stringstream message;
			message << "Published activity for participant " << getParticipantIndex() << ", "
					<< "domain " << getName() << " "
					<< "("
					<< "Active Control"
					<< ")";
			getParticipantServices()->writeMessageInfo(ParticipantMessage(FLF, message.str()));
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

void DomainActiveControl_001::clearCachedData(void)
{
	m_activeControlSet.invalidate();
	m_activeControlStaticCaps.invalidate();
	m_activeControlDynamicCaps.invalidate();
	m_activeControlStatus.invalidate();
}

std::shared_ptr<XmlNode> DomainActiveControl_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("active_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(getActiveControlStatus(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getActiveControlStaticCaps(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getActiveControlDynamicCaps(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getActiveControlSet(getParticipantIndex(), domainIndex).getXml());
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

	return root;
}

void DomainActiveControl_001::capture(void)
{
	try
	{
		m_initialStatus.set(getActiveControlStatus(getParticipantIndex(), getDomainIndex()));
	}
	catch (dptf_exception& e)
	{
		m_initialStatus.invalidate();
		std::string warningMsg = e.what();
		getParticipantServices()->writeMessageWarning(
			ParticipantMessage(FLF, "Failed to get the initial active control status. " + warningMsg));
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
			getParticipantServices()->writeMessageDebug(
				ParticipantMessage(FLF, "Failed to restore the initial active control status. "));
		}
	}
}

ActiveControlStaticCaps DomainActiveControl_001::createActiveControlStaticCaps(UIntN domainIndex)
{
	DptfBuffer buffer = getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_FAN_INFORMATION, ESIF_DATA_BINARY, domainIndex);
	return ActiveControlStaticCaps::createFromFif(buffer);
}

ActiveControlDynamicCaps DomainActiveControl_001::createActiveControlDynamicCaps(UIntN domainIndex)
{
	try
	{
		DptfBuffer buffer = getParticipantServices()->primitiveExecuteGet(
			esif_primitive_type::GET_FAN_CAPABILITIES, ESIF_DATA_BINARY, domainIndex);
		return ActiveControlDynamicCaps::createFromFcdc(buffer);
	}
	catch (...)
	{
		Percentage minSpeed = Percentage::fromWholeNumber(0);
		Percentage maxSpeed = Percentage::fromWholeNumber(100);
		return ActiveControlDynamicCaps(minSpeed, maxSpeed);
	}
}

ActiveControlStatus DomainActiveControl_001::createActiveControlStatus(UIntN domainIndex)
{
	DptfBuffer buffer = getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_FAN_STATUS, ESIF_DATA_BINARY, domainIndex);
	return ActiveControlStatus::createFromFst(buffer);
}

ActiveControlSet DomainActiveControl_001::createActiveControlSet(UIntN domainIndex)
{
	DptfBuffer buffer = getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_FAN_PERFORMANCE_STATES, ESIF_DATA_BINARY, domainIndex);
	return ActiveControlSet::createFromFps(buffer);
}

void DomainActiveControl_001::throwIfFineGrainedControlIsNotSupported(UIntN participantIndex, UIntN domainIndex)
{
	if (getActiveControlStaticCaps(participantIndex, domainIndex).supportsFineGrainedControl() == false)
	{
		throw dptf_exception("Fine grain control is not supported.");
	}
}

std::string DomainActiveControl_001::getName(void)
{
	return "Active Control";
}
