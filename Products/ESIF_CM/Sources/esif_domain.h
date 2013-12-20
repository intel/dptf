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

#ifndef _ESIF_DOMAIN_H_
#define _ESIF_DOMAIN_H_

#include "esif.h"

#ifdef ESIF_ATTR_KERNEL

/*
** Domain
*/
#define ESIF_DOMAIN_VERSION 0x1
#define ESIF_DOMAIN_MAX 10
#define ESIF_DOMAIN_TEMP_INVALID 0xffffffff

/* Translate Qualifier To Index */
static ESIF_INLINE enum esif_rc esif_lp_domain_index (
	u16 domain,
	u8 *index
	)
{
	enum esif_rc rc = ESIF_OK;

	switch (domain) {
	case '0D':	/* D0 */
		*index = 0;
		break;

	case '1D':	/* D1 */
		*index = 1;
		break;

	case '2D':	/* D2 */
		*index = 2;
		break;

	case '3D':	/* D3 */
		*index = 3;
		break;

	case '4D':	/* D4 */
		*index = 4;
		break;

	case '5D':	/* D5 */
		*index = 5;
		break;

	case '6D':	/* D6 */
		*index = 6;
		break;

	case '7D':	/* D7 */
		*index = 7;
		break;

	case '8D':	/* D8 */
		*index = 8;
		break;

	case '9D':	/* D9 */
		*index = 9;
		break;

	default:
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		break;
	}
	return rc;
}


struct esif_lp_domain {
	u8  instance;		/* Instance */
	esif_flags_t           capabilities;	/* Capabilities */
	enum esif_domain_type  domainType;	/* Domain Type */
	u16                    id;		/* Domain ID  */
	char                   *name_ptr;	/* Name               */
	char                   *desc_ptr;	/* Describe Qualifier */
	struct esif_lp         *lp_ptr;		/* Lower Participant Back Ref
						 * */

	/* ESIF will only poll when it has to otherwise it will utilize hardware
	** provided programmable thresholds and notifications but if it has no
	** other method it will poll.
	*/
	u8  poll;		/* Is Polling?   */
	esif_flags_t  poll_mask;		/* Want To Poll? */

	/* Timer For Polling */
	esif_ccb_timer_t  timer;		/* Periodic Work Timer  */
	u32  timer_period_msec;	/* Periodic Interval    */

	/* RAPL / Power */
	u32  rapl_energy_units_current;	/* Last energy accumulator read */
	u32  rapl_energy_units_last;	/* Read before for calc delta */
	u32  rapl_energy_units_per_sec;	/* Actual Energy Units Used Per Sec */
	u32  rapl_power;		/* Calculated Power */

	/* Temperature Thresholds */
	esif_temp_t  temp_cache0;	/* Cache Lower */
	esif_temp_t  temp_cache1;	/* Cache Upper */
	esif_temp_t  temp_aux0;		/* Lower */
	/* enum esif_lp_domain_th_state temp_aux0_state;  / * state machine
	 * state * / */
	u8           temp_notify_sent;
	esif_temp_t  temp_aux1;		/* Upper */
	/* enum esif_lp_domain_th_state temp_aux1_state;  / * State machine
	 * state * / */
	esif_temp_t  temp_hysteresis;	/* Lower Hysteresis */

	/* Power Thresholds */
	esif_power_t  power_aux0;	/* Lower */
	esif_power_t  power_aux1;	/* Upper */
	esif_power_t  power_hysteresis;	/* Lower Hysteresis */

	/* Unit Data */
	esif_temp_t   temp_tjmax;        /* Tjmax */
	esif_power_t  unit_power;        /* Power Unit */
	esif_power_t  unit_energy;       /* Energt Unit */
	u32           unit_time;         /* Time Unit */

#ifdef ESIF_ATTR_OS_LINUX
	struct device  device; /* Lower Participant Qualifier Class Device */
	struct thermal_zone_device     *tzd_ptr; /* Thermal Zone Device If Any */
	struct thermal_cooling_device  *cdev_ptr;	/* Cooling Device If Any */
#endif
};

#endif /* ESIF_ATTR_KERNEL */

#ifdef ESIF_ATTR_USER
#endif /* ESIF_ATTR_USER */
#endif /* _ESIF_DOMAIN_H_ */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
