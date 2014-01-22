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

#ifndef _ESIF_CPC_H_
#define _ESIF_CPC_H_

#include "esif.h"
#include "esif_primitive.h"
#include "esif_event.h"

/*--- CPC ---*/

/* Must Be Aligned With DSP's dsp_info.h: cpc_descriptor{} */
struct esif_cpc_descriptor {
	u32  signature;	/* %CPC                      */
	u8   version;	/* Version Of Data Structure */
	esif_flags_t  flags;		/* Flags If Any  */
	esif_flags_t  capability;	/* Capability  */
};

/* Must Be Aligned With DSP's dsp_info.h short_dsp_desriptor{} */
struct esif_cpc_header {
	struct esif_cpc_descriptor  cpc;	/* CPC Or Other */
	u8    version;		/* Version Of Data Structure */
	char  code[12 + 1];	/* Code Name For DSP         */
	u8    ver_major;	/* Content Major Version 1.x */
	u8    ver_minor;	/* Content Minor Version x.1 */
	esif_flags_t flags;	/* Flags If Any              */
};

/* Must Be Aligned With DSP's cpc.h: cpc{} */
struct esif_lp_cpc {
	u32  size;
	struct esif_cpc_header header;
	u32  number_of_algorithms;
	u32  number_of_basic_primitives;
	u32  number_of_domains;
	u32  number_of_events;
	/* primitives are laid down here */
	/* algorithms come after the primitives */
	/* domains come after algorithms */
	/* events are laid after domain */
};

/* CPC Primitive */
/* Must Be Aligned With DSP's basic_primitive.h: basic_primitive{} */
struct esif_cpc_primitive {
	u32  size;
	struct esif_primitive_tuple  tuple;
	enum esif_primitive_opcode   operation; /* ESIF_PRIMITIVE_OP_GET */
	u32  number_of_actions;
	/* actions are laid down here */
};

/* CPC Action */
/* Must Be Aligned With DSP's basic_action.h: basic_action{} */
struct esif_cpc_action {
	enum esif_action_type  type;			/* Example:
							 *ESIF_ACTION_ACPI    */
	u8   num_valid_params;	/* Number Of Valid Parameters   */
	u8   param_valid[5];	/* Array Of Valid Parameters    */
	u32  param[5];		/* _TMP, NULL, NULL, NULL, NULL */
};

/* CPC Algorithm */
/* Must Be Aligned With DSP's algorithm.h: algorithm{} */
struct esif_cpc_algorithm {
	enum esif_action_type  action_type;
	u32  temp_xform;
	u32  tempC1;
	u32  tempC2;
	u32  power_xform;
	u32  time_xform;
	u32  size;
};

#define MAX_NAME_STRING_LENGTH 32
#define MAX_DOMAIN_TYPE_STRING_LENGTH ESIF_GUID_LEN
#define MAX_DESCRIPTION_STRING_LENGTH 128

/* CPC Event */
/* Must Be Aligned With DSP's event.h: event{} */
/* Maps a Event_ID to ESIF Event ENUM OR UUID Based */
struct esif_cpc_event {
	char  name[MAX_NAME_STRING_LENGTH];
	u8    event_key[ESIF_GUID_LEN];			/* Event ID */
	enum esif_event_type esif_event;	/* ESIF Event */
	u8    event_guid[ESIF_GUID_LEN];	/* Event GUID */
	enum esif_event_group  esif_group;	/* ESIF Event Group */
	enum esif_data_type    esif_group_data_type;
};

struct domain_descriptor {
	char  name[MAX_NAME_STRING_LENGTH];
	char  description[MAX_DESCRIPTION_STRING_LENGTH];
	u16   qualifier;
	u8    guid[MAX_DOMAIN_TYPE_STRING_LENGTH];
	u32   priority;
	enum  esif_domain_type domainType;
};


struct capability {
	esif_flags_t  capability_flags;
	u8  number_of_capability_flags;
	u8  reserved[3];
	u8  capability_mask[32];
};

struct domain {
	u32  size;
	struct domain_descriptor descriptor;
	u32  number_of_primitives;
	struct capability capability_for_domain;
};

/* Unpack CPC Data Into DSP */
struct esif_lp_dsp;
enum esif_rc esif_cpc_unpack(struct esif_lp_dsp *dsp_ptr,
			     const struct esif_data *cpc_ptr);

/* Free CPC Data From DSP */
void esif_cpc_free(struct esif_lp_dsp *dsp_ptr);

/* Init / Exit */
enum esif_rc esif_cpc_init(void);
void esif_cpc_exit(void);

#include "esif_dsp.h"

#endif /* _ESIF_CPC_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
