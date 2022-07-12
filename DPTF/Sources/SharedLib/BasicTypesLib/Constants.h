/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#define POLICY_DISABLED 0x00
#define POLICY_ENABLED 0x01
#define ACTIVE_CONTROL_SUPPORTED 0x02
#define PASSIVE_CONTROL_SUPPORTED 0x04
#define CRITICAL_SHUTDOWN_SUPPORTED 0x08

namespace Constants
{
	const UInt32 Invalid = 0xFFFFFFFFU;
	const std::string InvalidString("X");
	const std::string NotAvailableString("N/A");
	const std::string EmptyString("");

	const UInt8 MaxUInt8 = 0xFFU;
	const UInt16 MaxUInt16 = 0xFFFFU;
	const UInt32 MaxUInt32 = 0xFFFFFFFFU;
	const UInt64 MaxUInt64 = 0xFFFFFFFFFFFFFFFFU;
	const Int32 MaxInt32 = 0x7FFFFFFFU;

	const UIntN DefaultBufferSize = 4096;
	const UIntN GuidSize = 16;

	namespace Participants
	{
		const UIntN MaxParticipantEstimate = 20;
	}

	namespace Esif
	{
		// NoParticipant and NoDomain must be set higher than any reasonable index value
		const UIntN NoParticipant = 1999;
		const UIntN NoDomain = 1999;

		// NoInstance is defined by ESIF to be 255 (default)
		const UInt8 NoInstance = 255;

		// NoPersistInstance indicates the non persistent version of the default instance (255)
		// for a SET Primitive that is NOT Persisted to disk (Reset on Restart)
		const UInt8 NoPersistInstance = 254;

		// NoPersistInstanceOffset indicates an offset added to the Instance ID (0-199)
		// for a SET Primitive that is NOT Persisted to disk (Reset on Restart)
		const UInt8 NoPersistInstanceOffset = 200;
	}
}
