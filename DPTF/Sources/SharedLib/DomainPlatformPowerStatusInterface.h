/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
#include "PlatformPowerSource.h"
#include "ChargerType.h"

class DomainPlatformPowerStatusInterface
{
public:
	virtual ~DomainPlatformPowerStatusInterface() {};

	virtual Power getPlatformRestOfPower(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual Power getAdapterPowerRating(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual PlatformPowerSource::Type getPlatformPowerSource(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual UInt32 getACNominalVoltage(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual UInt32 getACOperationalCurrent(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual Percentage getAC1msPercentageOverload(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual Percentage getAC2msPercentageOverload(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual Percentage getAC10msPercentageOverload(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual void notifyForProchotDeassertion(UIntN participantIndex, UIntN domainIndex) = 0;
};
