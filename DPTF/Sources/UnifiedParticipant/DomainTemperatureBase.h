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
#include "DomainTemperatureInterface.h"
#include "ControlBase.h"
#include "ParticipantActivityLoggingInterface.h"
#include <CachedValue.h>
#include "ArbitratorTemperatureThresholds.h"

class DomainTemperatureBase : public ControlBase,
	public DomainTemperatureInterface,
	public ParticipantActivityLoggingInterface
{
public:
	DomainTemperatureBase(
		UIntN participantIndex,
		UIntN domainIndex,
		Bool areTemperatureThresholdsSupported,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainTemperatureBase();

	// DomainTemperatureInterface
	virtual TemperatureThresholds getTemperatureThresholds() override;
	virtual void setTemperatureThresholds(const TemperatureThresholds& temperatureThresholds,
		const TemperatureThresholds& lastSetTemperatureThresholds) override;

	// ParticipantActivityLoggingInterface
	virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	virtual void onClearCachedData(void) override;
	virtual std::shared_ptr<XmlNode> getArbitratorXml(UIntN policyIndex) const override;

protected:
	Bool m_areTemperatureThresholdsSupported;
	ArbitratorTemperatureThresholds m_arbitratorTemperatureThresholds;

private:
	void bindRequestHandlers();

	DptfRequestResult handleClearCachedResults(const PolicyRequest& policyRequest);
	DptfRequestResult handleRemovePolicyRequests(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetTemperatureStatus(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetTemperatureThresholds(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetTemperatureThresholds(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetPowerShareTemperatureThreshold(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetCalibrationTable(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetPollingTable(const PolicyRequest& policyRequest);
	DptfRequestResult handleIsVirtualTemperatureControl(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetVirtualTemperature(const PolicyRequest& policyRequest);

	Temperature getAuxTemperatureThreshold(UIntN domainIndex, UInt8 auxNumber);
	Temperature getHysteresis(UIntN domainIndex) const;
	Bool setAux0(Temperature& aux0, UIntN domainIndex);
	Bool setAux1(Temperature& aux1, UIntN domainIndex);
	TemperatureStatus getCurrentTemperatureStatusForPolicy(UIntN policyIndex);
	TemperatureThresholds getCurrentTemperatureThresholdsForPolicy(UIntN policyIndex);
};