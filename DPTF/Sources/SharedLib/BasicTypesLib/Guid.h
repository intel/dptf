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
#include "esif_sdk_data.h"

class Guid final
{
public:
	static const UIntN GuidSize = 16;

	Guid(void);
	Guid(const UInt8 guid[GuidSize]);
	Guid(
		UInt8 value00,
		UInt8 value01,
		UInt8 value02,
		UInt8 value03,
		UInt8 value04,
		UInt8 value05,
		UInt8 value06,
		UInt8 value07,
		UInt8 value08,
		UInt8 value09,
		UInt8 value10,
		UInt8 value11,
		UInt8 value12,
		UInt8 value13,
		UInt8 value14,
		UInt8 value15);
	static Guid createInvalid();

	Bool operator==(const Guid& rhs) const;
	Bool operator!=(const Guid& rhs) const;
	friend std::ostream& operator<<(std::ostream& os, const Guid& guid);
	operator const UInt8*(void) const;

	Bool isValid() const;
	std::string toString() const;
	static Guid fromString(std::string guidString);
	static Guid fromUnmangledString(std::string guidString);
	void copyToBuffer(UInt8 buffer[GuidSize]) const;

private:
	Bool m_valid;
	UInt8 m_guid[GuidSize];
	void throwIfInvalid(const Guid& guid) const;
};
