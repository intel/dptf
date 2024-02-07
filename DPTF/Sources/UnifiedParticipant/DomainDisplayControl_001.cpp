/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "DomainDisplayControl_001.h"
#include "XmlNode.h"

DomainDisplayControl_001::DomainDisplayControl_001(
	UIntN participantIndex,
	UIntN domainIndex,
	const std::shared_ptr<ParticipantServicesInterface>& participantServicesInterface)
	: DomainDisplayControlBase(participantIndex, domainIndex, participantServicesInterface)
	, m_userPreferredIndex(Constants::Invalid)
	, m_lastSetDisplayBrightness(Constants::Invalid)
	, m_userPreferredSoftBrightnessIndex(Constants::Invalid)
	, m_capabilitiesLocked(false)
{
	DomainDisplayControl_001::onClearCachedData();
	ControlBase::capture();
}

DomainDisplayControl_001::~DomainDisplayControl_001()
{
	DomainDisplayControl_001::restore();
}

DisplayControlDynamicCaps DomainDisplayControl_001::getDisplayControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex)
{
	if (m_displayControlDynamicCaps.isInvalid())
	{
		m_displayControlDynamicCaps.set(createDisplayControlDynamicCaps(domainIndex));
	}
	return m_displayControlDynamicCaps.get();
}

DisplayControlStatus DomainDisplayControl_001::getDisplayControlStatus(UIntN participantIndex, UIntN domainIndex)
{
	const Percentage brightnessPercentage = getParticipantServices()->primitiveExecuteGetAsPercentage(
		esif_primitive_type::GET_DISPLAY_BRIGHTNESS, domainIndex);

	const auto currentIndex = getDisplayControlSet(participantIndex, domainIndex).getControlIndex(brightnessPercentage);

	return {currentIndex};
}

UIntN DomainDisplayControl_001::getUserPreferredDisplayIndex(UIntN participantIndex, UIntN domainIndex)
{
	if ((getParticipantServices()->isUserPreferredDisplayCacheValid(participantIndex, domainIndex)) == true)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({
			return "Attempting to get the user preferred index from the display cache.";
			});

		m_userPreferredIndex =
			getParticipantServices()->getUserPreferredDisplayCacheValue(participantIndex, domainIndex);

		PARTICIPANT_LOG_MESSAGE_DEBUG({
			return "Retrieved the user preferred index of " + std::to_string(m_userPreferredIndex) + " .";
			});

		getParticipantServices()->invalidateUserPreferredDisplayCache(participantIndex, domainIndex);
	}
	else
	{
		const auto currentStatus = getDisplayControlStatus(participantIndex, domainIndex);
		const auto currentIndex = currentStatus.getBrightnessLimitIndex();

		if (m_userPreferredIndex == Constants::Invalid
			|| (m_lastSetDisplayBrightness != Constants::Invalid && currentIndex != m_lastSetDisplayBrightness))
		{
			m_userPreferredIndex = currentIndex;
			m_isUserPreferredIndexModified = true;
		}
		else
		{
			m_isUserPreferredIndexModified = false;
		}
	}

	return m_userPreferredIndex;
}

UIntN DomainDisplayControl_001::getUserPreferredSoftBrightnessIndex(UIntN participantIndex, UIntN domainIndex)
{
	return m_userPreferredSoftBrightnessIndex;
}

Bool DomainDisplayControl_001::isUserPreferredIndexModified(UIntN participantIndex, UIntN domainIndex)
{
	return m_isUserPreferredIndexModified;
}

UIntN DomainDisplayControl_001::getSoftBrightnessIndex(UIntN participantIndex, UIntN domainIndex)
{
	const Percentage brightnessPercentage = getParticipantServices()->primitiveExecuteGetAsPercentage(
		esif_primitive_type::GET_DISPLAY_BRIGHTNESS_SOFT, domainIndex);

	const auto currentIndex = getDisplayControlSet(participantIndex, domainIndex).getControlIndex(brightnessPercentage);

	return currentIndex;
}

