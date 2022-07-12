/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_TRACE_H_
#define _ESIF_TRACE_H_

#include "esif.h"

/*
 * Kernel Module Types:
 * The following enumeration specifies the index of a given LF module into
 * an array of debug masks for each modue type.  Each LF C file should
 * contain a ESIF_DEBUG_MODULE item defined as one of the enum items.
 *
 * Note: The Kernel-Mode Debug Modules definitions are available in User Mode
 * so that the kernel-mode debug level can be decoded in user-mode.
 *
 * Note:  These values are tied to Windows ETW manifest files.
 * Any changes here must be reflected in the manifest files
 * and the associated WIKI page.
 */
enum esif_debug_mod {
	ESIF_DEBUG_MOD_API_TRACE       = 0,  /* OS Trace of Entry/Exit	*/
	ESIF_DEBUG_MOD_ACTION_CONST    = 1,  /* Action Constant		*/
	ESIF_DEBUG_MOD_ACTION_MSR      = 2,  /* Action MSR		*/
	ESIF_DEBUG_MOD_ACTION_MMIO     = 3,  /* Action MMIO		*/
	ESIF_DEBUG_MOD_ACTION_ACPI     = 4,  /* Action ACPI		*/
	ESIF_DEBUG_MOD_IPC             = 5,  /* IPC			*/
	ESIF_DEBUG_MOD_COMMAND         = 6,  /* Command Pre DSP		*/
	ESIF_DEBUG_MOD_PRIMITIVE       = 7,  /* Primitive Requires DSP	*/
	ESIF_DEBUG_MOD_ACTION          = 8,  /* Primitive Requires DSP	*/
	ESIF_DEBUG_MOD_CPC             = 9,  /* Loads DSP		*/
	ESIF_DEBUG_MOD_POWER           = 10, /* Power state change info	*/
	ESIF_DEBUG_MOD_DSP             = 11, /* DSP Operations		*/
	ESIF_DEBUG_MOD_EVENT           = 12, /* Event Processing	*/
	ESIF_DEBUG_MOD_ELF             = 13, /* ESIF Lower Framework	*/
	ESIF_DEBUG_MOD_PMG             = 14, /* Participant Manager	*/
	ESIF_DEBUG_MOD_QUEUE           = 15, /* Queue Manager		*/
	ESIF_DEBUG_MOD_HASH            = 16, /* Hash Tables		*/
	ESIF_DEBUG_MOD_PNP             = 17, /* PnP-related items	*/
	ESIF_DEBUG_MOD_ACTION_CODE     = 18, /* Action Code		*/
	ESIF_DEBUG_MOD_DOMAIN	       = 19, /* Domain Code		*/
	ESIF_DEBUG_MOD_ACTION_MBI      = 20, /* Action MBI (ATOM)	*/
	ESIF_DEBUG_MOD_DRVM            = 21, /* Driver Manager		*/
	ESIF_DEBUG_MOD_WINDOWS         = 22, /* Windows-Specific	*/
	ESIF_DEBUG_MOD_LINUX           = 23, /* Linux-Specific		*/
	ESIF_DEBUG_MOD_ACTION_DELEGATE = 24, /* Action Delegate		*/
	ESIF_DEBUG_MOD_LP              = 25, /* LF Participant		*/
	ESIF_DEBUG_MOD_ACTION_KIOCTL   = 26, /* Action Ioctle		*/
};

#define	ESIF_DEBUG_MOD_MAX	(ESIF_DEBUG_MOD_ACTION_KIOCTL + 1)

static ESIF_INLINE char *esif_debug_mod_str(enum esif_debug_mod mod)
{
	switch (mod) {
	ESIF_CASE(ESIF_DEBUG_MOD_API_TRACE, "API");
	ESIF_CASE(ESIF_DEBUG_MOD_ACTION_CONST, "CON");
	ESIF_CASE(ESIF_DEBUG_MOD_ACTION_MSR, "MSR");
	ESIF_CASE(ESIF_DEBUG_MOD_ACTION_MMIO, "MMI");
	ESIF_CASE(ESIF_DEBUG_MOD_ACTION_ACPI, "ACP");
	ESIF_CASE(ESIF_DEBUG_MOD_IPC, "IPC");
	ESIF_CASE(ESIF_DEBUG_MOD_COMMAND, "CMD");
	ESIF_CASE(ESIF_DEBUG_MOD_PRIMITIVE, "PRI");
	ESIF_CASE(ESIF_DEBUG_MOD_ACTION, "ACT");
	ESIF_CASE(ESIF_DEBUG_MOD_CPC, "CPC");
	ESIF_CASE(ESIF_DEBUG_MOD_POWER, "PWR");
	ESIF_CASE(ESIF_DEBUG_MOD_DSP, "DSP");
	ESIF_CASE(ESIF_DEBUG_MOD_EVENT, "EVE");
	ESIF_CASE(ESIF_DEBUG_MOD_ELF, "ELF");
	ESIF_CASE(ESIF_DEBUG_MOD_PMG, "PMG");
	ESIF_CASE(ESIF_DEBUG_MOD_QUEUE, "QUE");
	ESIF_CASE(ESIF_DEBUG_MOD_HASH, "HSH");
	ESIF_CASE(ESIF_DEBUG_MOD_PNP, "PNP");
	ESIF_CASE(ESIF_DEBUG_MOD_ACTION_CODE, "COD");
	ESIF_CASE(ESIF_DEBUG_MOD_DOMAIN, "DMN");
	ESIF_CASE(ESIF_DEBUG_MOD_ACTION_MBI, "MBI");
	ESIF_CASE(ESIF_DEBUG_MOD_DRVM, "DVM");
	ESIF_CASE(ESIF_DEBUG_MOD_WINDOWS, "WND");
	ESIF_CASE(ESIF_DEBUG_MOD_LINUX, "LNX");
	ESIF_CASE(ESIF_DEBUG_MOD_ACTION_DELEGATE, "DLG");
	ESIF_CASE(ESIF_DEBUG_MOD_LP, "LP ");
	ESIF_CASE(ESIF_DEBUG_MOD_ACTION_KIOCTL, "KIO");
	}
	return ESIF_NOT_AVAILABLE;
}

/* ESIF Trace Levels. These correspond to eLogLevel enum type */
#define ESIF_TRACELEVEL_FATAL       0
#define ESIF_TRACELEVEL_ERROR       1
#define ESIF_TRACELEVEL_WARN        2
#define ESIF_TRACELEVEL_INFO        3
#define ESIF_TRACELEVEL_DEBUG       4

#endif	/* _ESIF_TRACE_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
