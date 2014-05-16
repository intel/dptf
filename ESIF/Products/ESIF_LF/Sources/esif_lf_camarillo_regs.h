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

#ifndef _ESIF_CAMARILLO_REGS_H_
#define _ESIF_CAMARILLO_REGS_H_

/*
 * /////////////////////////////////////////////////////////////////////////////
 * General Definitions
 * /////////////////////////////////////////////////////////////////////////////
 */


#define OFFSET_PKG_THERM_CAMARILLO_INTERRUPT_HSW    0x5820
#define OFFSET_DDR_THERM_CAMARILLO_INTERRUPT_HSW    0x58A0
#define OFFSET_PKG_THERM_CAMARILLO_STATUS_HSW       0x6200
#define OFFSET_DDR_THERM_CAMARILLO_STATUS_HSW       0x6204

#define PCI_CAMARILLO_INTSTAT       0xDC
#define CAMARILLO_INTSTAT_MASK      0x01


/*
 * /////////////////////////////////////////////////////////////////////////////
 * Type Declarations
 * /////////////////////////////////////////////////////////////////////////////
 */

union reg_pkg_therm_camarillo_interrupt_hsw {
	/* OFFSET_PKG_THERM_CAMARILLO_INTERRUPT_HSW 0x5820 */
	struct {
		u32  HIGH_TEMP_INT_ENABLE:1;     /* 0 - */
		u32  LOW_TEMP_INT_ENABLE:1;      /* 1 - */
		u32  PROCHOT_INT_ENABLE:1;       /* 2 - */
		u32  Rsvd1:1;                    /* 3 - */
		u32  OUT_OF_SPEC_INT_ENABLE:1;   /* 4 - */
		u32  Rsvd2:3;                    /* 7:5 - */
		u32  THRESHOLD_1_REL_TEMP:7;     /* 14:8 - */
		u32  THRESHOLD_1_INT_ENABLE:1;   /* 15 - */
		u32  THRESHOLD_2_REL_TEMP:7;     /* 22:16 - */
		u32  THRESHOLD_2_INT_ENABLE:1;   /* 23 - */
		u32  POWER_INT_ENABLE:1;         /* 24 - */
		u32  Rsvd3:7;                    /* 31:25 */
	} u;

	u32  asDword;
};

union reg_ddr_therm_camarillo_interrupt_hsw {
	/* OFFSET_DDR_THERM_CAMARILLO_INTERRUPT_HSW 0x58A0 */
	struct {
		UINT32  ENABLE_WARM_INTERRUPT:1;       /* 0 - */
		UINT32  Rsvd1:1;                       /* 1 */
		UINT32  ENABLE_HOT_INTERRUPT:1;        /* 2 - */
		UINT32  Rsvd2:1;                       /* 3 */
		UINT32  ENABLE_2X_REFRESH_INTERRUPT:1; /* 4 - */
		UINT32  Rsvd3:3;                       /* 7:5 */
		UINT32  ENABLE_THRESHOLD1_INTERRUPT:1; /* 8 - */
		UINT32  Rsvd4:1;                       /* 9 */
		UINT32  ENABLE_THRESHOLD2_INTERRUPT:1; /* 10 - */
		UINT32  Rsvd5:5;                       /* 15:11 */
		UINT32  POLICY_FREE_THRESHOLD1:8;      /* 23:16 - */
		UINT32  POLICY_FREE_THRESHOLD2:8;      /* 31:24 - */
	} u;

	UINT32  asDword;
};

union reg_pkg_therm_camarillo_status_hsw {
	/* OFFSET_PKG_THERM_CAMARILLO_STATUS_HSW 0x6200 */
	struct {
		UINT32  THERMAL_MONITOR_STATUS:1;  /* 0 - */
		UINT32  THERMAL_MONITOR_LOG:1;     /* 1 - */
		UINT32  PROCHOT_STATUS:1;          /* 2 - */
		UINT32  PROCHOT_LOG:1;             /* 3 - */
		UINT32  OUT_OF_SPEC_STATUS:1;      /* 4 - */
		UINT32  OUT_OF_SPEC_LOG:1;         /* 5 - */
		UINT32  THRESHOLD1_STATUS:1;       /* 6 - */
		UINT32  THRESHOLD1_LOG:1;          /* 7 - */
		UINT32  THRESHOLD2_STATUS:1;       /* 8 - */
		UINT32  THRESHOLD2_LOG:1;          /* 9 - */
		UINT32  POWER_LIMITATION_STATUS:1; /* 10 - */
		UINT32  POWER_LIMITATION_LOG:1;    /* 11 - */
		UINT32  Rsvd1:4;                   /* 15:12 */
		UINT32  Temperature:7;             /* 22:16 */
		UINT32  Rsvd2:4;                   /* 26:23 */
		UINT32  Resolution:4;              /* 30:27 */
		UINT32  Valid:1;                   /* 31 */
	} u;

	UINT32  asDword;
};

union reg_ddr_therm_camarillo_status_hsw {
	/* OFFSET_DDR_THERM_CAMARILLO_STATUS_HSW 0x6204 */
	struct {
		UINT32  WARM_THRESHOLD_STATUS:1;  /* 0 - */
		UINT32  WARM_THRESHOLD_LOG:1;     /* 1 - */
		UINT32  HOT_THRESHOLD_STATUS:1;   /* 2 - */
		UINT32  HOT_THRESHOLD_LOG:1;      /* 3 - */
		UINT32  REFRESH2X_STATUS:1;       /* 4 - */
		UINT32  REFRESH2X_LOG:1;          /* 5 - */
		UINT32  FORCEMEMPR_STATUS:1;      /* 6 - */
		UINT32  FORCEMEMPR_LOG:1;         /* 7 - */
		UINT32  THRESHOLD1_STATUS:1;      /* 8 - */
		UINT32  THRESHOLD1_LOG:1;         /* 9 - */
		UINT32  THRESHOLD2_STATUS:1;      /* 10 - */
		UINT32  THRESHOLD2_LOG:1;         /* 11 - */
		UINT32  Rsvd1:20;                 /* 31:12 */
	} u;

	UINT32  asDword;
};


#endif /* _ESIF_CAMARILLO_REGS_H_ */