DisplayControlSet DomainDisplayControl_001::getDisplayControlSet(UIntN participantIndex, UIntN domainIndex)
{
	if (m_displayControlSet.isInvalid())
	{
		m_displayControlSet.set(createDisplayControlSet(domainIndex));
	}
	return m_displayControlSet.get();
}

void DomainDisplayControl_001::setDisplayControl(UIntN participantIndex, UIntN domainIndex, UIntN displayControlIndex)
{
	const auto indexToSet = getAllowableDisplayBrightnessIndex(participantIndex, domainIndex, displayControlIndex);
	const auto displaySet = getDisplayControlSet(participantIndex, domainIndex);
	const Percentage newBrightness = displaySet[indexToSet].getBrightness();

	try
	{
		getParticipantServices()->primitiveExecuteSetAsPercentage(
			esif_primitive_type::SET_DISPLAY_BRIGHTNESS, newBrightness, domainIndex);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_WARNING(
			{ return "Failed to set display brightness."; });
	}

	m_lastSetDisplayBrightness = indexToSet;
}

void DomainDisplayControl_001::setSoftBrightness(UIntN participantIndex, UIntN domainIndex, UIntN displayControlIndex)
{
	const auto displaySet = getDisplayControlSet(participantIndex, domainIndex);
	const Percentage newBrightness = displaySet[displayControlIndex].getBrightness();

	try
	{
		getParticipantServices()->primitiveExecuteSetAsPercentage(
			esif_primitive_type::SET_DISPLAY_BRIGHTNESS_SOFT, newBrightness, domainIndex);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_WARNING({ return "Failed to set display soft brightness."; });
	}
}

void DomainDisplayControl_001::updateUserPreferredSoftBrightnessIndex(UIntN participantIndex, UIntN domainIndex)
{
	m_userPreferredSoftBrightnessIndex = getSoftBrightnessIndex(participantIndex, domainIndex); 

	PARTICIPANT_LOG_MESSAGE_DEBUG({
		return "User preferred soft brightness index after update = "
			   + std::to_string(m_userPreferredSoftBrightnessIndex)
			   + " .";
	});
}

void DomainDisplayControl_001::restoreUserPreferredSoftBrightness(UIntN participantIndex, UIntN domainIndex)
{
	if (m_userPreferredSoftBrightnessIndex != Constants::Invalid)
	{
		setSoftBrightness(participantIndex, domainIndex, m_userPreferredSoftBrightnessIndex);

		PARTICIPANT_LOG_MESSAGE_DEBUG({
			return "Display soft brightness has been reset to = " + std::to_string(m_userPreferredSoftBrightnessIndex)
				   + " .";
		});

		m_userPreferredSoftBrightnessIndex = Constants::Invalid;
	}
	else
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "No user preference to restore."; });
	}
}

UIntN DomainDisplayControl_001::getAllowableDisplayBrightnessIndex(
	UIntN participantIndex,
	UIntN domainIndex,
	UIntN requestedIndex)
{
	const auto userPreferred = getUserPreferredDisplayIndex(participantIndex, domainIndex);
	const auto dynamicCapabilities = getDisplayControlDynamicCaps(participantIndex, domainIndex);
	const auto maxLimit = std::max(dynamicCapabilities.getCurrentUpperLimit(), userPreferred);

	auto valueToRequest = std::max(requestedIndex, maxLimit);

	const auto minIndex = dynamicCapabilities.getCurrentLowerLimit();
	if (userPreferred <= minIndex)
	{
		valueToRequest = std::min(valueToRequest, minIndex);
	}

	return valueToRequest;
}

