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

#ifndef _ESIF_PARTICIPANT_H_
#define _ESIF_PARTICIPANT_H_

#include "esif.h"
#include "esif_participant_iface.h"
#include "esif_event.h"
#include "esif_domain.h"

#ifdef ESIF_ATTR_KERNEL

#include "esif_dsp.h"
#include "esif_temp.h"
#include "esif_power.h"
#include "esif_primitive.h"

/*
** TODO: Hookup CCB Debug
*/
#define ESIF_PARTICIPANT_INVALID_TYPE 0xFFFFFFFF
#define ESIF_PARTICIPANT_INVALID_UID 0xFFFFFFFF

/* Lower Participant */
#ifndef ESIF_ATTR_OS_LINUX_DRIVER
struct esif_lp {
	u8  instance;			/* Lower Participant Instance */
	char pi_name[ESIF_NAME_LEN];	/* PI Name */
	struct esif_participant_iface *pi_ptr;	/* Particpant INTERFACE */
	u8  enable;			/* Boolean Enable Disable */
	struct esif_lp_dsp *dsp_ptr;			/* DSP */
	/* Number Of Qualifiers For A Participant */
	u8  domain_count;
	/* Domains For Participants */
	struct esif_lp_domain  domains[ESIF_DOMAIN_MAX];

	/* XFORM Temperature */
	enum esif_rc  (*xform_temp)(const enum esif_temperature_type,
				    esif_temp_t *temp,
				    const enum esif_action_type action,
				    const struct esif_lp_dsp *dsp,
				    const struct esif_lp_primitive *primitive_ptr,
				    struct esif_lp *lp_ptr);

	/* XFORM Power */
	enum esif_rc  (*xform_power)(const enum esif_power_unit_type,
				     esif_power_t *power,
				     const enum esif_action_type action,
				     const struct esif_lp_dsp *dsp,
				     const enum esif_primitive_opcode opcode);
};

#endif /* ESIF_ATTR_OS_LINUX_DRIVER */
#endif /* ESIF ATTR_KERNEL          */
#ifdef ESIF_ATTR_USER

#include "esif_uf_app_iface.h"

typedef enum {
	eParticipantOriginLF,
	eParticipantOriginUF
} eEsifParticipantOrigin;

/* Upper Particpant Data.  Everything we know about a participant */
typedef struct _t_EsifUpData {
	/* Common */
	esif_ver_t    fVersion;				/* Version */
	esif_guid_t   fDriverType;			/* Driver Type */
	char          fName[ESIF_NAME_LEN];		/* Friendly Name */
	char          fDesc[ESIF_DESC_LEN];		/* Description */
	char          fDriverName[ESIF_NAME_LEN];	/* Driver Name */
	char          fDeviceName[ESIF_NAME_LEN];	/* Device Name */
	char          fDevicePath[ESIF_PATH_LEN]; /* Device Path
						  * /sys/bus/platform...*/

	enum esif_participant_enum fEnumerator; /* Device Enumerator If Any */
	esif_flags_t  fFlags;				/* Flags If Any */

	/* ACPI */
	char  fAcpiDevice[ESIF_NAME_LEN]; /* Device INT340X */
	char  fAcpiScope[ESIF_SCOPE_LEN]; /* Scope/REGEX e.g. \_SB.PCI0.TPCH */
	UInt32       fAcpiUID;				/* Unique ID If Any */
	eDomainType  fAcpiType;			/* Participant Type If Any */

	/* PCI */
	UInt16  fPciVendor;	/* PCI Vendor e.g. 0x8086 For Intel */
	UInt16  fPciDevice;	/* Device ID Unique To Vendor */
	UInt8   fPciBus;	/* Bus This Device Resides On */
	UInt8   fPciBusDevice;	/* Device Number On Bus */
	UInt8   fPciFunction;	/* Function Of Device */
	UInt8   fPciRevision;	/* Revision Of PCI Hardware Device */
	UInt8   fPciClass;	/* Class 3 bytes: (base class,sub sclass, prog-if) */
	UInt8   fPciSubClass;	/* Sub Class */
	UInt8   fPciProgIf;	/* Program Interface */
} EsifUpData, *EsifUpDataPtr, **EsifUpDataPtrLocation;

/* Upper Participant */
typedef struct _t_EsifUp {
	UInt8  fInstance;	/* Unique Upper Participant Instance */
	UInt8  fLpInstance;	/* Lower Participant Instance */
	void   *fUpHandle;	/* Upper Participant Handle */
	UInt8  fEnabled;	/* Enabled / Disabled */
	eEsifParticipantOrigin  fOrigin;	/* Origin Of Creation */
	struct esif_up_dsp      *fDspPtr;	/* Pointer To Our DSP */
	EsifUpData fMetadata;	/* Participant Data */
} EsifUp, *EsifUpPtr, **EsifUpPtrLocation;

#endif /* ESIF_ATTR_USER */
#endif /* _ESIF_PARTICIPANT_H_ */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
