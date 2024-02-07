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

#include "DomainProxy.h"
#include "StatusFormat.h"
using namespace std;

DomainProxy::DomainProxy(
	UIntN domainIndex,
	ParticipantProxyInterface* participant,
	const PolicyServicesInterfaceContainer& policyServices)
	: m_participantIndex(participant->getIndex())
	, m_domainIndex(domainIndex)
	, m_participant(participant)
	, m_domainPriorityProperty(
		  Constants::Invalid,
		  Constants::Invalid,
		  DomainProperties(Guid(), Constants::Invalid, false, DomainType::Other, "", "", DomainFunctionalityVersions()),
		  PolicyServicesInterfaceContainer())
	, m_domainProperties(Guid(), Constants::Invalid, false, DomainType::Other, "", "", DomainFunctionalityVersions())
	, m_participantProperties(Guid(), "", "", BusType::None, PciInfo(), AcpiInfo())
	, m_policyServices(policyServices)
{
	m_domainProperties = DomainProperties(participant->getDomainPropertiesSet().getDomainProperties(domainIndex));
	m_participantProperties = ParticipantProperties(participant->getParticipantProperties());
	m_domainPriorityProperty =
		DomainPriorityCachedProperty(m_participantIndex, domainIndex, m_domainProperties, policyServices);

	// create control facades
	m_temperatureControl =
		make_shared<TemperatureControlFacade>(m_participantIndex, domainIndex, m_domainProperties, policyServices);
	m_performanceControl =
		make_shared<PerformanceControlFacade>(m_participantIndex, domainIndex, m_domainProperties, policyServices);
	m_powerControl =
		make_shared<PowerControlFacade>(m_participantIndex, domainIndex, m_domainProperties, policyServices);
	m_systemPowerControl =
		make_shared<SystemPowerControlFacade>(m_participantIndex, domainIndex, m_domainProperties, policyServices);
	m_displayControl =
		make_shared<DisplayControlFacade>(m_participantIndex, domainIndex, m_domainProperties, policyServices);
	m_coreControl =
		make_shared<CoreControlFacade>(m_participantIndex, domainIndex, m_domainProperties, policyServices);
	m_radioFrequencyControl = make_shared<RadioFrequencyControlFacade>(
		m_participantIndex, domainIndex, m_domainProperties, policyServices);
	m_activeCoolingControl = make_shared<ActiveCoolingControl>(
		m_participantIndex, domainIndex, m_domainProperties, m_participantProperties, policyServices);
	m_peakPowerControl =
		make_shared<PeakPowerControlFacade>(m_participantIndex, domainIndex, m_domainProperties, policyServices);
	m_processorControl =
		make_shared<ProcessorControlFacade>(m_participantIndex, domainIndex, m_domainProperties, policyServices);
	m_platformPowerStatus = make_shared<PlatformPowerStatusFacade>(
		m_participantIndex, domainIndex, m_domainProperties, policyServices);
	m_batteryStatus =
		make_shared<BatteryStatusFacade>(m_participantIndex, domainIndex, m_domainProperties, policyServices);
	m_socWorkloadClassification = make_shared<SocWorkloadClassificationFacade>(
		m_participantIndex, domainIndex, m_domainProperties, policyServices);
	m_dynamicEpp = make_shared<DynamicEppFacade>(
		m_participantIndex, domainIndex, m_domainProperties, policyServices);
	m_biasControl = make_shared<BiasControlFacade>(
		m_participantIndex, domainIndex, m_domainProperties, policyServices);
	m_energyControl = make_shared<EnergyControlFacade>(
		m_participantIndex, domainIndex, m_domainProperties, policyServices);
}

DomainProxy::DomainProxy()
	: m_participantIndex(Constants::Invalid)
	, m_domainIndex(Constants::Invalid)
	, m_participant(nullptr)
	, m_domainPriorityProperty(
		  Constants::Invalid,
		  Constants::Invalid,
		  DomainProperties(Guid(), Constants::Invalid, false, DomainType::Other, "", "", DomainFunctionalityVersions()),
		  PolicyServicesInterfaceContainer())
	, m_domainProperties(Guid(), Constants::Invalid, false, DomainType::Other, "", "", DomainFunctionalityVersions())
	, m_participantProperties(Guid(), "", "", BusType::None, PciInfo(), AcpiInfo())
{
}

DomainProxy::~DomainProxy()
{
}

UIntN DomainProxy::getParticipantIndex() const
{
	return m_participantIndex;
}

