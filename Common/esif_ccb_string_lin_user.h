/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

#include <ctype.h>
#include <stdarg.h>
#include <string.h>

/* Enable GNU Extensions for fnmatch() so we can use FNM_CASEFOLD */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <fnmatch.h>
#undef  _GNU_SOURCE
#else
#include <fnmatch.h>
#endif

/*
* Kernel/User Agnostic
*/

/* Safe strcpy that always null terminates target and truncates src if necessary */
static ESIF_INLINE void esif_ccb_strcpy(
	char *dst,
	const char *src,
	size_t siz
	)
{
	if (siz) {
		strncpy(dst, src, siz);
		dst[siz - 1] = 0;
	}
}

/* Safe strcat that always null terminates target and truncates src if necessary */
static ESIF_INLINE void esif_ccb_strcat(
	char *dst,
	const char *src,
	size_t siz
	)
{
	if (siz) {
		strncat(dst, src, siz - strnlen(dst, siz) - 1);
	}
}

#define esif_ccb_sprintf(siz, str, fmt, ...)	snprintf(str, siz, fmt, ##__VA_ARGS__)
#define esif_ccb_strlen(str, siz)		strnlen(str, siz)
#define esif_ccb_strpbrk(str, set)		strpbrk(str, set)
#define esif_ccb_strcspn(str, set)		strcspn(str, set)

/*
 * User Mode Only
 *
 * NOTE: SCANFBUF is mandatory when using esif_ccb_sscanf or esif_ccb_vsscanf to scan into strings:
 *       esif_ccb_sscanf(str, "%s=%d", SCANFBUF(name, sizeof(name)), &value);
 */
#define SCANFBUF(str, siz)			str
#define esif_ccb_vsprintf(siz, str, fmt, ...)	vsnprintf(str, siz, fmt, ##__VA_ARGS__)
#define esif_ccb_vsscanf(str, fmt, args)	vsscanf(str, fmt, args)
#define esif_ccb_sscanf(str, fmt, ...)		sscanf(str, fmt, ##__VA_ARGS__)
#define esif_ccb_strtok(str, sep, ctxt)		strtok_r(str, sep, ctxt)
#define esif_ccb_strcmp(s1, s2)			strcmp(s1, s2)
#define esif_ccb_stricmp(s1, s2)		strcasecmp(s1, s2)
#define esif_ccb_strncmp(s1, s2, count)		strncmp(s1, s2, count)
#define esif_ccb_strnicmp(s1, s2, cnt)		strncasecmp(s1, s2, cnt)
#define esif_ccb_strstr(str, sub)		strstr(str, sub)
#define esif_ccb_strchr(str, chr)		strchr(str, chr)
#define esif_ccb_strrchr(str, chr)		strrchr(str, chr)

/* Use Memtrace Overrides or Standard Functions? */
#ifdef ESIF_ATTR_MEMTRACE
#include "esif_ccb_memtrace.h"
#else
#define esif_ccb_strdup(str)			strdup(str)
#endif

/* Linux _strupr_s() equlivalent */
static ESIF_INLINE void esif_ccb_strupr(
	char *s,
	size_t count
	)
{
	for (; *s && count; s++, count--)
		*s = toupper(*s);
}


/* Linux _strlwr_s() equlivalent */
static ESIF_INLINE void esif_ccb_strlwr(
	char *s,
	size_t count
	)
{
	for (; s && *s && count; s++, count--)
		*s = tolower(*s);
}


/* Linux _vscprintf() equivalient */
static ESIF_INLINE int esif_ccb_vscprintf(
	const char *format,
	va_list args
	)
{
	int retval;
	va_list argcopy;
	va_copy(argcopy, args);
	retval = vsnprintf(0, 0, format, argcopy);
	va_end(argcopy);
	return retval;
}

/* Safe sprintf that concatenates results to the end of a string and always null terminates target */
static ESIF_INLINE int esif_ccb_sprintf_concat(
	size_t siz,		/* total size of str buffer, including existing string */
	char *str,		/* null terminated string */
	const char *fmt,	/* format string */
	...)
{
	int rc = 0;
	size_t len = esif_ccb_strlen(str, siz);
	if (siz > len) {
		va_list args;
		va_start(args, fmt);
		rc = esif_ccb_vsprintf(siz - len, str + len, fmt, args);
		va_end(args);
	}
	return rc;
}

/* Safe strncpy that avoids buffer overruns, always null-terminates target, and pads nulls to end of buffer */
static ESIF_INLINE void esif_ccb_strncpy(
	char *dst,
	const char *src,
	size_t siz)
{
	size_t len = esif_ccb_strlen(src, siz);
	esif_ccb_strcpy(dst, src, siz);
	if (len < siz)
		memset(dst + len, 0, siz - len);
}

/* Case-insensitive string pattern match function (only "*" and "?" supported) */
static ESIF_INLINE int esif_ccb_strmatch(
	esif_string string,
	esif_string pattern
	)
{
	int result = 0;

	/* [RegEx] Pattern matching unsupported in Windows so only match patterns without [RegEx] */
	if (esif_ccb_strpbrk(pattern, "[]") == NULL) {
		result = (int)(fnmatch(pattern, string, FNM_NOESCAPE | FNM_CASEFOLD) == 0);
	}
	return result;
}

#endif /* LINUX USER */