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

#pragma once

#include "PerformanceControlFacadeInterface.h"
#include "PolicyServicesInterfaceContainer.h"
#include "DomainProperties.h"

// this facade class provides a simpler interface on top of performance controls as well as combines all of the
// performance control properties and capabilities into a single class.  these properties also have the ability to be
// cached.
class dptf_export PerformanceControlFacade : public PerformanceControlFacadeInterface
{
public:
	PerformanceControlFacade(
		UIntN participantIndex,
		UIntN domainIndex,
		const DomainProperties& domainProperties,
		const PolicyServicesInterfaceContainer& policyServices);
	virtual ~PerformanceControlFacade();

	// controls
	virtual Bool supportsPerformanceControls() override;
	virtual void initializeControlsIfNeeded() override;
	virtual void setControl(UIntN performanceControlIndex) override;
	virtual void setControlsToMax() override;
	virtual void setPerformanceControlDynamicCaps(PerformanceControlDynamicCaps newCapabilities) override;
	virtual void lockCapabilities() override;
	virtual void unlockCapabilities() override;

	// properties
	virtual void refreshCapabilities() override;
	virtual void refreshControls() override;
	virtual PerformanceControlStatus getStatus() const override;
	virtual PerformanceControlStatus getLiveStatus() const override;
	virtual const PerformanceControlSet& getControls() override;
	virtual const PerformanceControlDynamicCaps& getDynamicCapabilities() override;

private:
	// services
	PolicyServicesInterfaceContainer m_policyServices;

	// domain properties
	UIntN m_participantIndex;
	UIntN m_domainIndex;
	DomainProperties m_domainProperties;

	// control properties
	PerformanceControlSetCachedProperty m_performanceControlSetProperty;
	PerformanceControlCapabilitiesCachedProperty m_performanceControlCapabilitiesProperty;
	Bool m_controlsHaveBeenInitialized;
	UIntN m_lastIssuedPerformanceControlIndex;
	const PolicyServicesInterfaceContainer& getPolicyServices() const;
};
