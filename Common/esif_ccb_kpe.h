/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#include "esif_ccb.h"
#include "esif_ccb_rc.h"
#include "esif_sdk_iface_kpe.h"


/*
 * MACRO DEFINITIONS
 */

#ifdef ESIF_ATTR_OS_WINDOWS
#include <initguid.h>
#include <wdm.h>

#ifdef ESIF_ATTR_DEBUG
#define KPE_TRACE_MSG(fmt, ...) DbgPrint("[%s@%s#%d]: " fmt, \
	ESIF_FUNC, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define KPE_TRACE_MSG(fmt, ...) (0)
#endif

#define PNP_INTERFACE_VERSION_KPE	2

/*
 * The following GUID is used to register for PnP notification for when the
 * DPTF driver arrives/leaves:
 * {EE27098E-1B22-472A-89D8-5CCCE16B1356}
 */
DEFINE_GUID(GUID_PNP_INTERFACE_ESIF_LF,
0xee27098e, 0x1b22, 0x472a, 0x89, 0xd8, 0x5c, 0xcc, 0xe1, 0x6b, 0x13, 0x56);

/*
 * The following GUID is used to query for the KPE interface once the DPTF
 * driver is present:
 * {286F08C0-C9BE-4C6E-A0A3-152D3F167D78}
 */
DEFINE_GUID(GUID_PNP_INTERFACE_KPE,
0x286f08c0, 0xc9be, 0x4c6e, 0xa0, 0xa3, 0x15, 0x2d, 0x3f, 0x16, 0x7d, 0x78);

typedef struct _PNP_INTERFACE_KPE {
	INTERFACE  InterfaceHeader;
	enum esif_rc (*esif_lf_register_driver)(struct esif_driver_iface *diPtr);
	enum esif_rc (*esif_lf_unregister_driver)(struct esif_driver_iface *diPtr);
} PNP_INTERFACE_KPE, *PPNP_INTERFACE_KPE;


#else /* NOT ESIF_ATTR_OS_WINDOWS */
#ifdef ESIF_ATTR_DEBUG
#define KPE_TRACE_MSG(fmt, ...) printk("[%s@%s#%d]: " fmt, \
	ESIF_FUNC, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define KPE_TRACE_MSG(fmt, ...) 
#endif

enum esif_rc esif_lf_register_driver(struct esif_driver_iface *diPtr);
enum esif_rc esif_lf_unregister_driver(struct esif_driver_iface *diPtr);

#endif /* NOT ESIF_ATTR_OS_WINDOWS */


