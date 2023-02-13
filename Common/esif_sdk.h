/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

/*
 * OS and Kernel/User Agnostic Types
 */

/* Data Lengths */
#define ESIF_NAME_LEN		64	/* Maximum Name Length        */
#define ESIF_DESC_LEN		64	/* Maximum Description Length */
#define ESIF_SCOPE_LEN		64	/* Maximum ACPI Scope Length  */
#define ESIF_OBJ_LEN		64	/* Maximum Object Name Length */
#define ESIF_GUID_LEN		16	/* Length of a GUID In Bytes  */
#define ESIF_PATH_LEN		128	/* Maximum Path Length        */
#define ESIF_LIBPATH_LEN	128	/* Maximum Lib Path Length    */
#define ESIF_ACPI_UID_LEN	64	/* Maximum ACPI UID Length  */
#define ESIF_ACPI_NAME_LEN	5	/* Maximum ACPI Device Name Length */
#define ESIF_IPADDR_LEN		20	/* Maximum IP Address Length */
#define ESIF_VERSION_LEN	20	/* Maximum Version String Length */

/* Opaque Types */
typedef u32 esif_flags_t;                   /* FLAGS        */
typedef u8  esif_guid_t[ESIF_GUID_LEN];     /* GUID         */
typedef u8  esif_ver_t;                     /* Version      */

/* Temperature Thresholds */
#define ESIF_SDK_MAX_AUX_TRIP 199	/* Celsius */
#define ESIF_SDK_MIN_AUX_TRIP -136	/* Celsius */

/* Construct/Extract Version Header Major.Minor.Revision Number as a UInt32 */
#define ESIFHDR_VERSION(major, minor, revision)	((UInt32)((((major) & 0xFF) << 24) | (((minor) & 0xFF) << 16) | ((revision) & 0xFFFF)))
#define ESIFHDR_GET_MAJOR(version)				((UInt32)(((version) >> 24) & 0xFF))
#define ESIFHDR_GET_MINOR(version)				((UInt32)(((version) >> 16) & 0xFF))
#define ESIFHDR_GET_REVISION(version)			((UInt32)((version) & 0xFFFF))

/* Participant Instances */
#define ESIF_INSTANCE_LF        0	/* Reserved For ESIF Primary Participant */
#define ESIF_INSTANCE_INVALID	255

/* Shared Parameters */
#define ESIF_MAX_CLIENTS			16						/* Maximum IPF Clients */
#define ESIF_MAX_PLUGINS			4						/* Additional In-Process ESIF Apps (IPFSRV, DPTF, IPFTSC, Plus 1) */
#define ESIF_MAX_APPS				(ESIF_MAX_CLIENTS + ESIF_MAX_PLUGINS)	/* Maximum ESIF Apps (IPF Clients + IPF_UF Plugins) */

/* Primitive Instance */
#define ESIF_INSTANCE_NO_PERSIST	254


#pragma pack(push, 1)

/*
* Header structure for events broadcast to all applications
* The UUID is expected to specify the type of the data contents
* The UUID also acts as a version field, as any change in the data
* content should be reflected in a change in the UUID for the data
*/
#define ESIF_APP_BROADCAST_LEN_MAX 0x400000

typedef struct esif_app_broadcast_header_s {
	esif_guid_t UUID; /* UUID of the contained data type */
	u32 dataLen; /* Length of the data which follows (not including header) */
	/* Data follows here -  Maximum length ESIF_APP_BROADCAST_LEN_MAX */
} EsifAppBroadcastHeader;

#pragma pack(pop)

