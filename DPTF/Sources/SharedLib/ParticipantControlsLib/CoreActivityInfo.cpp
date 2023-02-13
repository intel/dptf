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

#include "CoreActivityInfo.h"

CoreActivityInfo::CoreActivityInfo() 
	: m_activityCount(Constants::MaxUInt64)
	, m_timestamp(Constants::MaxUInt64)
	, m_isValid(false)
{
}

CoreActivityInfo::CoreActivityInfo(UInt64 activityCount, UInt64 timestamp)
	: m_activityCount(activityCount)
	, m_timestamp(timestamp)
	, m_isValid(true)
{
}

CoreActivityInfo CoreActivityInfo::getCoreActivityInfoFromBuffer(const DptfBuffer& buffer)
{
	if (buffer.size() == 0)
	{
		return CoreActivityInfo();
	}

	UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
	struct esif_data_core_activity_info* currentRow = reinterpret_cast<struct esif_data_core_activity_info*>(data);

	return CoreActivityInfo(
		static_cast<UInt64>(currentRow->activityCount.integer.value),
		static_cast<UInt64>(currentRow->timestamp.integer.value));
}

const UInt64 CoreActivityInfo::getActivityCounter()
{
	return m_activityCount;
}

const UInt64 CoreActivityInfo::getTimestamp()
{
	return m_timestamp;
}

const Bool CoreActivityInfo::isValid()
{
	return m_isValid;
}

CoreActivityInfo::~CoreActivityInfo()
{
}
