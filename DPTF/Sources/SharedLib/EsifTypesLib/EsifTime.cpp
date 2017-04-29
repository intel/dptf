/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#include "EsifTime.h"
#include "esif_ccb_time.h"

EsifTime::EsifTime(void)
{
	m_timeStamp = getCurrentTime();
}

EsifTime::EsifTime(UInt64 numMilliSeconds)
	: m_timeStamp(TimeSpan::createFromMilliseconds(numMilliSeconds))
{
}

void EsifTime::refresh(void)
{
	m_timeStamp = getCurrentTime();
}

const TimeSpan& EsifTime::getTimeStamp(void) const
{
	return m_timeStamp;
}

Bool EsifTime::operator==(const EsifTime& rhs) const
{
	return (this->getTimeStamp() == rhs.getTimeStamp());
}

Bool EsifTime::operator!=(const EsifTime& rhs) const
{
	return (this->getTimeStamp() != rhs.getTimeStamp());
}

Bool EsifTime::operator>(const EsifTime& rhs) const
{
	return (this->getTimeStamp() > rhs.getTimeStamp());
}

Bool EsifTime::operator>=(const EsifTime& rhs) const
{
	return (this->getTimeStamp() >= rhs.getTimeStamp());
}

Bool EsifTime::operator<(const EsifTime& rhs) const
{
	return (this->getTimeStamp() < rhs.getTimeStamp());
}

Bool EsifTime::operator<=(const EsifTime& rhs) const
{
	return (this->getTimeStamp() <= rhs.getTimeStamp());
}

TimeSpan EsifTime::operator-(const EsifTime& rhs) const
{
	if (rhs.getTimeStamp() > m_timeStamp)
	{
		throw dptf_exception("rhs numMilliSeconds > internal time stamp");
	}

	return m_timeStamp - rhs.getTimeStamp();
}

TimeSpan EsifTime::getCurrentTime(void)
{
	esif_ccb_time_t currentTimeInMilliSeconds;
	esif_ccb_system_time(&currentTimeInMilliSeconds);
	return TimeSpan::createFromMilliseconds(currentTimeInMilliSeconds);
}
