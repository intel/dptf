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

#pragma once

#include "Dptf.h"

#include "PolicyServicesInterfaceContainer.h"
#include "DomainProperties.h"
#include "ActiveCoolingControl.h"
#include "DomainPriorityCachedProperty.h"

#include "TemperatureControlFacade.h"
#include "PerformanceControlFacade.h"
#include "PowerControlFacade.h"
#include "DisplayControlFacade.h"
#include "CoreControlFacade.h"
#include "RadioFrequencyControlFacade.h"
#include "PeakPowerControlFacade.h"
#include "ProcessorControlFacade.h"
#include "SystemPowerControlFacade.h"
#include "PlatformPowerStatusFacade.h"
#include "BatteryStatusFacade.h"
#include "SocWorkloadClassificationFacade.h"
#include "DynamicEppFacade.h"
#include "BiasControlFacade.h"
#include "EnergyControlFacade.h"

#include "PowerControlKnob.h"
#include "DisplayControlKnob.h"
#include "CoreControlKnob.h"
#include "PerformanceControlKnob.h"
#include "DomainProxyInterface.h"
#include "ParticipantProxyInterface.h"

// represents a domain inside a participant.  holds cached records of all properties and potential controls for the
// domain.
class dptf_export DomainProxy : public DomainProxyInterface
{
public:
	DomainProxy();
	DomainProxy(
		UIntN domainIndex,
		ParticipantProxyInterface* participant,
		const PolicyServicesInterfaceContainer& policyServices);
	virtual ~DomainProxy();

	// domain properties
	UIntN getParticipantIndex() const override;
	UIntN getDomainIndex() const override;
	const DomainProperties& getDomainProperties() const override;
	const ParticipantProperties& getParticipantProperties() const override;
	DomainPriorityCachedProperty& getDomainPriorityProperty() override;
	UtilizationStatus getUtilizationStatus() override;

	// domain actions
	void clearTemperatureThresholds() override;

	// control facades
	void initializeControls() override;
	void setControlsToMax() override;
	std::shared_ptr<TemperatureControlFacadeInterface> getTemperatureControl() override;
	std::shared_ptr<ActiveCoolingControlFacadeInterface> getActiveCoolingControl() override;
	std::shared_ptr<PerformanceControlFacadeInterface> getPerformanceControl() override;
	std::shared_ptr<PowerControlFacadeInterface> getPowerControl() override;
	std::shared_ptr<SystemPowerControlFacadeInterface> getSystemPowerControl() override;
	std::shared_ptr<DisplayControlFacadeInterface> getDisplayControl() override;
	std::shared_ptr<CoreControlFacadeInterface> getCoreControl() override;
	RadioFrequencyControlFacade& getRadioFrequencyControl() const override;
	std::shared_ptr<PeakPowerControlFacadeInterface> getPeakPowerControl() override;
	std::shared_ptr<ProcessorControlFacadeInterface> getProcessorControl() override;
	std::shared_ptr<PlatformPowerStatusFacadeInterface> getPlatformPowerStatus() override;
	std::shared_ptr<BatteryStatusFacadeInterface> getBatteryStatus() override;
	std::shared_ptr<SocWorkloadClassificationFacadeInterface> getSocWorkloadClassification() override;
	std::shared_ptr<DynamicEppFacadeInterface> getDynamicEpp() override;
	std::shared_ptr<BiasControlFacadeInterface> getBiasControl() override;
	std::shared_ptr<EnergyControlFacadeInterface> getEnergyControl() override;

	// status
	virtual std::shared_ptr<XmlNode> getXml() const override;

protected:
	// domain properties
	UIntN m_participantIndex;
	UIntN m_domainIndex;
	ParticipantProxyInterface* m_participant;
	DomainPriorityCachedProperty m_domainPriorityProperty;
	DomainProperties m_domainProperties;
	ParticipantProperties m_participantProperties;

	// control facades
	std::shared_ptr<ActiveCoolingControlFacadeInterface> m_activeCoolingControl;
	std::shared_ptr<TemperatureControlFacadeInterface> m_temperatureControl;
	std::shared_ptr<PerformanceControlFacade> m_performanceControl;
	std::shared_ptr<PowerControlFacade> m_powerControl;
	std::shared_ptr<SystemPowerControlFacade> m_systemPowerControl;
	std::shared_ptr<DisplayControlFacadeInterface> m_displayControl;
	std::shared_ptr<CoreControlFacadeInterface> m_coreControl;
	std::shared_ptr<RadioFrequencyControlFacade> m_radioFrequencyControl;
	std::shared_ptr<PeakPowerControlFacade> m_peakPowerControl;
	std::shared_ptr<ProcessorControlFacade> m_processorControl;
	std::shared_ptr<PlatformPowerStatusFacade> m_platformPowerStatus;
	std::shared_ptr<BatteryStatusFacade> m_batteryStatus;
	std::shared_ptr<SocWorkloadClassificationFacade> m_socWorkloadClassification;
	std::shared_ptr<DynamicEppFacade> m_dynamicEpp;
	std::shared_ptr<BiasControlFacade> m_biasControl;
	std::shared_ptr<EnergyControlFacade> m_energyControl;

	// services
	PolicyServicesInterfaceContainer m_policyServices;
};
