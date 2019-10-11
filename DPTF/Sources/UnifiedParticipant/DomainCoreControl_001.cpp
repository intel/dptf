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

#include "DomainCoreControl_001.h"
#include "XmlNode.h"

DomainCoreControl_001::DomainCoreControl_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainCoreControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	onClearCachedData();
	capture();
}

DomainCoreControl_001::~DomainCoreControl_001(void)
{
	restore();
}

CoreControlStaticCaps DomainCoreControl_001::getCoreControlStaticCaps(UIntN participantIndex, UIntN domainIndex)
{
	if (m_coreControlStaticCaps.isInvalid())
	{
		m_coreControlStaticCaps.set(createCoreControlStaticCaps(domainIndex));
	}

	return m_coreControlStaticCaps.get();
}

CoreControlDynamicCaps DomainCoreControl_001::getCoreControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
	if (m_coreControlDynamicCaps.isInvalid())
	{
		m_coreControlDynamicCaps.set(createCoreControlDynamicCaps(domainIndex));
	}

	return m_coreControlDynamicCaps.get();
}

CoreControlLpoPreference DomainCoreControl_001::getCoreControlLpoPreference(UIntN participantIndex, UIntN domainIndex)
{
	if (m_coreControlLpoPreference.isInvalid())
	{
		m_coreControlLpoPreference.set(createCoreControlLpoPreference(domainIndex));
	}

	return m_coreControlLpoPreference.get();
}

CoreControlStatus DomainCoreControl_001::getCoreControlStatus(UIntN participantIndex, UIntN domainIndex)
{
	if (m_coreControlStatus.isInvalid())
	{
		m_coreControlStatus.set(CoreControlStatus(Constants::Invalid));
	}

	return m_coreControlStatus.get();
}

void DomainCoreControl_001::setActiveCoreControl(
	UIntN participantIndex,
	UIntN domainIndex,
	const CoreControlStatus& coreControlStatus)
{
	verifyCoreControlStatus(domainIndex, coreControlStatus);
	UIntN totalCores = getCoreControlStaticCaps(participantIndex, domainIndex).getTotalLogicalProcessors();
	UIntN totalOfflineCoreRequest = totalCores - coreControlStatus.getNumActiveLogicalProcessors();

	getParticipantServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_PROC_NUMBER_OFFLINE_CORES, totalOfflineCoreRequest, domainIndex);

	// Refresh the status
	m_coreControlStatus.set(CoreControlStatus(coreControlStatus.getNumActiveLogicalProcessors()));
}

void DomainCoreControl_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			createCoreControlStaticCaps(domainIndex);
			UInt32 activeCores = getCoreControlStatus(participantIndex, domainIndex).getNumActiveLogicalProcessors();

			if (activeCores == Constants::Invalid)
			{
				activeCores = getCoreControlStaticCaps(participantIndex, domainIndex).getTotalLogicalProcessors();
			}

			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_CORE_CONTROL;
			capability.size = sizeof(capability);
			capability.data.coreControl.activeLogicalProcessors = activeCores;
			capability.data.coreControl.minimumActiveCores = 1;
			capability.data.coreControl.maximumActiveCores =
				getCoreControlStaticCaps(participantIndex, domainIndex).getTotalLogicalProcessors();

			getParticipantServices()->sendDptfEvent(
				ParticipantEvent::DptfParticipantControlAction,
				domainIndex,
				Capability::getEsifDataFromCapabilityData(&capability));

			PARTICIPANT_LOG_MESSAGE_INFO({
				std::stringstream message;
				message << "Published activity for participant " << getParticipantIndex() << ", "
						<< "domain " << getName() << " "
						<< "("
						<< "Core Control"
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

void DomainCoreControl_001::onClearCachedData(void)
{
	// Do not delete m_coreControlStatus.  We can't read this from ESIF.  We store it whenever it is 'set' and return
	// the value when requested.

	m_coreControlStaticCaps.invalidate();
	m_coreControlDynamicCaps.invalidate();
	m_coreControlLpoPreference.invalidate();
}

std::shared_ptr<XmlNode> DomainCoreControl_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("core_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(getCoreControlStatus(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getCoreControlStaticCaps(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getCoreControlDynamicCaps(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getCoreControlLpoPreference(getParticipantIndex(), domainIndex).getXml());
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

	return root;
}

void DomainCoreControl_001::restore(void)
{
	try
	{
		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_PROC_NUMBER_OFFLINE_CORES,
			0, // No cores are parked
			getDomainIndex());
	}
	catch (...)
	{
		// best effort
		PARTICIPANT_LOG_MESSAGE_WARNING({
			return "Failed to restore the initial core status. ";
			});
	}
}

CoreControlStaticCaps DomainCoreControl_001::createCoreControlStaticCaps(UIntN domainIndex)
{
	UInt32 logicalCoreCount = getParticipantServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_PROC_LOGICAL_PROCESSOR_COUNT, domainIndex);

	return CoreControlStaticCaps(logicalCoreCount);
}

CoreControlDynamicCaps DomainCoreControl_001::createCoreControlDynamicCaps(UIntN domainIndex)
{
	UInt32 minActiveCoreLimit = 1;
	UInt32 maxActiveCoreLimit =
		getCoreControlStaticCaps(getParticipantIndex(), domainIndex).getTotalLogicalProcessors();
	return CoreControlDynamicCaps(minActiveCoreLimit, maxActiveCoreLimit);
}

CoreControlLpoPreference DomainCoreControl_001::createCoreControlLpoPreference(UIntN domainIndex)
{
	Bool useDefault = false;
	DptfBuffer buffer;
	try
	{
		buffer = getParticipantServices()->primitiveExecuteGet(
			esif_primitive_type::GET_PROC_CURRENT_LOGICAL_PROCESSOR_OFFLINING, ESIF_DATA_BINARY, domainIndex);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_WARNING({
			return "CLPO not found.  Using defaults.";
			});
		useDefault = true;
	}

	if (useDefault == false)
	{
		try
		{
			return CoreControlLpoPreference::createFromClpo(buffer);
		}
		catch (...)
		{
			PARTICIPANT_LOG_MESSAGE_WARNING({
				return "Could not parse CLPO data.  Using defaults.";
				});
			useDefault = true;
		}
	}

	return CoreControlLpoPreference(
		true, 0, Percentage(.50), CoreControlOffliningMode::Smt, CoreControlOffliningMode::Core);
}

void DomainCoreControl_001::verifyCoreControlStatus(UIntN domainIndex, const CoreControlStatus& coreControlStatus)
{
	auto caps = getCoreControlDynamicCaps(getParticipantIndex(), domainIndex);
	if (coreControlStatus.getNumActiveLogicalProcessors() > caps.getMaxActiveCores()
		|| coreControlStatus.getNumActiveLogicalProcessors() < caps.getMinActiveCores())
	{
		throw dptf_exception("Desired number of cores outside dynamic caps range.");
	}
}

std::string DomainCoreControl_001::getName(void)
{
	return "Core Control";
}