void DomainDisplayControl_001::setDisplayControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex,
	DisplayControlDynamicCaps newCapabilities)
{
	const auto displaySet = getDisplayControlSet(participantIndex, domainIndex);
	const auto upperLimitIndex = newCapabilities.getCurrentUpperLimit();
	auto lowerLimitIndex = newCapabilities.getCurrentLowerLimit();

	if (upperLimitIndex != Constants::Invalid && lowerLimitIndex != Constants::Invalid)
	{
		const auto size = displaySet.getCount();
		if (upperLimitIndex >= size)
		{
			throw dptf_exception("Upper Limit index is out of control set bounds.");
		}
		else if (upperLimitIndex > lowerLimitIndex || lowerLimitIndex >= size)
		{
			lowerLimitIndex = size - 1;
			PARTICIPANT_LOG_MESSAGE_WARNING({
				return "Limit index mismatch, setting lower limit to lowest possible index.";
				});
		}

		m_displayControlDynamicCaps.invalidate();

		const Percentage upperLimitBrightness = displaySet[upperLimitIndex].getBrightness();
		const UInt32 uint32UpperLimit = upperLimitBrightness.toWholeNumber();
		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_DISPLAY_CAPABILITY,
			uint32UpperLimit,
			domainIndex,
			Constants::Esif::NoPersistInstance);

		const Percentage lowerLimitBrightness = displaySet[lowerLimitIndex].getBrightness();
		const UInt32 uint32LowerLimit = lowerLimitBrightness.toWholeNumber();
		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_DISPLAY_DEPTH_LIMIT,
			uint32LowerLimit,
			domainIndex,
			Constants::Esif::NoPersistInstance);
	}
	else
	{
		m_displayControlDynamicCaps.invalidate();

		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_DISPLAY_CAPABILITY,
			upperLimitIndex,
			domainIndex,
			Constants::Esif::NoPersistInstance);

		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_DISPLAY_DEPTH_LIMIT,
			lowerLimitIndex,
			domainIndex,
			Constants::Esif::NoPersistInstance);
	}
}

void DomainDisplayControl_001::setDisplayCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
	m_capabilitiesLocked = lock;
}

void DomainDisplayControl_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	if (isActivityLoggingEnabled() == true)
	{
		try
		{
			auto dynamicCaps = getDisplayControlDynamicCaps(participantIndex, domainIndex);
			auto displaySet = getDisplayControlSet(participantIndex, domainIndex);
			auto displayControlIndex = getDisplayControlStatus(participantIndex, domainIndex).getBrightnessLimitIndex();

			if (displayControlIndex == Constants::Invalid)
			{
				displayControlIndex = dynamicCaps.getCurrentUpperLimit();
			}

			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL;
			capability.size = sizeof(capability);
			capability.data.displayControl.currentDPTFLimit =
				displaySet[displayControlIndex].getBrightness().toWholeNumber();
			capability.data.displayControl.lowerLimit =
				displaySet[dynamicCaps.getCurrentLowerLimit()].getBrightness().toWholeNumber();
			capability.data.displayControl.upperLimit =
				displaySet[dynamicCaps.getCurrentUpperLimit()].getBrightness().toWholeNumber();

			getParticipantServices()->sendDptfEvent(
				ParticipantEvent::DptfParticipantControlAction,
				domainIndex,
				Capability::getEsifDataFromCapabilityData(&capability));

			PARTICIPANT_LOG_MESSAGE_INFO({
				std::stringstream message;
				message << "Published activity for participant " << getParticipantIndex() << ", "
						<< "domain " << getName() << " "
						<< "("
						<< "Display Control"
						<< ")";
				return message.str();
			});
		}
		catch(...)
		{
		}
	}
}

