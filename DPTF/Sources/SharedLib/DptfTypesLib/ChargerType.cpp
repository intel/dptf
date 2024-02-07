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

#include "ChargerType.h"
#include "DptfBufferStream.h"

namespace ChargerType
{
	std::string ToString(ChargerType::Type type)
	{
		switch (type)
		{
		case ChargerType::Traditional:
			return "Traditional";
		case ChargerType::Hybrid:
			return "Hybrid";
		case ChargerType::NVDC:
			return "NVDC";
		default:
			throw dptf_exception("ChargerType::Type is invalid.");
		}
	}

	DptfBuffer toDptfBuffer(ChargerType::Type type)
	{
		DptfBuffer buffer;
		buffer.append((UInt8*)&type, sizeof(type));
		return buffer;
	}

	ChargerType::Type createFromDptfBuffer(const DptfBuffer& buffer)
	{
		if (buffer.size() != sizeof(ChargerType::Type))
		{
			throw dptf_exception("Buffer given to ChargerType class has invalid length.");
		}

		DptfBuffer bufferCopy = buffer;
		DptfBufferStream stream(bufferCopy);
		ChargerType::Type newRequest = (ChargerType::Type)stream.readNextUInt32();
		return newRequest;
	}

}
