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
#include "EnvironmentProfile.h"
#include "EnvironmentProfileGenerator.h"
#include "EsifServicesInterface.h"
#include "MessageLogger.h"

class dptf_export RealEnvironmentProfileGenerator : public EnvironmentProfileGenerator
{
public:
	RealEnvironmentProfileGenerator(const std::shared_ptr<MessageLogger>& messageLogger, EsifServicesInterface* esifServices);
	~RealEnvironmentProfileGenerator() override = default;

	EnvironmentProfile generateWithCpuIdOnly() const override;
	EnvironmentProfile generateWithFullProfile(UInt32 socParticipant, UInt32 socDomain) const override;

private:
	static void throwIfInvalidConfigTdpLevel(UInt32 configTdpLevel);
	static UInt64 getCpuId(void);
	Power getSocBasePower(UInt32 socParticipant, UInt32 socDomain) const;
	Power getConfigTdpLevel(UInt32 socParticipant, UInt32 socDomain) const;
	Power getProcessorTdpLevel(UInt32 socParticipant, UInt32 socDomain) const;
	std::shared_ptr<MessageLogger> m_messageLogger;
	EsifServicesInterface* m_esifServices;
};