void DomainDisplayControl_001::onClearCachedData()
{
	m_displayControlDynamicCaps.invalidate();
	m_displayControlSet.invalidate();

	if (m_capabilitiesLocked == false)
	{
		try
		{
			const DptfBuffer capabilityBuffer = createResetPrimitiveTupleBinary(
				esif_primitive_type::SET_DISPLAY_CAPABILITY, Constants::Esif::NoPersistInstance);
			getParticipantServices()->primitiveExecuteSet(
				esif_primitive_type::SET_CONFIG_RESET,
				ESIF_DATA_BINARY,
				capabilityBuffer.get(),
				capabilityBuffer.size(),
				capabilityBuffer.size(),
				0,
				Constants::Esif::NoInstance);

			const DptfBuffer depthLimitBuffer = createResetPrimitiveTupleBinary(
				esif_primitive_type::SET_DISPLAY_DEPTH_LIMIT, Constants::Esif::NoPersistInstance);
			getParticipantServices()->primitiveExecuteSet(
				esif_primitive_type::SET_CONFIG_RESET,
				ESIF_DATA_BINARY,
				depthLimitBuffer.get(),
				depthLimitBuffer.size(),
				depthLimitBuffer.size(),
				0,
				Constants::Esif::NoInstance);
		}
		catch (...)
		{
			// best effort
			PARTICIPANT_LOG_MESSAGE_DEBUG({
				return "Failed to restore the initial display control capabilities. ";
				});
		}
	}
}

std::shared_ptr<XmlNode> DomainDisplayControl_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("display_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(getDisplayControlStatus(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getDisplayControlDynamicCaps(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getDisplayControlSet(getParticipantIndex(), domainIndex).getXml());
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

	return root;
}

void DomainDisplayControl_001::restore()
{
	try
	{
		if (m_userPreferredIndex != Constants::Invalid)
		{
			getParticipantServices()->setUserPreferredDisplayCacheValue(
				getParticipantIndex(), getDomainIndex(), m_userPreferredIndex);

			const auto displaySet = getDisplayControlSet(getParticipantIndex(), getDomainIndex());
			const auto upperLimitIndex =
				getDisplayControlDynamicCaps(getParticipantIndex(), getDomainIndex()).getCurrentUpperLimit();
			if (m_userPreferredIndex < upperLimitIndex)
			{
				m_userPreferredIndex = upperLimitIndex;
			}
			const Percentage newBrightness = displaySet[m_userPreferredIndex].getBrightness();

			PARTICIPANT_LOG_MESSAGE_DEBUG({
				std::stringstream message;
				message << "Saved the user preferred index of " + std::to_string(m_userPreferredIndex) + ". ";
				message << "Attempting to set the brightness to the user preferred value .";
				return message.str();
				});

			getParticipantServices()->primitiveExecuteSetAsPercentage(
				esif_primitive_type::SET_DISPLAY_BRIGHTNESS, newBrightness, getDomainIndex());
		}
	}
	catch (...)
	{
		// best effort
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to restore the user preferred display status. "; });
	}

	try
	{
		if (m_userPreferredSoftBrightnessIndex != Constants::Invalid)
		{
			// soft brightness cache and restore
			const auto displaySet = getDisplayControlSet(getParticipantIndex(), getDomainIndex());
			const auto indexToSet = getAllowableDisplayBrightnessIndex(
				getParticipantIndex(), getDomainIndex(), m_userPreferredSoftBrightnessIndex);
			const Percentage newSoftBrightness = displaySet[indexToSet].getBrightness();

			PARTICIPANT_LOG_MESSAGE_DEBUG({
				std::stringstream messageStr;
				messageStr << "Saved the user preferred soft brightness index of "
								  + std::to_string(m_userPreferredSoftBrightnessIndex) + ". ";
				messageStr << "Attempting to set the brightness to the user preferred soft brightness value .";
				return messageStr.str();
			});

			getParticipantServices()->primitiveExecuteSetAsPercentage(
				esif_primitive_type::SET_DISPLAY_BRIGHTNESS_SOFT, newSoftBrightness, getDomainIndex());
		}
	}
	catch (...)
	{
		// best effort
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to restore the user preferred soft brightness status. "; });
	}
}

