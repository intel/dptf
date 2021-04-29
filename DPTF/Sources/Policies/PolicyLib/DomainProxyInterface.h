/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#include "TemperatureControlFacadeInterface.h"
#include "PerformanceControlFacadeInterface.h"
#include "PowerControlFacadeInterface.h"
#include "DisplayControlFacadeInterface.h"
#include "CoreControlFacadeInterface.h"
#include "RadioFrequencyControlFacade.h"
#include "SystemPowerControlFacadeInterface.h"
#include "ActiveCoolingControlFacadeInterface.h"
#include "PeakPowerControlFacadeInterface.h"
#include "ProcessorControlFacadeInterface.h"
#include "PlatformPowerStatusFacadeInterface.h"
#include "BatteryStatusFacadeInterface.h"
#include "SocWorkloadClassificationFacadeInterface.h"
#include "DynamicEppFacadeInterface.h"

#include "PowerControlKnob.h"
#include "DisplayControlKnob.h"
#include "CoreControlKnob.h"
#include "PerformanceControlKnob.h"
#include "XmlNode.h"

// represents a domain inside a participant.  holds cached records of all properties and potential controls for the
// domain.
class dptf_export DomainProxyInterface
{
public:
	virtual ~DomainProxyInterface(){};

	virtual UIntN getParticipantIndex() const = 0;
	virtual UIntN getDomainIndex() const = 0;

	// properties
	virtual const DomainProperties& getDomainProperties() const = 0;
	virtual const ParticipantProperties& getParticipantProperties() const = 0;
	virtual DomainPriorityCachedProperty& getDomainPriorityProperty() = 0;
	virtual UtilizationStatus getUtilizationStatus() = 0;

	// temperature
	virtual void clearTemperatureThresholds() = 0;

	// control facades
	virtual void initializeControls() = 0;
	virtual void setControlsToMax() = 0;
	virtual std::shared_ptr<TemperatureControlFacadeInterface> getTemperatureControl() = 0;
	virtual std::shared_ptr<ActiveCoolingControlFacadeInterface> getActiveCoolingControl() = 0;
	virtual std::shared_ptr<PerformanceControlFacadeInterface> getPerformanceControl() = 0;
	virtual std::shared_ptr<PowerControlFacadeInterface> getPowerControl() = 0;
	virtual std::shared_ptr<SystemPowerControlFacadeInterface> getSystemPowerControl() = 0;
	virtual std::shared_ptr<DisplayControlFacadeInterface> getDisplayControl() = 0;
	virtual std::shared_ptr<CoreControlFacadeInterface> getCoreControl() = 0;
	virtual RadioFrequencyControlFacade& getRadioFrequencyControl() const = 0;
	virtual std::shared_ptr<PeakPowerControlFacadeInterface> getPeakPowerControl() = 0;
	virtual std::shared_ptr<ProcessorControlFacadeInterface> getProcessorControl() = 0;
	virtual std::shared_ptr<PlatformPowerStatusFacadeInterface> getPlatformPowerStatus() = 0;
	virtual std::shared_ptr<BatteryStatusFacadeInterface> getBatteryStatus() = 0;
	virtual std::shared_ptr<SocWorkloadClassificationFacadeInterface> getSocWorkloadClassification() = 0;
	virtual std::shared_ptr<DynamicEppFacadeInterface> getDynamicEpp() = 0;

	virtual std::shared_ptr<XmlNode> getXml() const = 0;
};
