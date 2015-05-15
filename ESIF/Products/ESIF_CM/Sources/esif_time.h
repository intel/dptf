/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_TIME_H_
#define _ESIF_TIME_H_

#define TIME_DEBUG 15	/* Debug Module Level */

#ifdef ESIF_ATTR_KERNEL
#define ESIF_TRACE_DYN_TIME(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ELF, TIME_DEBUG, format, ##__VA_ARGS__)
#endif /* ESIF_ATTR_KERNEL */

#ifdef ESIF_ATTR_USER

/*
* TODO:  User mode does not currently support this debug area.
* Need to update when user mode unified debug infrastructre
* is in place.
*/
#define ESIF_TRACE_DYN_TIME NO_ESIF_DEBUG
#endif /* ESIF_ATTR_USER */

/* Time Unit Type */
enum esif_time_type {
	ESIF_TIME_S = 0,	/* Seconds				*/
	ESIF_TIME_DECIS,	/* Deciseconds 1s would be 10		*/
	ESIF_TIME_CENTIS,	/* Centiseconds 1S would be 100		*/
	ESIF_TIME_MILLIS,	/* Milliseconds 1S would be 1000	*/
	ESIF_TIME_MICROS	/* Microseconds 1S would be 1000000	*/
};

/* Time Unit Type String */
static ESIF_INLINE esif_string esif_time_type_str(
	enum esif_time_type type
	)
{
	switch (type) {
		ESIF_CASE_ENUM(ESIF_TIME_S);
		ESIF_CASE_ENUM(ESIF_TIME_DECIS);
		ESIF_CASE_ENUM(ESIF_TIME_CENTIS);
		ESIF_CASE_ENUM(ESIF_TIME_MILLIS);
		ESIF_CASE_ENUM(ESIF_TIME_MICROS);
	}
	return ESIF_NOT_AVAILABLE;
}


/* Time Unit Description */
static ESIF_INLINE esif_string esif_time_type_desc(
enum esif_time_type type)
{
	switch (type) {
		ESIF_CASE(ESIF_TIME_S, "s");
		ESIF_CASE(ESIF_TIME_DECIS, "Deciseconds");
		ESIF_CASE(ESIF_TIME_CENTIS, "Centiseconds");
		ESIF_CASE(ESIF_TIME_MILLIS, "ms");
		ESIF_CASE(ESIF_TIME_MICROS, "us");
	}
	return ESIF_NOT_AVAILABLE;
}


/*
 * Select time unit to normalize temperatures to
 */

/* DPTF wants everything in ms */
#define NORMALIZE_TIME_TYPE ESIF_TIME_MILLIS

/* Normalize time */
static ESIF_INLINE int esif_convert_time(
	enum esif_time_type in,
	enum esif_time_type out,
	esif_time_t *time_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	u64 val = 0;

	if (NULL == time_ptr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	val = (u64)(*time_ptr);
	if (val == 0 || in == out)
		goto exit;

	/* Always normalize input to microseconds first */
	switch (in) {
	case ESIF_TIME_S:
		val *= 1000000;
		break;

	case ESIF_TIME_DECIS:
		val *= 100000;
		break;

	case ESIF_TIME_CENTIS:
		val *= 10000;
		break;

	case ESIF_TIME_MILLIS:
		val *= 1000;
		break;

	case ESIF_TIME_MICROS:
		break;

	default:
		rc = ESIF_E_UNSUPPORTED_REQUEST_TIME_TYPE;
		goto exit;
		break;
	}

	/* Normalize To Output Type */
	switch (out) {
	case ESIF_TIME_S:
		val /= 1000000;
		break;

	case ESIF_TIME_DECIS:
		val /= 100000;
		break;

	case ESIF_TIME_CENTIS:
		val /= 10000;
		break;

	case ESIF_TIME_MILLIS:
		val /= 1000;
		break;

	case ESIF_TIME_MICROS:
		break;

	default:
		rc = ESIF_E_UNSUPPORTED_RESULT_TIME_TYPE;
		goto exit;
		break;
	}

	ESIF_TRACE_DYN_TIME("IN %u %s, OUT %llu %s\n",
		*time_ptr,
		esif_time_type_desc(in),
		val,
		esif_time_type_desc(out));

	if (val >= (1ULL << 32)) {
		rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		goto exit;
	}

	*time_ptr = (u32)val;

exit:
	return rc;
}


#endif /* _ESIF_TIME_H_ */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

