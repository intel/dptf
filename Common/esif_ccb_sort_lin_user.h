/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#if defined(ESIF_ATTR_OS_LINUX) && defined(ESIF_ATTR_USER)

#include <stdlib.h>

#if defined (ESIF_ATTR_OS_ANDROID)

// qsort_r unavailable Android Bionic library so use standard qsort
static ESIF_INLINE void esif_ccb_qsort(
	void *base,
	size_t num,
	size_t size,
	int(*compar)(const void *, const void *),
	void *ctx
	)
{
	UNREFERENCED_PARAMETER(ctx);
	qsort(base, num, size, compar);
}

// qsort() callback to case-insenstive sort an array of null-terminated ANSI strings
static ESIF_INLINE int ESIF_CALLCONV esif_ccb_qsort_stricmp(
	const void *arg1,
	const void *arg2
	)
{
	return esif_ccb_stricmp(*(char **)arg1, *(char **)arg2);
}

// qsort() callback to case-senstive sort an array of null-terminated ANSI strings
static ESIF_INLINE int ESIF_CALLCONV esif_ccb_qsort_strcmp(
	const void *arg1,
	const void *arg2
	)
{
	return esif_ccb_strcmp(*(char **)arg1, *(char **)arg2);
}

#else /* Chrome and all other Linux-derived OS */

#define esif_ccb_qsort(bas, num, siz, fn, ctx)	qsort_r(bas, num, siz, fn, ctx)

// qsort() callback to case-insenstive sort an array of null-terminated ANSI strings
static ESIF_INLINE int ESIF_CALLCONV esif_ccb_qsort_stricmp(
	const void *arg1,
	const void *arg2,
	void *ctx
	)
{
	UNREFERENCED_PARAMETER(ctx);
	return esif_ccb_stricmp(*(char **)arg1, *(char **)arg2);
}

// qsort() callback to case-senstive sort an array of null-terminated ANSI strings
static ESIF_INLINE int ESIF_CALLCONV esif_ccb_qsort_strcmp(
	const void *arg1,
	const void *arg2,
	void *ctx
	)
{
	UNREFERENCED_PARAMETER(ctx);
	return esif_ccb_strcmp(*(char **)arg1, *(char **)arg2);
}
#endif /* ANDROID */

#endif /* LINUX USER */