DisplayControlDynamicCaps DomainDisplayControl_001::createDisplayControlDynamicCaps(UIntN domainIndex)
{
	const auto displaySet = getDisplayControlSet(getParticipantIndex(), domainIndex);

	// Get dynamic caps
	//  The caps are stored in BIOS as brightness percentage.  They must be converted
	//  to indices before they can be used.
	UIntN lowerLimitIndex = getLowerLimitIndex(domainIndex, displaySet);
	const UIntN upperLimitIndex = getUpperLimitIndex(domainIndex, displaySet);

	const auto size = displaySet.getCount();
	if (upperLimitIndex >= size)
	{
		throw dptf_exception("Upper Limit index is out of control set bounds.");
	}
	else if (upperLimitIndex > lowerLimitIndex || lowerLimitIndex >= size)
	{
		lowerLimitIndex = size - 1;

		PARTICIPANT_LOG_MESSAGE_WARNING({
			return "Limit index mismatch, ignoring lower limit.";
			});
	}

	return {upperLimitIndex, lowerLimitIndex};
}

DisplayControlSet DomainDisplayControl_001::createDisplayControlSet(UIntN domainIndex) const
{
	// _BCL Table
	const DptfBuffer buffer = getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_DISPLAY_BRIGHTNESS_LEVELS, ESIF_DATA_BINARY, domainIndex);
	auto displayControlSet = DisplayControlSet::createFromBcl(buffer);
	throwIfDisplaySetIsEmpty(displayControlSet.getCount());

	return displayControlSet;
}

void DomainDisplayControl_001::throwIfControlIndexIsOutOfRange(UIntN displayControlIndex, UIntN domainIndex)
{
	const auto dynamicCaps = getDisplayControlDynamicCaps(getParticipantIndex(), domainIndex);
	const auto upperLimit = dynamicCaps.getCurrentUpperLimit();
	const auto lowerLimit = dynamicCaps.getCurrentLowerLimit();
	const auto displaySet = getDisplayControlSet(getParticipantIndex(), domainIndex);
	const auto size = displaySet.getCount();

	if (displayControlIndex >= size || displayControlIndex < upperLimit || displayControlIndex > lowerLimit)
	{
		std::stringstream infoMessage;

		infoMessage << "Control index is outside the allowable range." << std::endl
			<< "Desired Index : " << displayControlIndex << std::endl
			<< "DisplayControlSet size :" << size << std::endl
			<< "Upper Limit : " << upperLimit << std::endl
			<< "Lower Limit : " << lowerLimit << std::endl;

		throw dptf_exception(infoMessage.str());
	}
}

void DomainDisplayControl_001::throwIfDisplaySetIsEmpty(UIntN sizeOfSet)
{
	if (sizeOfSet == 0)
	{
		throw dptf_exception("Display Brightness set is empty. Impossible if we support display controls.");
	}
}

UIntN DomainDisplayControl_001::getLowerLimitIndex(UIntN domainIndex, DisplayControlSet displaySet) const
{
	const UInt32 uint32val = getParticipantServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_DISPLAY_DEPTH_LIMIT, domainIndex);
	const Percentage lowerLimitBrightness = Percentage::fromWholeNumber(uint32val);
	return displaySet.getControlIndex(lowerLimitBrightness);
}

UIntN DomainDisplayControl_001::getUpperLimitIndex(UIntN domainIndex, DisplayControlSet displaySet) const
{
	UIntN upperLimitIndex;
	try
	{
		const UInt32 uint32val = getParticipantServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_DISPLAY_CAPABILITY, domainIndex);
		const Percentage upperLimitBrightness = Percentage::fromWholeNumber(uint32val);
		upperLimitIndex = displaySet.getControlIndex(upperLimitBrightness);
	}
	catch (...)
	{
		// DDPC is optional
		PARTICIPANT_LOG_MESSAGE_DEBUG({
			return "DDPC was not present.  Setting upper limit to 100.";
			});

		upperLimitIndex = 0; // Max brightness
	}
	return upperLimitIndex;
}

std::string DomainDisplayControl_001::getName()
{
	return "Display Control";
}
