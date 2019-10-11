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
#include <string>
#include "DptfExceptions.h"
#include "DptfBuffer.h"
#include "DptfRequest.h"

class DptfRequestResult
{
public:
	DptfRequestResult();
	DptfRequestResult(Bool isSuccessful, const std::string& message, const DptfRequest& request);
	virtual ~DptfRequestResult();

	Bool isSuccessful() const;
	Bool isFailure() const;
	void throwIfFailure() const;
	void setData(const DptfBuffer& data);
	void setDataFromUInt32(const UInt32 data);
	void setDataFromBool(const Bool data);
	const DptfBuffer& getData() const;
	const UInt32 getDataAsUInt32() const;
	const Bool getDataAsBool() const;
	const DptfRequest& getRequest() const;
	const std::string getMessage() const;

private:
	Bool m_isSuccessful;
	std::string m_message;
	DptfBuffer m_data;
	DptfRequest m_request;
};
