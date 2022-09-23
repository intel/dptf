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


#include <sys/time.h>
#include <time.h>
#include <errno.h>

static void ESIF_INLINE esif_ccb_ctime(
	esif_string time_str,
	size_t len,
	const time_t *time
	)
{
	ctime_r(time, time_str);
}

/* Return system Time In Milliseconds */
static void ESIF_INLINE esif_ccb_system_time(esif_ccb_time_t *time)
{
	struct timeval now = {0};

	gettimeofday(&now, NULL);
	*time  = (now.tv_sec * 1000);	/* Convert sec to msec */
	*time += (now.tv_usec / 1000);	/* Convert usec to msec */
}


static void ESIF_INLINE esif_ccb_get_time(struct timeval *tv)
{
	gettimeofday(tv, NULL);
}

static int ESIF_INLINE esif_ccb_localtime(
	struct tm *tm_ptr,
	const time_t *time
	)
{
	return (localtime_r(time, tm_ptr) != NULL ? 0 : EINVAL);
}

static int ESIF_INLINE esif_ccb_gmtime(
	struct tm *tm_ptr,
	const time_t *time
	)
{
	return (gmtime_r(time, tm_ptr) != NULL ? 0 : EINVAL);
}

/* Real-Time Functions */

static ESIF_INLINE esif_ccb_realtime_t esif_ccb_realtime_current(void)
{
	time_t now = time(NULL);
	struct timespec ticks = { 0 };
	clock_gettime(CLOCK_BOOTTIME, &ticks);
	esif_ccb_realtime_t result = { 0 };
	result.clockticks = (((u64)ticks.tv_sec * 1000000000) + ticks.tv_nsec);
	result.clocktime = (u64)now;
	return result;
}

static ESIF_INLINE double esif_ccb_realtime_diff_msec(esif_ccb_realtime_t t1, esif_ccb_realtime_t t2)
{
	struct timespec res;
	clock_getres(CLOCK_BOOTTIME, &res);
	unsigned long long dur64 = t2.clockticks - t1.clockticks;
	unsigned long long res64 = ((unsigned long long)res.tv_sec * 1000000000) + res.tv_nsec;
	return (double)(dur64 / res64) / 1000000.0;
}

static ESIF_INLINE Int64 esif_ccb_realtime_diff_sec(esif_ccb_realtime_t t1, esif_ccb_realtime_t t2)
{
	struct timespec res;
	clock_getres(CLOCK_BOOTTIME, &res);
	unsigned long long dur64 = t2.clockticks - t1.clockticks;
	unsigned long long res64 = ((unsigned long long)res.tv_sec * 1000000000) + res.tv_nsec;
	return (Int64)(dur64 / res64 / 1000000000);
}

static ESIF_INLINE time_t esif_ccb_realtime_clocktime(esif_ccb_realtime_t time)
{
	return (time_t)(time.clocktime);
}

static ESIF_INLINE esif_ccb_realtime_t esif_ccb_realtime_null()
{
	esif_ccb_realtime_t result = { 0 };
	return result;
}

