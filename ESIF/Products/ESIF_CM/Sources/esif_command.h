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
#ifndef _ESIF_COMMAND_H_
#define _ESIF_COMMAND_H_

#include "esif_ccb.h"
#include "esif_sdk.h"
#include "esif_pm.h"
#include "esif_dsp.h"

#define ESIF_COMMAND_VERSION 0x1

/* Command Kernel Trace Selectors */
#define CMD_TRACE_INIT		0	/* Init Debug */
#define CMD_TRACE_DEBUG		1	/* Command Debug */
#define CMD_TRACE_DECODE	2	/* Decode Debug */


/* Command Priority */
enum esif_command_priority {
	ESIF_COMMAND_PRIORITY_NORMAL = 0,
};

/* Command Priority String */
static ESIF_INLINE esif_string esif_command_priority_str(
	enum esif_command_priority priority)
{
	switch (priority) {
	ESIF_CASE_ENUM(ESIF_COMMAND_PRIORITY_NORMAL);
	}
	return ESIF_NOT_AVAILABLE;
}


/* Command Types */
enum esif_command_type {
	ESIF_COMMAND_TYPE_GET_DEBUG_MODULES = 0,
	ESIF_COMMAND_TYPE_SET_DEBUG_MODULES,
	ESIF_COMMAND_TYPE_GET_DEBUG_MODULE_LEVEL,
	ESIF_COMMAND_TYPE_SET_DEBUG_MODULE_LEVEL,
	ESIF_COMMAND_TYPE_GET_KERNEL_INFO,
	ESIF_COMMAND_TYPE_GET_PARTICIPANTS,
	ESIF_COMMAND_TYPE_GET_PARTICIPANT_DETAIL,
	ESIF_COMMAND_TYPE_GET_MEMORY_STATS,
	ESIF_COMMAND_TYPE_GET_DRIVERS,
	ESIF_COMMAND_TYPE_GET_ACTIONS,
	ESIF_COMMAND_TYPE_SEND_KPE_EVENT,
	ESIF_COMMAND_TYPE_SEND_DSP,
	ESIF_COMMAND_TYPE_PARTICIPANT_CREATE,
	ESIF_COMMAND_TYPE_PARTICIPANT_DESTROY,
};

static ESIF_INLINE esif_string esif_command_type_str(
	enum esif_command_type type)
{
	switch (type) {
	ESIF_CASE_ENUM(ESIF_COMMAND_TYPE_GET_DEBUG_MODULE_LEVEL);
	ESIF_CASE_ENUM(ESIF_COMMAND_TYPE_GET_DEBUG_MODULES);
	ESIF_CASE_ENUM(ESIF_COMMAND_TYPE_SET_DEBUG_MODULE_LEVEL);
	ESIF_CASE_ENUM(ESIF_COMMAND_TYPE_SET_DEBUG_MODULES);
	ESIF_CASE_ENUM(ESIF_COMMAND_TYPE_GET_KERNEL_INFO);
	ESIF_CASE_ENUM(ESIF_COMMAND_TYPE_GET_PARTICIPANTS);
	ESIF_CASE_ENUM(ESIF_COMMAND_TYPE_GET_PARTICIPANT_DETAIL);
	ESIF_CASE_ENUM(ESIF_COMMAND_TYPE_GET_MEMORY_STATS);
	ESIF_CASE_ENUM(ESIF_COMMAND_TYPE_GET_DRIVERS);
	ESIF_CASE_ENUM(ESIF_COMMAND_TYPE_GET_ACTIONS);
	ESIF_CASE_ENUM(ESIF_COMMAND_TYPE_SEND_KPE_EVENT);
	ESIF_CASE_ENUM(ESIF_COMMAND_TYPE_SEND_DSP);
	ESIF_CASE_ENUM(ESIF_COMMAND_TYPE_PARTICIPANT_CREATE);
	ESIF_CASE_ENUM(ESIF_COMMAND_TYPE_PARTICIPANT_DESTROY);
	}
	return ESIF_NOT_AVAILABLE;
}


/* Packaed Command Data structures */

#pragma pack(push, 1)
struct esif_command_get_kernel_info {
	char  ver_str[ESIF_NAME_LEN];
};

struct esif_command_debug_modules {
	u32  modules;
};

struct esif_command_get_debug_module_level {
	u32  modules;
	u32  levels[32];
	u32  tracelevel;
};

struct esif_command_set_debug_module_level {
	u32  module;
	u32  level;
};

struct esif_part_info {
	u32   id;
	u32   version;
	u32   state;
	u32   bus_enum;
	char  name[ESIF_NAME_LEN];
	char  desc[ESIF_DESC_LEN];
	u8    class_guid[ESIF_GUID_LEN];	/* Class GUID  */
	char  dsp_code[12 + 1];
	u8    dsp_ver_major;
	u8    dsp_ver_minor;
};

