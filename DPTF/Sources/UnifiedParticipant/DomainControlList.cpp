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

#include "DomainControlList.h"
#include "StatusFormat.h"
using namespace std;

DomainControlList::DomainControlList(
	UIntN participantIndex,
	UIntN domainIndex,
	DomainFunctionalityVersions domainFunctionalityVersions,
	const ControlFactoryList& controlFactoryList,
	std::shared_ptr<ParticipantServicesInterface> participantServices)
	: m_participantIndex(participantIndex)
	, m_domainIndex(domainIndex)
	, m_domainFunctionalityVersions(domainFunctionalityVersions)
	, m_controlFactoryList(controlFactoryList)
	, m_participantServices(participantServices)
	, m_controlList()
{
	makeAllControls();
}

DomainControlList::~DomainControlList(void)
{
}

void DomainControlList::makeAllControls()
{
	// if an error is thrown we don't want to catch it as the domain can't be created anyway.
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::Active,
		makeControl<DomainActiveControlBase>(
			ControlFactoryType::Active, m_domainFunctionalityVersions.activeControlVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::Core,
		makeControl<DomainCoreControlBase>(
			ControlFactoryType::Core, m_domainFunctionalityVersions.coreControlVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::Display,
		makeControl<DomainDisplayControlBase>(
			ControlFactoryType::Display, m_domainFunctionalityVersions.displayControlVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::EnergyControl,
		makeControl<DomainEnergyControlBase>(
			ControlFactoryType::EnergyControl, m_domainFunctionalityVersions.energyControlVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::PeakPowerControl,
		makeControl<DomainPeakPowerControlBase>(
			ControlFactoryType::PeakPowerControl, m_domainFunctionalityVersions.peakPowerControlVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::Performance,
		makeControl<DomainPerformanceControlBase>(
			ControlFactoryType::Performance, m_domainFunctionalityVersions.performanceControlVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::PowerControl,
		makeControl<DomainPowerControlBase>(
			ControlFactoryType::PowerControl, m_domainFunctionalityVersions.powerControlVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::PowerStatus,
		makeControl<DomainPowerStatusBase>(
			ControlFactoryType::PowerStatus, m_domainFunctionalityVersions.powerStatusVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::Priority,
		makeControl<DomainPriorityBase>(
			ControlFactoryType::Priority, m_domainFunctionalityVersions.domainPriorityVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::RfProfileControl,
		makeControl<DomainRfProfileControlBase>(
			ControlFactoryType::RfProfileControl, m_domainFunctionalityVersions.rfProfileControlVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::RfProfileStatus,
		makeControl<DomainRfProfileStatusBase>(
			ControlFactoryType::RfProfileStatus, m_domainFunctionalityVersions.rfProfileStatusVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::Temperature,
		makeControl<DomainTemperatureBase>(
			ControlFactoryType::Temperature,
			m_domainFunctionalityVersions.temperatureVersion,
			m_domainFunctionalityVersions.temperatureThresholdVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::ProcessorControl,
		makeControl<DomainProcessorControlBase>(
			ControlFactoryType::ProcessorControl, m_domainFunctionalityVersions.processorControlVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::Utilization,
		makeControl<DomainUtilizationBase>(
			ControlFactoryType::Utilization, m_domainFunctionalityVersions.utilizationVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::SystemPower,
		makeControl<DomainSystemPowerControlBase>(
			ControlFactoryType::SystemPower, m_domainFunctionalityVersions.systemPowerControlVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::PlatformPowerStatus,
		makeControl<DomainPlatformPowerStatusBase>(
			ControlFactoryType::PlatformPowerStatus, m_domainFunctionalityVersions.platformPowerStatusVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::ActivityStatus,
		makeControl<DomainActivityStatusBase>(
			ControlFactoryType::ActivityStatus, m_domainFunctionalityVersions.activityStatusVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::BatteryStatus,
		makeControl<DomainBatteryStatusBase>(
			ControlFactoryType::BatteryStatus, m_domainFunctionalityVersions.batteryStatusVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::SocWorkloadClassification,
		makeControl<DomainSocWorkloadClassificationBase>(
			ControlFactoryType::SocWorkloadClassification,
			m_domainFunctionalityVersions.socWorkloadClassificationVersion)));
	m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
		ControlFactoryType::DynamicEpp,
		makeControl<DomainDynamicEppBase>(
			ControlFactoryType::DynamicEpp, m_domainFunctionalityVersions.dynamicEppVersion)));
}

template <typename T>
std::shared_ptr<T> DomainControlList::makeControl(ControlFactoryType::Type factoryType, UInt8& controlVersion)
{
	auto factory = m_controlFactoryList.getFactory(factoryType);
	std::shared_ptr<T> control(
		dynamic_cast<T*>(factory->make(m_participantIndex, m_domainIndex, controlVersion, m_participantServices)));
	return control;
}

template <typename T>
std::shared_ptr<T> DomainControlList::makeControl(
	ControlFactoryType::Type factoryType,
	UInt8& controlVersion,
	UInt8& associatedControlVersion)
{
	auto factory = m_controlFactoryList.getFactory(factoryType);
	std::shared_ptr<T> control(dynamic_cast<T*>(factory->make(
		m_participantIndex, m_domainIndex, controlVersion, associatedControlVersion, m_participantServices)));
	return control;
}

std::shared_ptr<XmlNode> DomainControlList::getXml()
{
	auto domain = XmlNode::createWrapperElement("domain_controls");
	for (auto control = m_controlList.begin(); control != m_controlList.end(); control++)
	{
		try
		{
			domain->addChild(control->second->getXml(m_domainIndex));
		}
		catch (not_implemented&)
		{
			// if not implemented, then eat the error.
		}
		catch (dptf_exception& ex)
		{
			PARTICIPANT_LOG_MESSAGE_WARNING_EX({
				return "Unable to get " + control->second->getName() + " control XML status: " + ex.getDescription();
			});
		}
		catch (...)
		{
			PARTICIPANT_LOG_MESSAGE_WARNING(
				{ return "Unable to get " + control->second->getName() + " control XML status."; });
		}
	}
	return domain;
}

std::shared_ptr<XmlNode> DomainControlList::getArbitratorStatusForPolicy(
	UIntN policyIndex,
	ControlFactoryType::Type type) const
{
	auto domain = XmlNode::createWrapperElement("domain_controls_arbitrator");
	for (auto control = m_controlList.begin(); control != m_controlList.end(); ++control)
	{
		try
		{
			if (type == control->first)
			{
				domain->addChild(control->second->getArbitratorXml(policyIndex));
			}
		}
		catch (not_implemented&)
		{
			// if not implemented, then eat the error.
		}
		catch (dptf_exception& ex)
		{
			PARTICIPANT_LOG_MESSAGE_WARNING_EX({
				return "Unable to get " + control->second->getName()
					   + " control arbitrator XML status: " + ex.getDescription();
			});
		}
		catch (...)
		{
			PARTICIPANT_LOG_MESSAGE_WARNING(
				{ return "Unable to get " + control->second->getName() + " control arbitrator XML status."; });
		}
	}
	return domain;
}

void DomainControlList::clearAllCachedData(void)
{
	for (auto control = m_controlList.begin(); control != m_controlList.end(); control++)
	{
		control->second->clearCachedData();
	}
}

void DomainControlList::clearAllCachedResults(void)
{
	for (auto control = m_controlList.begin(); control != m_controlList.end(); control++)
	{
		control->second->clearAllCachedResults();
	}
}

std::shared_ptr<DomainActiveControlBase> DomainControlList::getActiveControl(void)
{
	return dynamic_pointer_cast<DomainActiveControlBase>(m_controlList.at(ControlFactoryType::Active));
}

std::shared_ptr<DomainActivityStatusBase> DomainControlList::getActivityStatusControl(void)
{
	return dynamic_pointer_cast<DomainActivityStatusBase>(m_controlList.at(ControlFactoryType::ActivityStatus));
}

std::shared_ptr<DomainCoreControlBase> DomainControlList::getCoreControl(void)
{
	return dynamic_pointer_cast<DomainCoreControlBase>(m_controlList.at(ControlFactoryType::Core));
}

std::shared_ptr<DomainDisplayControlBase> DomainControlList::getDisplayControl(void)
{
	return dynamic_pointer_cast<DomainDisplayControlBase>(m_controlList.at(ControlFactoryType::Display));
}

std::shared_ptr<DomainEnergyControlBase> DomainControlList::getEnergyControl(void)
{
	return dynamic_pointer_cast<DomainEnergyControlBase>(m_controlList.at(ControlFactoryType::EnergyControl));
}

std::shared_ptr<DomainPeakPowerControlBase> DomainControlList::getPeakPowerControl(void)
{
	return dynamic_pointer_cast<DomainPeakPowerControlBase>(m_controlList.at(ControlFactoryType::PeakPowerControl));
}

std::shared_ptr<DomainPerformanceControlBase> DomainControlList::getPerformanceControl(void)
{
	return dynamic_pointer_cast<DomainPerformanceControlBase>(m_controlList.at(ControlFactoryType::Performance));
}

std::shared_ptr<DomainPowerControlBase> DomainControlList::getPowerControl(void)
{
	return dynamic_pointer_cast<DomainPowerControlBase>(m_controlList.at(ControlFactoryType::PowerControl));
}

std::shared_ptr<DomainPowerStatusBase> DomainControlList::getPowerStatusControl(void)
{
	return dynamic_pointer_cast<DomainPowerStatusBase>(m_controlList.at(ControlFactoryType::PowerStatus));
}

std::shared_ptr<DomainSystemPowerControlBase> DomainControlList::getSystemPowerControl(void)
{
	return dynamic_pointer_cast<DomainSystemPowerControlBase>(m_controlList.at(ControlFactoryType::SystemPower));
}

std::shared_ptr<DomainPlatformPowerStatusBase> DomainControlList::getPlatformPowerStatusControl(void)
{
	return dynamic_pointer_cast<DomainPlatformPowerStatusBase>(
		m_controlList.at(ControlFactoryType::PlatformPowerStatus));
}

std::shared_ptr<DomainPriorityBase> DomainControlList::getDomainPriorityControl(void)
{
	return dynamic_pointer_cast<DomainPriorityBase>(m_controlList.at(ControlFactoryType::Priority));
}

std::shared_ptr<DomainRfProfileControlBase> DomainControlList::getRfProfileControl(void)
{
	return dynamic_pointer_cast<DomainRfProfileControlBase>(m_controlList.at(ControlFactoryType::RfProfileControl));
}

std::shared_ptr<DomainRfProfileStatusBase> DomainControlList::getRfProfileStatusControl(void)
{
	return dynamic_pointer_cast<DomainRfProfileStatusBase>(m_controlList.at(ControlFactoryType::RfProfileStatus));
}

std::shared_ptr<DomainTemperatureBase> DomainControlList::getTemperatureControl(void)
{
	return dynamic_pointer_cast<DomainTemperatureBase>(m_controlList.at(ControlFactoryType::Temperature));
}

std::shared_ptr<DomainProcessorControlBase> DomainControlList::getProcessorControl(void)
{
	return dynamic_pointer_cast<DomainProcessorControlBase>(m_controlList.at(ControlFactoryType::ProcessorControl));
}

std::shared_ptr<DomainUtilizationBase> DomainControlList::getUtilizationControl(void)
{
	return dynamic_pointer_cast<DomainUtilizationBase>(m_controlList.at(ControlFactoryType::Utilization));
}

std::shared_ptr<DomainBatteryStatusBase> DomainControlList::getBatteryStatusControl(void)
{
	return dynamic_pointer_cast<DomainBatteryStatusBase>(m_controlList.at(ControlFactoryType::BatteryStatus));
}

std::shared_ptr<DomainSocWorkloadClassificationBase> DomainControlList::getSocWorkloadClassificationControl(void)
{
	return dynamic_pointer_cast<DomainSocWorkloadClassificationBase>(
		m_controlList.at(ControlFactoryType::SocWorkloadClassification));
}

std::shared_ptr<DomainDynamicEppBase> DomainControlList::getDynamicEppControl()
{
	return dynamic_pointer_cast<DomainDynamicEppBase>(m_controlList.at(ControlFactoryType::DynamicEpp));
}

std::shared_ptr<ParticipantServicesInterface> DomainControlList::getParticipantServices() const
{
	return m_participantServices;
}
