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
#include "esif_sdk_data.h"

class EsifDataUInt32 final
{
public:
	EsifDataUInt32(void);
	EsifDataUInt32(UInt32 data);
	EsifDataUInt32(const EsifDataPtr esifDataPtr);

	operator EsifDataPtr(void);
	operator UInt32(void) const;

private:
	// hide the copy constructor and assignment operator.
	EsifDataUInt32(const EsifDataUInt32& rhs);
	EsifDataUInt32& operator=(const EsifDataUInt32& rhs);

	UInt32 m_esifDataValue;
	EsifData m_esifData;

	void initialize(UInt32 data);
};
