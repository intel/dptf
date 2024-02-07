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
#include <string>
#include "Constants.h"
#include "DptfRequestType.h"

class DptfRequest
{
public:
	DptfRequest();
	DptfRequest(DptfRequestType::Enum requestType);
	DptfRequest(DptfRequestType::Enum requestType, UInt32 participantIndex);
	DptfRequest(DptfRequestType::Enum requestType, UInt32 participantIndex, UInt32 domainIndex);
	DptfRequest(DptfRequestType::Enum requestType, const DptfBuffer& data, UInt32 participantIndex, UInt32 domainIndex);
	DptfRequest(DptfRequestType::Enum requestType, const DptfBuffer& data);
	virtual ~DptfRequest() = default;

	DptfRequest(const DptfRequest& other) = default;
	DptfRequest& operator=(const DptfRequest& other) = default;
	DptfRequest(DptfRequest&& other) = default;
	DptfRequest& operator=(DptfRequest&& other) = default;

	DptfRequestType::Enum getRequestType() const;
	UInt32 getParticipantIndex() const;
	UInt32 getDomainIndex() const;
	const DptfBuffer& getData() const;
	UInt32 getDataAsUInt32() const;
	UInt32 getFirstUInt32Data() const;
	void setData(const DptfBuffer& data);
	void setDataFromUInt32(UInt32 data);
	Bool operator==(const DptfRequest& rhs) const;

private:
	DptfRequestType::Enum m_requestType;
	UInt32 m_participantIndex;
	UInt32 m_domainIndex;
	DptfBuffer m_data;
};
