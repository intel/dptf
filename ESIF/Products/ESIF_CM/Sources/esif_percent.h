/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_PERCENT_H_
#define _ESIF_PERCENT_H_

#define ESIF_PERCENT_HDC_CONV_FACTOR 127 /* HDC duty cycle is in 1/127ths % */
#define ESIF_PERCENT_BYTE_CONV_FACTOR 255 /* BYTE percent range is 0 to 255 */
#define ESIF_PERCENT_STD_MAX 10000 /* Representation of the max standard percent */
#define ESIF_PERCENT_MILLI_MAX 100000 /* Representation of the max milli-percent */

/* Percent Unit Type */
enum esif_percent_type {
	ESIF_PERCENT = 0,	/* Straight percentage			*/
	ESIF_PERCENT_DECI,	/* Deci-percent (1% == 10)		*/
	ESIF_PERCENT_CENTI,	/* Centi-percent (1% == 100)		*/
	ESIF_PERCENT_MILLI,	/* Milli-percent (1% == 1000)		*/
	ESIF_PERCENT_BYTE,	/* Percentage of 255 (100% == 255)		*/
};

/* Percent Unit Type String */
static ESIF_INLINE esif_string esif_percent_type_str(
	enum esif_percent_type type
	)
{
	switch (type) {
		ESIF_CASE_ENUM(ESIF_PERCENT);
		ESIF_CASE_ENUM(ESIF_PERCENT_DECI);
		ESIF_CASE_ENUM(ESIF_PERCENT_CENTI);
		ESIF_CASE_ENUM(ESIF_PERCENT_MILLI);
		ESIF_CASE_ENUM(ESIF_PERCENT_BYTE);
	}
	return ESIF_NOT_AVAILABLE;
}


/* Percent Unit Description */
static ESIF_INLINE esif_string esif_percent_type_desc(
enum esif_percent_type type)
{
	switch (type) {
		ESIF_CASE(ESIF_PERCENT, "Percent");
		ESIF_CASE(ESIF_PERCENT_DECI, "Deci-Percent");
		ESIF_CASE(ESIF_PERCENT_CENTI, "Centi-Percent");
		ESIF_CASE(ESIF_PERCENT_MILLI, "Milli-Percent");
		ESIF_CASE(ESIF_PERCENT_BYTE, "255-Based Percent");
	}
	return ESIF_NOT_AVAILABLE;
}


/*
 * Select percent unit to normalize percentages to
 */

/* DPTF wants everything in ms */
#define NORMALIZE_PERCENT_TYPE ESIF_PERCENT_CENTI

/* Normalize percentage */
static ESIF_INLINE int esif_convert_percent(
	enum esif_percent_type in,
	enum esif_percent_type out,
	u32 *value_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	u64 val = 0;

	if (NULL == value_ptr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	val = (u64)(*value_ptr);
	if (val == 0 || in == out)
		goto exit;

	/* Always normalize input to milli-percent first */
	switch (in) {
	case ESIF_PERCENT:
		val *= 1000;
		break;

	case ESIF_PERCENT_DECI:
		val *= 100;
		break;

	case ESIF_PERCENT_CENTI:
		val *= 10;
		break;

	case ESIF_PERCENT_MILLI:
		break;

	case ESIF_PERCENT_BYTE:
		val = val * 100 * 1000 / ESIF_PERCENT_BYTE_CONV_FACTOR;
		break;

	default:
		rc = ESIF_E_UNSUPPORTED_REQUEST_PERCENT_TYPE;
		goto exit;
		break;
	}

	/* Normalize To Output Type */
	switch (out) {
	case ESIF_PERCENT:
		val /= 1000;
		break;

	case ESIF_PERCENT_DECI:
		val /= 100;
		break;

	case ESIF_PERCENT_CENTI:
		val /= 10;
		break;

	case ESIF_PERCENT_MILLI:
		break;

	case ESIF_PERCENT_BYTE:
		// val = Round up (% * 255)
		val = (val * ESIF_PERCENT_BYTE_CONV_FACTOR / 100 + 500) / 1000;
		break;

	default:
		rc = ESIF_E_UNSUPPORTED_RESULT_PERCENT_TYPE;
		goto exit;
		break;
	}

	if (val >= (1ULL << 32)) {
		rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		goto exit;
	}

	*value_ptr = (u32)val;

exit:
	return rc;
}


#endif /* _ESIF_PERCENT_H_ */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

