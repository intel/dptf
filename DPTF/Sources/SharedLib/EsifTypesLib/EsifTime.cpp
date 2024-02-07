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

#include "EsifTime.h"
#include "esif_ccb_time.h"

using namespace std;

EsifTime::EsifTime()
{
	refresh();
}

void EsifTime::refresh()
{
	m_timeStamp = getCurrentTime();
	const time_t now = time(nullptr);
	esif_ccb_localtime(&m_localTime, &now);
}

const TimeSpan& EsifTime::getTimeStamp() const
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
		throw dptf_exception("rhs numMilliseconds > internal time stamp"s);
	}

	return m_timeStamp - rhs.getTimeStamp();
}

tm EsifTime::getLocalTime() const
{
	return m_localTime;
}

TimeSpan EsifTime::getCurrentTime() const
{
	esif_ccb_time_t currentTimeInMilliseconds;
	esif_ccb_system_time(&currentTimeInMilliseconds);
	return TimeSpan::createFromMilliseconds(currentTimeInMilliseconds);
}
