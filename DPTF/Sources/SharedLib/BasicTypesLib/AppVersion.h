/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

// AOSP 7.0 header files have major() and minor() defined as macros
// Undefine these macros to avoid compilation errors
#ifdef major
#undef major
#endif

#ifdef minor
#undef minor
#endif

class AppVersion
{
public:
	AppVersion(UInt16 major, UInt16 minor, UInt16 hotfix, UInt16 build);
	AppVersion(UInt64 concatenatedVersions);
	virtual ~AppVersion();
	Bool operator==(const AppVersion& right) const;
	Bool operator!=(const AppVersion& right) const;

	UInt16 major() const;
	UInt16 minor() const;
	UInt16 hotfix() const;
	UInt16 build() const;

	std::string toString() const;
	UInt64 toUInt64() const;

private:
	UInt16 m_major;
	UInt16 m_minor;
	UInt16 m_hotfix;
	UInt16 m_build;
};