UIntN DomainProxy::getDomainIndex() const
{
	return m_domainIndex;
}

const DomainProperties& DomainProxy::getDomainProperties() const
{
	return m_domainProperties;
}

const ParticipantProperties& DomainProxy::getParticipantProperties() const
{
	return m_participantProperties;
}

shared_ptr<TemperatureControlFacadeInterface> DomainProxy::getTemperatureControl()
{
	return m_temperatureControl;
}

DomainPriorityCachedProperty& DomainProxy::getDomainPriorityProperty()
{
	return m_domainPriorityProperty;
}

shared_ptr<PerformanceControlFacadeInterface> DomainProxy::getPerformanceControl()
{
	return m_performanceControl;
}

shared_ptr<PowerControlFacadeInterface> DomainProxy::getPowerControl()
{
	return m_powerControl;
}

shared_ptr<SystemPowerControlFacadeInterface> DomainProxy::getSystemPowerControl()
{
	return m_systemPowerControl;
}

shared_ptr<DisplayControlFacadeInterface> DomainProxy::getDisplayControl()
{
	return m_displayControl;
}

shared_ptr<CoreControlFacadeInterface> DomainProxy::getCoreControl()
{
	return m_coreControl;
}

RadioFrequencyControlFacade& DomainProxy::getRadioFrequencyControl() const
{
	return *m_radioFrequencyControl;
}

shared_ptr<ActiveCoolingControlFacadeInterface> DomainProxy::getActiveCoolingControl()
{
	return m_activeCoolingControl;
}

shared_ptr<PeakPowerControlFacadeInterface> DomainProxy::getPeakPowerControl()
{
	return m_peakPowerControl;
}

shared_ptr<ProcessorControlFacadeInterface> DomainProxy::getProcessorControl()
{
	return m_processorControl;
}

shared_ptr<PlatformPowerStatusFacadeInterface> DomainProxy::getPlatformPowerStatus()
{
	return m_platformPowerStatus;
}

shared_ptr<BatteryStatusFacadeInterface> DomainProxy::getBatteryStatus()
{
	return m_batteryStatus;
}

shared_ptr<SocWorkloadClassificationFacadeInterface> DomainProxy::getSocWorkloadClassification()
{
	return m_socWorkloadClassification;
}

shared_ptr<DynamicEppFacadeInterface> DomainProxy::getDynamicEpp()
{
	return m_dynamicEpp;
}

shared_ptr<BiasControlFacadeInterface> DomainProxy::getBiasControl()
{
	return m_biasControl;
}

shared_ptr<EnergyControlFacadeInterface> DomainProxy::getEnergyControl()
{
	return m_energyControl;
}

UtilizationStatus DomainProxy::getUtilizationStatus()
{
	return m_policyServices.domainUtilization->getUtilizationStatus(m_participantIndex, m_domainIndex);
}

void DomainProxy::clearTemperatureThresholds()
{
	try
	{
		if (m_temperatureControl->supportsTemperatureControls())
		{
			m_temperatureControl->setTemperatureNotificationThresholds(
				Temperature::createInvalid(), Temperature::createInvalid());
		}
	}
	catch (...)
	{
	}
}

void DomainProxy::initializeControls()
{
	try
	{
		m_performanceControl->initializeControlsIfNeeded();
	}
	catch (...)
	{
	}

	try
	{
		m_powerControl->initializeControlsIfNeeded();
	}
	catch (...)
	{
	}

	try
	{
		m_coreControl->initializeControlsIfNeeded();
	}
	catch (...)
	{
	}
}

void DomainProxy::setControlsToMax()
{
	try
	{
		m_performanceControl->setControlsToMax();
	}
	catch (...)
	{
	}

	try
	{
		m_powerControl->setControlsToMax();
	}
	catch (...)
	{
	}

	try
	{
		m_coreControl->setControlsToMax();
	}
	catch (...)
	{
	}

	try
	{
		m_displayControl->setControlsToMax();
	}
	catch (...)
	{
	}
}

shared_ptr<XmlNode> DomainProxy::getXml() const
{
	auto wrapper = XmlNode::createWrapperElement("domain");
	wrapper->addChild(XmlNode::createDataElement("participant_index", StatusFormat::friendlyValue(m_participantIndex)));
	wrapper->addChild(XmlNode::createDataElement("domain_index", StatusFormat::friendlyValue(m_domainIndex)));
	wrapper->addChild(m_domainProperties.getXml());
	wrapper->addChild(m_participantProperties.getXml());
	return wrapper;
}
