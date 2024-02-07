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


#include <stdlib.h>


static ESIF_INLINE void esif_ccb_qsort(
	void *base,
	size_t num,
	size_t size,
	int (*compar)(const void *, const void *, void *),
	void *ctx
	)
{
	if (base && num > 0) {
		qsort_r(base, num, size, compar, ctx);
	}
}

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

