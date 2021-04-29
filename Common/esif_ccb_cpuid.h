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

#pragma once

/*
 * C/C++ OS-Agnostic Universal Implementation of the cpuid intrinsic.
 */

/*
 * Some common cpuid specific defines
 */
#define ESIF_CPUID_BRAND_MOD_LEN    2
#define ESIF_CPUID_BRAND_STR_LEN    48

#define ESIF_CPUID_BRAND_MOD_I3    "i3"
#define ESIF_CPUID_BRAND_MOD_I5    "i5"
#define ESIF_CPUID_BRAND_MOD_I7    "i7"
#define ESIF_CPUID_BRAND_MOD_I9    "i9"
#define ESIF_CPUID_BRAND_MOD_NA    "NA"

/*
 * Various CPUID Leaf's
 */
#define ESIF_CPUID_LEAF_CPU_INFO 0x0
#define ESIF_CPUID_LEAF_PROCESSOR_SIGNATURE 0x1
#define ESIF_CPUID_LEAF_XTAL_CLOCK_FREQ_INFO 0x15
#define ESIF_CPUID_LEAF_CPU_BRAND_STR_SUPPORT 0x80000000
#define ESIF_CPUID_LEAF_CPU_BRAND_STR_PART1 0x80000002
#define ESIF_CPUID_LEAF_CPU_BRAND_STR_PART2 0x80000003
#define ESIF_CPUID_LEAF_CPU_BRAND_STR_PART3 0x80000004

/*
 * Various CPU Family models
 */
// TBD: Merge the list from the windows file here eventually.
#define CPUID_FAMILY_MODEL_MASK     0x0FFF0FF0		// 27:20-xFam, 19:16-xMod, 11:8-Fam, 7:4-Mod
#define CPUID_FAMILY_MODEL_SNB      0x000206A0		// Sandy Bridge
#define CPUID_FAMILY_MODEL_IVB      0x000306A0		// Ivy Bridge
#define CPUID_FAMILY_MODEL_HSW      0x000306C0		// Haswell
#define CPUID_FAMILY_MODEL_CHT      0x000406C0		// CherryTrail-T/Braswell/Cherry View (Platform)
#define CPUID_FAMILY_MODEL_BSW      0x000406C0		// CherryTrail-T/Braswell/Cherry View (Platform)
#define CPUID_FAMILY_MODEL_BDW      0x000306D0		// Broadwell
#define CPUID_FAMILY_MODEL_SKL      0x000406E0		// Sky Lake
#define CPUID_FAMILY_MODEL_KBL      0x000806E0		// Kaby Lake
#define CPUID_FAMILY_MODEL_CVT      0x00030650		// Clover Trail
#define CPUID_FAMILY_MODEL_HSW_ULT  0x00040650		// Haswell ULT
#define CPUID_FAMILY_MODEL_CRW      0x00040660		// Crystal Well
#define CPUID_FAMILY_MODEL_BYT      0x00030670		// BayTrail-T/BayTrail-CR/Valley View
#define CPUID_FAMILY_MODEL_BXT      0x000506C0		// Broxton-A0/A1 / Broxton-E0 / Broxton-P
#define CPUID_FAMILY_MODEL_GLK      0x000706A0		// Gemini Lake (BXT follow-on)
#define CPUID_FAMILY_MODEL_CNL_ULT  0x00060660		// Cannon Lake U/Y
#define CPUID_FAMILY_MODEL_CNL_H    0x00060670		// Cannon Lake H
#define CPUID_FAMILY_MODEL_ICL      0x000706E0		// Ice Lake
#define CPUID_FAMILY_MODEL_LKF      0x000806A0		// Lakefield
#define CPUID_FAMILY_MODEL_RKL      0x000806C0		// Rocket Lake U/Y
#define CPUID_FAMILY_MODEL_RKL_H    0x000806D0		// Rocket Lake H/S

#pragma pack(push, 1)

typedef struct esif_ccb_cpuid_s {
	u32 eax;
	u32 ebx;
	u32 ecx;
	u32 edx;
	u32 leaf;
} esif_ccb_cpuid_t;

#pragma pack(pop)

#if defined(ESIF_ATTR_KERNEL)
#if defined(ESIF_ATTR_OS_WINDOWS)
#include "esif_ccb_cpuid_win_kern.h"
#elif defined(ESIF_ATTR_OS_LINUX)
#include "esif_ccb_cpuid_lin_kern.h"
#endif
#elif defined(ESIF_ATTR_USER)
#if defined(ESIF_ATTR_OS_WINDOWS)
#include "esif_ccb_cpuid_win_user.h"
#elif defined(ESIF_ATTR_OS_LINUX)
#include "esif_ccb_cpuid_lin_user.h"
#endif

#endif /* USER */
