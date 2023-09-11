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

#include "Dptf.h"
#include "DomainType.h"
#include "DomainFunctionalityVersions.h"
#include "XmlNode.h"

class DomainProperties final
{
public:
	DomainProperties(
		const Guid& guid,
		UIntN domainIndex,
		Bool domainEnabled,
		DomainType::Type domainType,
		const std::string& domainName,
		const std::string& domainDescription,
		const DomainFunctionalityVersions& domainFunctionalityVersions);

	Guid getGuid() const;
	UIntN getDomainIndex() const;
	Bool isEnabled() const;
	DomainType::Type getDomainType() const;
	std::string getName() const;
	std::string getDescription() const;
	DomainFunctionalityVersions getDomainFunctionalityVersions() const;

	Bool implementsActiveControlInterface() const;
	Bool implementsActivityStatusInterface() const;
	Bool implementsCoreControlInterface() const;
	Bool implementsDisplayControlInterface() const;
	Bool implementsEnergyControlInterface() const;
	Bool implementsPeakPowerControlInterface() const;
	Bool implementsPerformanceControlInterface() const;
	Bool implementsPowerControlInterface() const;
	Bool implementsSystemPowerControlInterface() const;
	Bool implementsBatteryStatusInterface() const;
	Bool implementsPlatformPowerStatusInterface() const;
	Bool implementsPowerStatusInterface() const;
	Bool implementsDomainPriorityInterface() const;
	Bool implementsRfProfileControlInterface() const;
	Bool implementsRfProfileStatusInterface() const;
	Bool implementsTemperatureInterface() const;
	Bool implementsTemperatureThresholdInterface() const;
	Bool implementsProcessorControlInterface() const;
	Bool implementsUtilizationInterface() const;
	Bool implementsSocWorkloadClassificationInterface() const;
	Bool implementsDynamicEppInterface() const;

	Bool operator==(const DomainProperties& domain);
	Bool operator!=(const DomainProperties& domain);

	std::shared_ptr<XmlNode> getXml() const;

private:
	Guid m_guid;
	UIntN m_domainIndex;
	Bool m_enabled;
	DomainType::Type m_domainType;
	std::string m_name;
	std::string m_description;
	DomainFunctionalityVersions m_domainFunctionalityVersions;

	static Bool isInterfaceImplemented(UInt8 version);
};
