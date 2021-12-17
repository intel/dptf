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

#include "OsPowerSlider.h"
#include "OsPowerSchemePersonality.h"
#include "SystemMode.h"
#include "DptfManagerInterface.h"
#include "IgccBroadcastData.h"

class dptf_export SystemModeManagerInterface
{
public:
	virtual ~SystemModeManagerInterface(){};
	virtual void setPolicySystemModeValue(const SystemMode::Type systemMode) = 0;
	virtual void executeOperatingSystemPowerSliderChanged() = 0;
	virtual void executeOperatingSystemPowerSchemePersonalityChanged() = 0;
	virtual void executeOperatingSystemPowerSourceChanged(OsPowerSource::Type powerSource) = 0;
	virtual void registerFrameworkEvents() = 0;
	virtual void unregisterFrameworkEvents() = 0;
	virtual void arbitrateAndCreateEventSystemModeChanged()= 0;
};

class SystemModeManager : public SystemModeManagerInterface
{
public:
	SystemModeManager(DptfManagerInterface* dptfManager);
	~SystemModeManager(void);

	virtual void setPolicySystemModeValue(const SystemMode::Type systemMode) override;
	virtual void executeOperatingSystemPowerSliderChanged() override;
	virtual void executeOperatingSystemPowerSchemePersonalityChanged() override;
	virtual void executeOperatingSystemPowerSourceChanged(OsPowerSource::Type powerSource) override;
	virtual void registerFrameworkEvents() override;
	virtual void unregisterFrameworkEvents() override;
	virtual void arbitrateAndCreateEventSystemModeChanged() override;

private:
	DptfManagerInterface* m_dptfManager;
	SystemMode::Type m_policySystemModeValue;
	SystemMode::Type m_arbitratedValue;
	Bool m_arbitratedValueChangedSinceLastSet;
	std::bitset<PolicyEvent::Max> m_registeredEvents;
	OsPowerSource::Type m_powerSource;

	
	void arbitrateSystemMode(
		const OsPowerSlider::Type osPowerSlider,
		const OsPowerSchemePersonality::Type powerSchemePersonality);
	void registerEvent(FrameworkEvent::Type frameworkEvent);
	void unregisterEvent(FrameworkEvent::Type frameworkEvent);

	EsifServicesInterface* getEsifServices();
};
