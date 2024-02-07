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
#include "esif_ccb_rc.h"
#include "esif_sdk_data.h"

class EsifDataString final
{
public:
	// create based on an STL string
	EsifDataString(const std::string& data);

	// create empty based on initialBufferSize
	EsifDataString(UIntN initialBufferSize);

	// create from esif data
	EsifDataString(const EsifDataPtr esifDataPtr);

	operator EsifData(void);
	operator EsifDataPtr(void);
	operator std::string(void) const;

private:
	// hide the copy constructor and assignment operator.
	EsifDataString(const EsifDataString& rhs);
	EsifDataString& operator=(const EsifDataString& rhs);

	std::string m_esifDataValue;
	EsifData m_esifData;

	void initialize(const std::string& data);
};

eEsifError FillDataPtrWithString(EsifDataPtr dataPtr, std::string dataString);
