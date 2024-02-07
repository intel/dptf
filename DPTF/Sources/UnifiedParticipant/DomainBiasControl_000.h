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

#include "DomainBiasControlBase.h"

//
// Implements the Null Object pattern.  In the case that the functionality isn't implemented, we use
// this in place so we don't have to check for NULL pointers all throughout the participant implementation.
//

class DomainBiasControl_000 : public DomainBiasControlBase
{
public:
	DomainBiasControl_000(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);

	// DomainBiasControlInterface
	void setCpuOpboostEnableAC(Bool enabled) override;
	void setCpuOpboostEnableDC(Bool enabled) override;
	void setGpuOpboostEnableAC(Bool enabled) override;
	void setGpuOpboostEnableDC(Bool enabled) override;
	void setSplitRatio(const Percentage& splitRatio) override;
	void setSplitRatioMax(const Percentage& splitRatio) override;
	Bool getCpuOpboostEnableAC() override;
	Bool getCpuOpboostEnableDC() override;
	Bool getGpuOpboostEnableAC() override;
	Bool getGpuOpboostEnableDC() override;
	Percentage getSplitRatio() override;
	Percentage getSplitRatioActive() override;
	Percentage getSplitRatioMax() override;
	Power getReservedTgp() override;
	OpportunisticBoostMode::Type getOppBoostMode() override;

	// ComponentExtendedInterface
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;
};
