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
#include "esif_ccb_thread.h"
#include "esif_ccb_timer.h"

// Function prototype for timer callback:
// void (*esif_ccb_timer_cb)(void *context);

class EsifTimer
{
public:
	EsifTimer(esif_ccb_timer_cb callbackFunction, void* contextPtr = nullptr);
	~EsifTimer();
	EsifTimer(const EsifTimer& other) = delete;
	EsifTimer(EsifTimer&& other) noexcept = delete;
	EsifTimer& operator=(const EsifTimer& other) = delete;
	EsifTimer& operator=(EsifTimer&& other) noexcept = delete;

	void startTimer(const TimeSpan& expirationTime);
	void cancelTimer();

	Bool isExpirationTimeValid() const;
	const TimeSpan& getExpirationTime() const;

private:
	esif_ccb_timer_cb m_callbackFunction;
	void* m_contextPtr;

	Bool m_timerInitialized;
	esif_ccb_timer_t m_timer;
	TimeSpan m_expirationTime;

	void esifTimerInit();
	void esifTimerKill();
	void esifTimerSet(const TimeSpan& expirationTime);

	static UInt64 calculateMillisecondsUntilTimerExpires(const TimeSpan& expirationTime);
};
