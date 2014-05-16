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

#ifndef _ESIF_POWER_H_
#define _ESIF_POWER_H_

#include "esif_primitive.h"
#include "esif_dsp.h"

#define POWER_DEBUG 14	/* Debug Module Level */

#ifdef ESIF_ATTR_KERNEL
#define ESIF_TRACE_DYN_POWER(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ELF, POWER_DEBUG, format, ##__VA_ARGS__)
#endif /* ESIF_ATTR_KERNEL */

#ifdef ESIF_ATTR_USER

/*
 * TODO:  User mode does not currently support this debug area.
 * Need to update when user mode unified debug infrastructre
 * is in place.
 */
#define ESIF_TRACE_DYN_POWER NO_ESIF_DEBUG
#endif /* ESIF_ATTR_USER */

/* Power Unit Type */
enum esif_power_unit_type {
	ESIF_POWER_W = 0,	    /* Watts                     */
	ESIF_POWER_DECIW,	    /* Deci Watts   .1 Watts     */
	ESIF_POWER_CENTIW,	    /* Centi Watts  .01 Watts    */
	ESIF_POWER_MILLIW,	    /* Milli Watts .001 Watts    */
	ESIF_POWER_UNIT_ATOM,   /* 1 * (2 ^ RAPL_POWER_UNIT) */
	ESIF_POWER_UNIT_CORE	/* 1 / (2 ^ RAPL_POWER_UNIT)   */
};

/* Power Unit Type String */
static ESIF_INLINE esif_string esif_power_unit_type_str(
	enum esif_power_unit_type type)
{
	#define ESIF_CREATE_POWER_UNIT_TYPE(t, str) case t: str = #t; break;

	esif_string str = ESIF_NOT_AVAILABLE;
	switch (type) {
		ESIF_CREATE_POWER_UNIT_TYPE(ESIF_POWER_W, str)
		ESIF_CREATE_POWER_UNIT_TYPE(ESIF_POWER_DECIW, str)
		ESIF_CREATE_POWER_UNIT_TYPE(ESIF_POWER_CENTIW, str)
		ESIF_CREATE_POWER_UNIT_TYPE(ESIF_POWER_MILLIW, str)
		ESIF_CREATE_POWER_UNIT_TYPE(ESIF_POWER_UNIT_ATOM, str)
		ESIF_CREATE_POWER_UNIT_TYPE(ESIF_POWER_UNIT_CORE, str)
	}
	return str;
}


/* Power Unit Description */
static ESIF_INLINE esif_string esif_power_unit_desc(
	enum esif_power_unit_type type)
{
	#define ESIF_CREATE_POWER_UNIT_DESC(t, td, str) case t: str = td; break;
	esif_string str = ESIF_NOT_AVAILABLE;

	switch (type) {
		ESIF_CREATE_POWER_UNIT_DESC(ESIF_POWER_W, "Watts", str)
		ESIF_CREATE_POWER_UNIT_DESC(ESIF_POWER_DECIW, "DeciW", str)
		ESIF_CREATE_POWER_UNIT_DESC(ESIF_POWER_CENTIW, "CentiW", str)
		ESIF_CREATE_POWER_UNIT_DESC(ESIF_POWER_MILLIW, "MilliW", str)
	        ESIF_CREATE_POWER_UNIT_DESC(ESIF_POWER_UNIT_ATOM, "UnitAtom", str)
		ESIF_CREATE_POWER_UNIT_DESC(ESIF_POWER_UNIT_CORE, "UnitCore", str)
	}
	return str;
}


/*
 * Select Power Unit To Normalize Power To.
 */

/* DPTF Wants Everything In Milli Watts */
#define NORMALIZE_POWER_UNIT_TYPE ESIF_POWER_MILLIW

/* Normalize Power To Watts. */
static ESIF_INLINE int esif_convert_power(
	enum esif_power_unit_type in,
	enum esif_power_unit_type out,
	esif_power_t *power_ptr
	)
{
	esif_power_t val = 0;

	if (NULL == power_ptr)
		return ESIF_E_PARAMETER_IS_NULL;

	val = *power_ptr;
	if (val == 0 || in == out)
		return ESIF_OK;

	/* For the sake of interger operation, do mutiple before divid
	 *   (37/8)*1000 = 4000, (34/8)*1000 = 4000 (less precise)
	 *   (37*1000)/8 = 4625, (34*1000)/8 = 4250 (more precise)
	 */

	/* Convert To Output */
	switch (out) {
	case ESIF_POWER_MILLIW:
		val *= 1000;
		break;

	case ESIF_POWER_CENTIW:
		val *= 100;
		break;

	case ESIF_POWER_DECIW:
		val *= 10;
		break;

	case ESIF_POWER_UNIT_ATOM:
		val *= 32; /* RAPL_POWER_UNIT = 5  1 * (2 ^ POWER_UNIT) = 32 */
		/* LIFU TODO Get Power Unit From GET_RAPL_POWER_UNIT */
		break;

	case ESIF_POWER_UNIT_CORE:
		val *= 8;   /* RAPL_POWER_UNIT = 3  1 * (2 ^ POWER_UNIT) = 8 */
		/* LIFU TODO Get Power Unit From GET_RAPL_POWER_UNIT */
		break;

	case ESIF_POWER_W:
		break;

	default:
		return ESIF_E_UNSUPPORTED_RESULT_POWER_TYPE;
	}

	/* Normalize To Watts */
	switch (in) {
	case ESIF_POWER_MILLIW:
		val /= 1000;
		break;

	case ESIF_POWER_CENTIW:
		val /= 100;
		break;

	case ESIF_POWER_DECIW:
		val /= 10;
		break;

	case ESIF_POWER_UNIT_ATOM:
		val /= 32;  /* RAPL_POWER_UNIT - 5 1 / (2 ^ (POWER_UNIT) = 32 */
		/* LIFU TODO Get Power Unit From GET_RAPL_POWER_UNIT */
		break;

	case ESIF_POWER_UNIT_CORE:
		val /= 8;   /* RAPL_POWER_UNIT = 3 1 / (2 ^ (POWER_UNIT) = 8 */
		/* LIFU TODO Get Power Unit From GET_RAPL_POWER_UNIT */
		break;

	case ESIF_POWER_W:
		break;

	default:
		return ESIF_E_UNSUPPORTED_REQUEST_POWER_TYPE;
	}

	*power_ptr = val;
	return ESIF_OK;
}


#endif /* _ESIF_POWER_H_ */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

