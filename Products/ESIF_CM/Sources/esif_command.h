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
#ifndef _ESIF_COMMAND_H_
#define _ESIF_COMMAND_H_

#define ESIF_COMMAND_VERSION 0x1

/* Command Priority */
enum esif_command_priority {
	ESIF_COMMAND_PRIORITY_NORMAL = 0,
};

/* Command Priority String */
static ESIF_INLINE esif_string esif_command_priority_str(
	enum esif_command_priority priority)
{
	#define CREATE_COMMAND_PRIORITY(cp, str) case cp: str = (esif_string) #cp; break;

	esif_string str = (esif_string)ESIF_NOT_AVAILABLE;

	switch (priority) {
		CREATE_COMMAND_PRIORITY(ESIF_COMMAND_PRIORITY_NORMAL, str);
	}
	return str;
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
};

static ESIF_INLINE esif_string esif_command_type_str(
	enum esif_command_type type)
{
	#define CREATE_COMMAND_TYPE(ct, str) case ct: str = (esif_string) #ct; break;

	esif_string str = (esif_string)ESIF_NOT_AVAILABLE;

	switch (type) {
		CREATE_COMMAND_TYPE(ESIF_COMMAND_TYPE_GET_DEBUG_MODULE_LEVEL, str)
		CREATE_COMMAND_TYPE(ESIF_COMMAND_TYPE_GET_DEBUG_MODULES, str)
		CREATE_COMMAND_TYPE(ESIF_COMMAND_TYPE_SET_DEBUG_MODULE_LEVEL, str)
		CREATE_COMMAND_TYPE(ESIF_COMMAND_TYPE_SET_DEBUG_MODULES, str)
		CREATE_COMMAND_TYPE(ESIF_COMMAND_TYPE_GET_KERNEL_INFO, str)
		CREATE_COMMAND_TYPE(ESIF_COMMAND_TYPE_GET_PARTICIPANTS, str)
		CREATE_COMMAND_TYPE(ESIF_COMMAND_TYPE_GET_PARTICIPANT_DETAIL, str)
		CREATE_COMMAND_TYPE(ESIF_COMMAND_TYPE_GET_MEMORY_STATS, str)
	}
	return str;
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
};

struct esif_command_set_debug_module_level {
	u32  module;
	u32  level;
};

struct esif_command_get_participant {
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
	struct esif_command_get_participant participant_info[20];
};

struct esif_command_get_participant_detail {
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
	char  acpi_scope[ESIF_SCOPE_LEN]; /* Scope/REGEX e.g. \_SB.PCI0.TPCH */
	u32   acpi_uid; /* Unique ID If Any */
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
	char  dsp_code[12 + 1];
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

struct esif_memtype_stat {
	u32   pool_tag;
	char  name[ESIF_NAME_LEN];
	u32   alloc_count;
	u32   free_count;
};

struct esif_command_get_memory_stats {
	struct esif_mempool_stat  mempool_stat[ESIF_MEMPOOL_TYPE_MAX];
	struct esif_memtype_stat  memtype_stat[ESIF_MEMTYPE_TYPE_MAX];
	struct esif_memory_stats  stats;
};

#pragma pack(pop)

struct esif_ipc *esif_execute_ipc_command (struct esif_ipc *ipc_ptr);

enum esif_rc esif_command_init (void);
void esif_command_exit (void);

#endif	/* _ESIF_COMMAND_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

