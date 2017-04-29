/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_TEMP_H_
#define _ESIF_TEMP_H_

#define TEMP_DEBUG 13	/* Debug Module Level */
#define ESIF_DISABLED_TEMP_VALUE 0xFFFFFFFF


#ifdef ESIF_ATTR_KERNEL
#define ESIF_TRACE_DYN_TEMP(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ELF, TEMP_DEBUG, format, ##__VA_ARGS__)
#endif /* ESIF_ATTR_KERNEL */

#ifdef ESIF_ATTR_USER

/*
 * TODO:  User mode does not currently support this debug area.
 * Need to update when user mode unified debug infrastructre
 * is in place.
 */
#define ESIF_TRACE_DYN_TEMP NO_ESIF_DEBUG
#endif /* ESIF_ATTR_USER */

/* Termperature Unit Type */
enum esif_temperature_type {
	ESIF_TEMP_K = 0,	/* Kelvin                               */
	ESIF_TEMP_DECIK,	/* Deci Kelvin 300.0 K would be 3000    */
	ESIF_TEMP_CENTIK,	/* Centi Kelvin 300.0 K would be 30000  */
	ESIF_TEMP_MILLIK,	/* Milli Kelvin 300.0 K would be 300000 */
	ESIF_TEMP_C,		/* Celsius                              */
	ESIF_TEMP_DECIC,	/* Deci Celsisus  40.0 C would be 400   */
	ESIF_TEMP_CENTIC,	/* Centi Celsisus 40.0 C would be 4000  */
	ESIF_TEMP_MILLIC,	/* Milli Celsisus 40.0 C would be 40000 */
	ESIF_TEMP_THERMISTOR,   /* EC Thermistor                        */
};

/* Termperature Unit Type String */
static ESIF_INLINE esif_string esif_temperature_type_str(
	enum esif_temperature_type type)
{
	switch (type) {
	ESIF_CASE_ENUM(ESIF_TEMP_K);
	ESIF_CASE_ENUM(ESIF_TEMP_DECIK);
	ESIF_CASE_ENUM(ESIF_TEMP_CENTIK);
	ESIF_CASE_ENUM(ESIF_TEMP_MILLIK);
	ESIF_CASE_ENUM(ESIF_TEMP_C);
	ESIF_CASE_ENUM(ESIF_TEMP_DECIC);
	ESIF_CASE_ENUM(ESIF_TEMP_CENTIC);
	ESIF_CASE_ENUM(ESIF_TEMP_MILLIC);
	ESIF_CASE_ENUM(ESIF_TEMP_THERMISTOR);
	}
	return ESIF_NOT_AVAILABLE;
}


/* Power Unit Description */
static ESIF_INLINE esif_string esif_temperature_type_desc(
	enum esif_temperature_type type
	)
{
	switch (type) {
	ESIF_CASE(ESIF_TEMP_K, "K");
	ESIF_CASE(ESIF_TEMP_DECIK, "DeciK");
	ESIF_CASE(ESIF_TEMP_CENTIK, "CentiK");
	ESIF_CASE(ESIF_TEMP_MILLIK, "MilliK");
	ESIF_CASE(ESIF_TEMP_C, "C");
	ESIF_CASE(ESIF_TEMP_DECIC, "DeciC");
	ESIF_CASE(ESIF_TEMP_CENTIC, "CentiC");
	ESIF_CASE(ESIF_TEMP_MILLIC, "MilliC");
	ESIF_CASE(ESIF_TEMP_THERMISTOR, "Thermistor");
	}
	return ESIF_NOT_AVAILABLE;
}


/*
 * Select Temperature Unit To Normalize Temperatures To.
 */

/* Convert In Between Kelvin and Celsius */
#define DPTF_KELVIN_BASE 2732

/* DPTF Wants Everything In 1/10's Degree K */
#define NORMALIZE_TEMP_TYPE ESIF_TEMP_DECIK

/* Normalize Temp Adjustment based on NORMALIZE_TEMP_TYPE:
 * ESIF_TEMP_K      = (DPTF_KELVIN_BASE / 10)
 * ESIF_TEMP_DECIK  = (DPTF_KELVIN_BASE)
 * ESIF_TEMP_CENTIK = (DPTF_KELVIN_BASE * 10)
 * ESIF_TEMP_MILLIK = (DPTF_KELVIN_BASE * 100)
 * Absolute         = (0)
 */
#define NORMALIZE_TEMP_ADJUSTMENT	(DPTF_KELVIN_BASE)

/* Normalize Values Functions: Add 5, 50 or 500 To Make It Round Up */
static ESIF_INLINE esif_temp_t esif_temp_mc2c(
	enum esif_temperature_type out,
	esif_temp_t val
	)
{
	switch (out) {
	case ESIF_TEMP_C:
		val = ((val) + 500) / 1000;
		break;
	case ESIF_TEMP_DECIC:
		val = ((val) + 50) / 100;
		break;
	case ESIF_TEMP_CENTIC:
		val = ((val) + 5) / 10;
		break;
	case ESIF_TEMP_MILLIC:
	default:
		break;
	}
	return val;
}

