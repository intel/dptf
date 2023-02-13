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

class EsifTime final
{
public:
	// Creates a new instance of EsifTime initialized to the current time.
	EsifTime(void);

	// sets the internal structure to the current time.
	void refresh(void);

	// returns the internal time stamp
	const TimeSpan& getTimeStamp(void) const;

	tm getLocalTime(void);

	Bool operator==(const EsifTime& rhs) const;
	Bool operator!=(const EsifTime& rhs) const;
	Bool operator>(const EsifTime& rhs) const;
	Bool operator>=(const EsifTime& rhs) const;
	Bool operator<(const EsifTime& rhs) const;
	Bool operator<=(const EsifTime& rhs) const;
	TimeSpan operator-(const EsifTime& rhs) const;

private:
	TimeSpan m_timeStamp; // stores time stamp for fixed point in time
	tm m_localTime;

	TimeSpan getCurrentTime(void);
};
