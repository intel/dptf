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

#include "AppVersion.h"
#include "NumberOps.h"
#include <sstream>

AppVersion::AppVersion(UInt16 major, UInt16 minor, UInt16 hotfix, UInt16 build)
	: m_major(major)
	, m_minor(minor)
	, m_hotfix(hotfix)
	, m_build(build)
{
}

AppVersion::AppVersion(UInt64 concatenatedVersions)
	: m_major(NumberOps::getWord(concatenatedVersions, 3))
	, m_minor(NumberOps::getWord(concatenatedVersions, 2))
	, m_hotfix(NumberOps::getWord(concatenatedVersions, 1))
	, m_build(NumberOps::getWord(concatenatedVersions, 0))
{
}

AppVersion::~AppVersion()
{
}

Bool AppVersion::operator==(const AppVersion& right) const
{
	return toUInt64() == right.toUInt64();
}

Bool AppVersion::operator!=(const AppVersion& right) const
{
	return !(*this == right);
}

UInt16 AppVersion::major() const
{
	return m_major;
}

UInt16 AppVersion::minor() const
{
	return m_minor;
}

UInt16 AppVersion::hotfix() const
{
	return m_hotfix;
}

UInt16 AppVersion::build() const
{
	return m_build;
}

std::string AppVersion::toString() const
{
	std::stringstream stringVersion;
	stringVersion << m_major << "." << m_minor << "." << m_hotfix << "." << m_build;
	return stringVersion.str();
}

UInt64 AppVersion::toUInt64() const
{
	return NumberOps::concatenateFourWords(m_major, m_minor, m_hotfix, m_build);
}