static ESIF_INLINE esif_temp_t esif_temp_mk2c(
	enum esif_temperature_type out,
	esif_temp_t val)
{
	int signedVal = val - (DPTF_KELVIN_BASE * 100);
	switch (out) {
	case ESIF_TEMP_C:
		val = (signedVal >= 0) ? 
			(esif_temp_t)((signedVal + 500) / 1000) :
			(esif_temp_t)((signedVal - 500) / 1000);
		break;
	case ESIF_TEMP_DECIC:
		val = (signedVal >= 0) ? 
			(esif_temp_t)((signedVal + 50) / 100) :
			(esif_temp_t)((signedVal - 50) / 100);
		break;
	case ESIF_TEMP_CENTIC:
		val = (signedVal >= 0) ? 
			(esif_temp_t)((signedVal + 5) / 10) :
			(esif_temp_t)((signedVal - 5) / 10);
		break;
	case ESIF_TEMP_MILLIC:
		val = (esif_temp_t)signedVal;
		break;
	default:
		break;
	}
	return val;
}

static ESIF_INLINE esif_temp_t esif_temp_mc2k(
	enum esif_temperature_type out,
	esif_temp_t val
	)
{
	switch (out) {
	case ESIF_TEMP_K:
		val = (((val) + (DPTF_KELVIN_BASE * 100)) + 500) / 1000;
		break;
	case ESIF_TEMP_DECIK:
		val = (((val) + (DPTF_KELVIN_BASE * 100)) + 50) / 100;
		break;
	case ESIF_TEMP_CENTIK:
		val = (((val) + (DPTF_KELVIN_BASE * 100)) + 5) / 10;
		break;
	case ESIF_TEMP_MILLIK:
		val = ((val) + (DPTF_KELVIN_BASE * 100));
		break;
	default:
		break;
	}
	return val;
}

static ESIF_INLINE esif_temp_t esif_temp_mk2k(
	enum esif_temperature_type out,
	esif_temp_t val
	)
{
	switch (out) {
	case ESIF_TEMP_K:
		val = ((val) + 500) / 1000;
		break;
	case ESIF_TEMP_DECIK:
		val = ((val) + 50) / 100;
		break;
	case ESIF_TEMP_CENTIK:
		val = ((val) + 5) / 10;
		break;
	case ESIF_TEMP_MILLIK:
	default:
		break;
	}
	return val;
}

/* Normalize Celisus temperature. */
static ESIF_INLINE int esif_convert_temp(
	enum esif_temperature_type in,
	enum esif_temperature_type out,
	esif_temp_t *temp_ptr
	)
{
	esif_temp_t val = 0;

	if (NULL == temp_ptr)
		return ESIF_E_PARAMETER_IS_NULL;

	if (in == out)
		return ESIF_OK;

	val = *temp_ptr;

	/* Always Raise Up Input Value To Milli-C/K */
	switch (in) {
	case ESIF_TEMP_C:
	case ESIF_TEMP_K:
		val *= 1000;
		break;

	case ESIF_TEMP_DECIC:
	case ESIF_TEMP_DECIK:
		val *= 100;
		break;

	case ESIF_TEMP_CENTIC:
	case ESIF_TEMP_CENTIK:
		val *= 10;
		break;

	case ESIF_TEMP_MILLIC:
	case ESIF_TEMP_MILLIK:
		break;

	default:
		return ESIF_E_UNSUPPORTED_REQUEST_TEMP_TYPE;
	}

	/* Normalize To Output Type */
	switch (out) {
	case ESIF_TEMP_C:
	case ESIF_TEMP_DECIC:
	case ESIF_TEMP_CENTIC:
	case ESIF_TEMP_MILLIC:
		switch (in) {
		case ESIF_TEMP_K:
		case ESIF_TEMP_DECIK:
		case ESIF_TEMP_CENTIK:
		case ESIF_TEMP_MILLIK:
			val = esif_temp_mk2c(out, val);
			break;

		default:
			val = esif_temp_mc2c(out, val);
		}
		break;

	case ESIF_TEMP_K:
	case ESIF_TEMP_DECIK:
	case ESIF_TEMP_CENTIK:
	case ESIF_TEMP_MILLIK:
		switch (in) {
		case ESIF_TEMP_K:
		case ESIF_TEMP_DECIK:
		case ESIF_TEMP_CENTIK:
		case ESIF_TEMP_MILLIK:
			val = esif_temp_mk2k(out, val);
			break;

		default:
			val = esif_temp_mc2k(out, val);
		}
		break;

	default:
		return ESIF_E_UNSUPPORTED_REQUEST_TEMP_TYPE;
	}

	ESIF_TRACE_DYN_TEMP("IN %6u %-6s, OUT %6u %-6s\n",
		*temp_ptr,
		esif_temperature_type_desc(in),
		val,
		esif_temperature_type_desc(out));

	*temp_ptr = val;
	return ESIF_OK;
}


/*
 * Convert a relative temperature to an absolute value based on our normalized
 * type (assumes relative temperature already in normalized units)
 */
static ESIF_INLINE esif_temp_t esif_temp_rel_to_abs(
	esif_temp_t rel_temp
	)
{
	esif_temp_t abs_temp = rel_temp;

	abs_temp = rel_temp + NORMALIZE_TEMP_ADJUSTMENT;

	return abs_temp;
}


/*
 * Convert an absolute temperature to a relative value based on our normalized
 * type (assumes absolute temperature already in normalized units)
 */
static ESIF_INLINE esif_temp_t esif_temp_abs_to_rel(
	esif_temp_t abs_temp
	)
{
	esif_temp_t rel_temp = abs_temp;

	rel_temp = abs_temp - NORMALIZE_TEMP_ADJUSTMENT;

	/* no negative relative temps are allowed */
	if (((int)rel_temp) < 0) {
		rel_temp = 0;
	}
	return rel_temp;
}


#endif /* _ESIF_TEMP_H_ */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

