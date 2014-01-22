/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013 Intel Corporation All Rights Reserved
**
** This program is free software; you can redistribute it and/or modify it under
** the terms of version 2 of the GNU General Public License as published by the
** Free Software Foundation.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
** details.
**
** You should have received a copy of the GNU General Public License along with
** this program; if not, write to the Free Software  Foundation, Inc.,
** 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
** The full GNU General Public License is included in this distribution in the
** file called LICENSE.GPL.
**
** BSD LICENSE
**
** Copyright (c) 2013 Intel Corporation All Rights Reserved
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** * Redistributions of source code must retain the above copyright notice, this
**   list of conditions and the following disclaimer.
** * Redistributions in binary form must reproduce the above copyright notice,
**   this list of conditions and the following disclaimer in the documentation
**   and/or other materials provided with the distribution.
** * Neither the name of Intel Corporation nor the names of its contributors may
**   be used to endorse or promote products derived from this software without
**   specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
*******************************************************************************/

#ifndef _ESIF_CCB_TIME_H_
#define _ESIF_CCB_TIME_H_

/* Agnostic */
typedef u64 esif_ccb_time_t;

/*
** Return system Time In Milliseconds
*/

/* Kernel */
#ifdef ESIF_ATTR_KERNEL
#ifdef ESIF_ATTR_OS_LINUX
static void ESIF_INLINE esif_ccb_system_time(esif_ccb_time_t *time)
{
	struct timeval now = {0};

	do_gettimeofday(&now);
	*time  = (now.tv_sec * 1000);	/* Convert sec to msec */
	*time += (now.tv_usec / 1000);	/* Convert usec to msec */
}


#endif /* ESIF_ATTR_OS_LINUX */
#ifdef ESIF_ATTR_OS_WINDOWS
static void ESIF_INLINE esif_ccb_system_time(esif_ccb_time_t *time)
{
	LARGE_INTEGER tickPerSecond = {0};
	LARGE_INTEGER tick = {0};

	/* get the high resolution counter's accuracy and tick's / sec */
	tick = KeQueryPerformanceCounter(&tickPerSecond);

	/* sytem time in msec not wall clock */
	*time = (tick.QuadPart / (tickPerSecond.QuadPart / 1000));
}


#endif /* ESIF_ATTR_OS_WINDOWS */
#endif /* ESIF_ATTR_KERNEL */

/*
** User
*/
#ifdef ESIF_ATTR_USER

#include "esif.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef ESIF_ATTR_OS_LINUX
#include <sys/time.h>

static void ESIF_INLINE esif_ccb_ctime(
	EsifString time_str,
	size_t len,
	const time_t *time
	)
{
	ctime_r(time, time_str);
}


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


#endif /* ESIF_ATTR_OS_LINUX */
#ifdef ESIF_ATTR_OS_WINDOWS

static void ESIF_INLINE esif_ccb_ctime(
	esif_string time_str,
	size_t len,
	const time_t *time
	)
{
	ctime_s(time_str, len, time);
}


static void ESIF_INLINE esif_ccb_system_time(esif_ccb_time_t *time)
{
	LARGE_INTEGER tickPerSecond = {0};
	LARGE_INTEGER tick = {0};

	/* get the high resolution counter's accuracy and tick / sec */
	QueryPerformanceFrequency(&tickPerSecond);
	QueryPerformanceCounter(&tick);

	/* sytem time in msec not wall clock */
	*time = (tick.QuadPart / (tickPerSecond.QuadPart / 1000));
}


static void ESIF_INLINE esif_ccb_get_time(struct timeval *tv)
{
	LARGE_INTEGER tickPerSecond = {0};
	LARGE_INTEGER tick = {0};
	time_t rawtime;

	/* Get Seconds */
	time(&rawtime);
	tv->tv_sec = (long)rawtime;

	/* get the high resolution counter's accuracy and tick/sec */
	QueryPerformanceFrequency(&tickPerSecond);
	QueryPerformanceCounter(&tick);

	/* Get usec */
	tv->tv_usec =
		(long)(tick.QuadPart / (tickPerSecond.QuadPart / 1000000));
}


#endif /* ESIF_ATTR_OS_WINDOWS */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ESIF_ATTR_USER */
#endif /* _ESIF_CCB_TIME_H_ */

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
