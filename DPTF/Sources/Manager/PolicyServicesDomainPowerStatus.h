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
#include "PolicyServices.h"
#include "DomainPowerStatusInterface.h"

class PolicyServicesDomainPowerStatus final : public PolicyServices, public DomainPowerStatusInterface
{
public:
	PolicyServicesDomainPowerStatus(DptfManagerInterface* dptfManager, UIntN policyIndex);

	virtual PowerStatus getPowerStatus(UIntN participantIndex, UIntN domainIndex) override final;
	virtual Power getAveragePower(
		UIntN participantIndex,
		UIntN domainIndex,
		const PowerControlDynamicCaps& capabilities) override final;
	virtual Power getPowerValue(UIntN participantIndex, UIntN domainIndex) override final;
	virtual void setCalculatedAveragePower(UIntN participantIndex, UIntN domainIndex, Power powerValue) override final;
};