struct esif_command_get_participants {
	u32  count;
	struct esif_part_info participant_info[MAX_PARTICIPANT_ENTRY];
};

struct esif_command_get_part_detail {
	/* Participant Info */
	u32   id;			/* Participant ID */
	u8    version; /* Participant Version */
	u8    enumerator; /* Participant Enumerator */
	char  name[ESIF_NAME_LEN]; /* Participant Display Name */
	char  desc[ESIF_DESC_LEN]; /* Participant Description */
	char  driver_name[ESIF_NAME_LEN]; /* Participant Driver Name */
	char  device_name[ESIF_NAME_LEN]; /* Participant Device Name */
	char  device_path[ESIF_PATH_LEN]; /* Participant Device Path */
	u8    class_guid[ESIF_GUID_LEN]; /* Participant Class GUID */
	u32   flags; /* Participant Flags If Any */
	u32   capability; /* Participant Static Capability*/

	/* ACPI */
	char  acpi_device[ESIF_SCOPE_LEN];	/* Device e.g. INT3400 */
	char  acpi_scope[ESIF_SCOPE_LEN];/* Scope/REGEX e.g. \_SB.PCI0.TPCH */
	char  acpi_uid[ESIF_ACPI_UID_LEN];	/* Unique ID If Any */
	u32   acpi_type; /* Participant Type If Any */

	/* PCI */
	u16  pci_vendor;		/* PCI Vendor For PCI Devices */
	u16  pci_device;		/* PCE Device For PCI Devices */
	u8   pci_bus;			/* Bus Device Was Enumerated On */
	u8   pci_bus_device;	/* Device Number On Bus */
	u8   pci_function;		/* PCI Function Of Device */
	u8   pci_revision;			/* PCI Hardware Revision */
	u8   pci_class;				/* PCI Hardware Class */
	u8   pci_sub_class;			/* PCI Hardware Sub Class */
	u8   pci_prog_if;			/* PCI Hardware Iface */

	/* LP */
	u32  state;
	u32  timer_period;

	/* DSP */
	u32   have_dsp;
	char  dsp_code[ESIF_DSP_NAME_LEN];
	u8    dsp_ver_major;
	u8    dsp_ver_minor;

	/* CPC */
	u32  have_cpc;
	u8   cpc_version;
	u32  cpc_signature;
	u32  cpc_size;
	u32  cpc_primitive_count;
};

struct esif_mempool_stat {
	u32   pool_tag;
	u32   object_size;
	char  name[ESIF_NAME_LEN];
	u32   alloc_count;
	u32   free_count;
};

struct esif_command_get_memory_stats {
	struct esif_mempool_stat  mempool_stat[ESIF_MEMPOOL_TYPE_MAX];
	struct esif_memory_stats  stats;
};

struct esif_driver_info {
	enum esif_action_type action_type; /* Action exported by the KPE */
	esif_ver_t version;    /* Interface version */
	esif_guid_t class_guid;/* KPE class GUID */
	esif_flags_t flags;

	char name[ESIF_NAME_LEN]; /* KPE name */
	char desc[ESIF_DESC_LEN]; /* KPE description */

	char driver_name[ESIF_NAME_LEN]; /* Driver name */
	char device_name[ESIF_NAME_LEN]; /* Driver device description */
};

struct esif_action_info {
	enum esif_action_type action_type;
	u8 dynamic_action; /* TRUE if Dynamic action type */

};

struct esif_command_get_drivers {
	u32 available_count;
	u32 returned_count;
	/* Array size depends on the number of drivers */
	struct esif_driver_info driver_info[1];
};

struct esif_command_get_actions {
	u32 available_count;
	u32 returned_count;
	/* Array size depends on the number of actions */
	struct esif_action_info action_info[1];
};

struct esif_command_send_kpe_event {
	u32 instance;
	enum esif_event_type event_type;
	u8 data_present;
	u32 data;
};

struct esif_command_send_dsp {
	u32 id;	/* Participant ID */
	u32 data_len; /* Length of data (not including this header) */
	/* Data Is Here ... */
};

struct esif_command_participant_create {
	struct esif_ipc_event_data_create_participant creation_data;
};

struct esif_command_participant_destroy {
	char name[ESIF_NAME_LEN];
};

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

struct esif_ipc_command;
void esif_execute_ipc_command(struct esif_ipc_command *cmd_ptr);

enum esif_rc esif_command_init(void);
void esif_command_exit(void);

#ifdef ESIF_ATTR_KERNEL
u32 esif_ipc_command_get_data_len(struct esif_ipc_command *cmd_ptr);
#endif

#ifdef __cplusplus
}
#endif

#endif	/* _ESIF_COMMAND_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

