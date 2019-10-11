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

#include "DomainConfigTdpControl_001.h"
#include "XmlNode.h"

DomainConfigTdpControl_001::DomainConfigTdpControl_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainConfigTdpControlBase(participantIndex, domainIndex, participantServicesInterface)
	, m_configTdpLevelsAvailable(0)
	, m_currentConfigTdpControlId(Constants::Invalid)
	, m_configTdpLock(false)
{
	onClearCachedData();
	capture();
}

DomainConfigTdpControl_001::~DomainConfigTdpControl_001(void)
{
	restore();
}

ConfigTdpControlDynamicCaps DomainConfigTdpControl_001::getConfigTdpControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex)
{
	if (m_configTdpControlDynamicCaps.isInvalid())
	{
		m_configTdpControlDynamicCaps.set(createConfigTdpControlDynamicCaps(domainIndex));
	}

	return m_configTdpControlDynamicCaps.get();
}

ConfigTdpControlStatus DomainConfigTdpControl_001::getConfigTdpControlStatus(UIntN participantIndex, UIntN domainIndex)
{
	if (m_configTdpControlStatus.isInvalid())
	{
		UIntN currentTdpControl = // ulTdpControl NOT index...
			getParticipantServices()->primitiveExecuteGetAsUInt32(
				esif_primitive_type::GET_PROC_CTDP_CURRENT_SETTING, domainIndex);

		// Determine the index...
		Bool controlIdFound = false;
		auto controlSet = getConfigTdpControlSet(participantIndex, domainIndex);
		for (UIntN i = 0; i < controlSet.getCount(); i++)
		{
			if (currentTdpControl == controlSet[i].getControlId())
			{
				m_configTdpControlStatus.set(ConfigTdpControlStatus(i));
				controlIdFound = true;
			}
		}

		if (controlIdFound == false)
		{
			throw dptf_exception("cTDP control not found in set.");
		}
	}

	return m_configTdpControlStatus.get();
}

ConfigTdpControlSet DomainConfigTdpControl_001::getConfigTdpControlSet(UIntN participantIndex, UIntN domainIndex)
{
	if (m_configTdpControlSet.isInvalid())
	{
		checkHWConfigTdpSupport(domainIndex);
		m_configTdpControlSet.set(createConfigTdpControlSet(domainIndex));

		// If any lock bit is set, we only provide 1 cTDP level to the policies.
		if (m_configTdpLock)
		{
			auto controlSet = m_configTdpControlSet.get();
			controlSet.removeLastControl();
			m_configTdpControlSet.set(controlSet);
		}
	}

	return m_configTdpControlSet.get();
}

void DomainConfigTdpControl_001::setConfigTdpControl(
	UIntN participantIndex,
	UIntN domainIndex,
	UIntN configTdpControlIndex)
{
	// If any of the lock bits are set, we cannot program cTDP
	if (m_configTdpLock)
	{
		PARTICIPANT_LOG_MESSAGE_WARNING({ return "cTDP set level ignored, lock bit is set!"; });

		return;
	}

	throwIfConfigTdpControlIndexIsOutOfBounds(participantIndex, domainIndex, configTdpControlIndex);

	auto controlSet = getConfigTdpControlSet(participantIndex, domainIndex);
	getParticipantServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_PROC_CTDP_CONTROL,
		controlSet[configTdpControlIndex].getControlId(), // This is what 7.x does
		domainIndex);

	getParticipantServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_PROC_TURBO_ACTIVATION_RATIO,
		controlSet[configTdpControlIndex].getTdpRatio() - 1, // This is what 7.x does
		domainIndex);

	// Then BIOS
	getParticipantServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_CTDP_POINT, configTdpControlIndex, domainIndex);

	// Refresh the status
	m_configTdpControlStatus.set(ConfigTdpControlStatus(configTdpControlIndex));
}

void DomainConfigTdpControl_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			UInt32 configTdpControlIndex =
				getConfigTdpControlStatus(participantIndex, domainIndex).getCurrentControlIndex();

			if (configTdpControlIndex == Constants::Invalid)
			{
				configTdpControlIndex =
					getConfigTdpControlDynamicCaps(participantIndex, domainIndex).getCurrentUpperLimitIndex();
			}

			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_CTDP_CONTROL;
			capability.size = sizeof(capability);
			capability.data.configTdpControl.controlId = configTdpControlIndex;
			auto controlSet = getConfigTdpControlSet(participantIndex, domainIndex);
			capability.data.configTdpControl.tdpFrequency = controlSet[configTdpControlIndex].getTdpFrequency();
			capability.data.configTdpControl.tdpPower = controlSet[configTdpControlIndex].getTdpPower();
			capability.data.configTdpControl.tdpRatio = controlSet[configTdpControlIndex].getTdpRatio();

			getParticipantServices()->sendDptfEvent(
				ParticipantEvent::DptfParticipantControlAction,
				domainIndex,
				Capability::getEsifDataFromCapabilityData(&capability));

			PARTICIPANT_LOG_MESSAGE_INFO({
				std::stringstream message;
				message << "Published activity for participant " << getParticipantIndex() << ", "
						<< "domain " << getName() << " "
						<< "("
						<< "Config TDP Control"
						<< ")";
				return message.str();
			});
		}
	}
	catch (...)
	{
		// skip if there are any issue in sending log data
	}
}

void DomainConfigTdpControl_001::onClearCachedData(void)
{
	m_configTdpControlSet.invalidate();
	m_configTdpControlDynamicCaps.invalidate();
	m_configTdpControlStatus.invalidate();
}

std::shared_ptr<XmlNode> DomainConfigTdpControl_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("config_tdp_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(getConfigTdpControlDynamicCaps(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getConfigTdpControlSet(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getConfigTdpControlStatus(getParticipantIndex(), domainIndex).getXml());
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

	return root;
}

