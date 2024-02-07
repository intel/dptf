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

#define ESIF_CCB_LINK_LIST_MAIN
#define ESIF_CCB_TIMER_MAIN
#include "EsifTimer.h"
#include "EsifTime.h"

EsifTimer::EsifTimer(esif_ccb_timer_cb callbackFunction, void* contextPtr)
	: m_callbackFunction(callbackFunction)
	, m_contextPtr(contextPtr)
	, m_timerInitialized(false)
	, m_expirationTime(TimeSpan::createFromSeconds(0))
{
	esif_ccb_memset(&m_timer, 0, sizeof(esif_ccb_timer_t));
}

EsifTimer::~EsifTimer(void)
{
	esifTimerKill();
}

void EsifTimer::startTimer(const TimeSpan& expirationTime)
{
	esifTimerSet(expirationTime);
}

void EsifTimer::cancelTimer(void)
{
	esifTimerKill();
}

Bool EsifTimer::isExpirationTimeValid(void) const
{
	return (m_expirationTime.asMillisecondsInt() != 0);
}

const TimeSpan& EsifTimer::getExpirationTime(void) const
{
	return m_expirationTime;
}

void EsifTimer::esifTimerInit()
{
	if (m_timerInitialized == false)
	{
		const eEsifError rc = esif_ccb_timer_init(&m_timer, m_callbackFunction, m_contextPtr);
		if (rc != ESIF_OK)
		{
			esif_ccb_memset(&m_timer, 0, sizeof(esif_ccb_timer_t));
			throw dptf_exception("Failed to initialize timer.");
		}

		m_timerInitialized = true;
	}
}

void EsifTimer::esifTimerKill()
{
	if (m_timerInitialized == true)
	{
		// Do not check the return code or throw an exception.  ESIF is responsible for
		// killing the timer and we can't do anything if this fails.
		esif_ccb_timer_kill_w_wait(&m_timer);

		esif_ccb_memset(&m_timer, 0, sizeof(esif_ccb_timer_t));
		m_timerInitialized = false;
		m_expirationTime = TimeSpan::createFromMilliseconds(0);
	}
}

void EsifTimer::esifTimerSet(const TimeSpan& expirationTime)
{
	esifTimerInit();

	const eEsifError rc = esif_ccb_timer_set_msec(&m_timer, calculateMillisecondsUntilTimerExpires(expirationTime));
	if (rc != ESIF_OK)
	{
		throw dptf_exception("Failed to start timer.");
	}

	m_expirationTime = expirationTime;
}

UInt64 EsifTimer::calculateMillisecondsUntilTimerExpires(const TimeSpan& expirationTime)
{
	const auto currentTime = EsifTime().getTimeStamp();
	const auto numMilliseconds =
		(expirationTime > currentTime) ? (expirationTime - currentTime) : TimeSpan::createFromMilliseconds(1);
	return numMilliseconds.asMillisecondsUInt();
}