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
#include "DptfBuffer.h"
#include "DptfBufferStream.h"

class EventPayloadEnduranceGamingRequest
{
public:
	EventPayloadEnduranceGamingRequest(UInt8 enduranceGamingRequestStatus)
		: status(enduranceGamingRequestStatus)
	{
		
	}

	EventPayloadEnduranceGamingRequest(const DptfBuffer& payload)
	{
		auto payloadCopy = payload;
		auto data = DptfBufferStream(payloadCopy);
		status = data.readNextUInt8();
	}

	operator DptfBuffer() const
	{
		DptfBuffer buffer;
		const DptfBufferStream bufferStream(buffer);
		bufferStream.appendUInt8(status);
		return buffer;
	}


	UInt8 status;
};