void DomainConfigTdpControl_001::capture(void)
{
	try
	{
		m_initialStatus.set(getConfigTdpControlStatus(getParticipantIndex(), getDomainIndex()));
	}
	catch (dptf_exception& ex)
	{
		PARTICIPANT_LOG_MESSAGE_WARNING_EX({ return "Failed to get the initial cTDP status. " + ex.getDescription(); });
	}
}

void DomainConfigTdpControl_001::restore(void)
{
	if (m_initialStatus.isValid())
	{
		try
		{
			setConfigTdpControl(
				getParticipantIndex(), getDomainIndex(), m_initialStatus.get().getCurrentControlIndex());
		}
		catch (dptf_exception& ex)
		{
			PARTICIPANT_LOG_MESSAGE_WARNING_EX(
				{ return "Failed to restore the initial cTDP status. " + ex.getDescription(); });
		}
		catch (...)
		{
			// best effort
			PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to restore the initial cTDP control status. "; });
		}
	}
}

ConfigTdpControlDynamicCaps DomainConfigTdpControl_001::createConfigTdpControlDynamicCaps(UIntN domainIndex)
{
	auto controlSet = getConfigTdpControlSet(getParticipantIndex(), domainIndex);

	// Get dynamic caps
	UInt32 lowerLimitIndex = 0;
	UInt32 upperLimitIndex = 0;

	if (!m_configTdpLock)
	{
		lowerLimitIndex = controlSet.getCount() - 1; // This is what the 7.0 code does

		upperLimitIndex = getParticipantServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_PROC_CTDP_CAPABILITY, domainIndex);

		if (lowerLimitIndex < upperLimitIndex)
		{
			throw dptf_exception("Config TDP capabilities are incorrect.");
		}
	}

	return ConfigTdpControlDynamicCaps(lowerLimitIndex, upperLimitIndex);
}

ConfigTdpControlSet DomainConfigTdpControl_001::createConfigTdpControlSet(UIntN domainIndex)
{
	// Build TDPL table
	DptfBuffer buffer = getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_PROC_CTDP_POINT_LIST, ESIF_DATA_BINARY, domainIndex);
	return ConfigTdpControlSet(ConfigTdpControlSet::createFromTdpl(buffer));
}

void DomainConfigTdpControl_001::checkHWConfigTdpSupport(UIntN domainIndex)
{
	m_configTdpLevelsAvailable = getLevelCount(domainIndex);
	m_configTdpLock = isLockBitSet(domainIndex);
}

void DomainConfigTdpControl_001::throwIfConfigTdpControlIndexIsOutOfBounds(
	UIntN participantIndex,
	UIntN domainIndex,
	UIntN configTdpControlIndex)
{
	auto controlSet = getConfigTdpControlSet(participantIndex, domainIndex);
	if (configTdpControlIndex >= controlSet.getCount())
	{
		std::stringstream infoMessage;

		infoMessage << "Control index out of control set bounds." << std::endl
					<< "Desired Index : " << configTdpControlIndex << std::endl
					<< "ConfigTdpControlSet size :" << controlSet.getCount() << std::endl;

		throw dptf_exception(infoMessage.str());
	}

	auto dynamicCaps = getConfigTdpControlDynamicCaps(participantIndex, domainIndex);
	if (configTdpControlIndex < dynamicCaps.getCurrentUpperLimitIndex()
		|| configTdpControlIndex > dynamicCaps.getCurrentLowerLimitIndex())
	{
		std::stringstream infoMessage;

		infoMessage << "Got a config TDP control index that was outside the allowable range." << std::endl
					<< "Desired Index : " << configTdpControlIndex << std::endl
					<< "Upper Limit Index : " << dynamicCaps.getCurrentUpperLimitIndex() << std::endl
					<< "Lower Limit Index : " << dynamicCaps.getCurrentLowerLimitIndex() << std::endl;

		throw dptf_exception(infoMessage.str());
	}
}

UIntN DomainConfigTdpControl_001::getLevelCount(UIntN domainIndex)
{
	UInt32 cTDPSupport = getParticipantServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_PROC_CTDP_SUPPORT_CHECK, domainIndex);

	if (cTDPSupport == 0 || cTDPSupport == 3)
	{
		// cTDP is not supported in HW!
		// Because of this Custom cTDP is not supported as well
		//   PLATFORM_INFO Register(Read Only)
		//   bit 34:33 - Number of configurable TDP levels - default : 00b
		//     00 - Config TDP not supported
		//     01 - One additional TDP level supported
		//     10 - Two additional TDP levels supported
		//     11 - Reserved
		throw dptf_exception("ConfigTdp not supported.");
	}
	else
	{
		return cTDPSupport + 1; // Add 1 to account for the nominal level
	}
}

Bool DomainConfigTdpControl_001::isLockBitSet(UIntN domainIndex)
{
	// Check to see if the TAR or if cTDP is locked
	Bool tarLock = (getParticipantServices()->primitiveExecuteGetAsUInt32(
						esif_primitive_type::GET_PROC_CTDP_TAR_LOCK_STATUS, domainIndex)
					== 1)
					   ? true
					   : false;

	Bool configTdpLock = (getParticipantServices()->primitiveExecuteGetAsUInt32(
							  esif_primitive_type::GET_PROC_CTDP_LOCK_STATUS, domainIndex)
						  == 1)
							 ? true
							 : false;

	if (tarLock || configTdpLock)
	{
		PARTICIPANT_LOG_MESSAGE_WARNING({ return "cTDP is supported, but the lock bit is set!"; });
		return true;
	}
	else
	{
		return false;
	}
}

std::string DomainConfigTdpControl_001::getName(void)
{
	return "Config TDP Control";
}
