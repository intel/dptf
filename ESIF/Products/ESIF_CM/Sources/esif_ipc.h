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
#ifndef _ESIF_IPC_H_
#define _ESIF_IPC_H_

/*
** User Decleration
*/

/*
** Agnostic Decleration
*/
#include "esif.h"
#include "esif_pm.h"
#include "esif_command.h"

#define IPC_DEVICE "ipf_lf"

/* Kernel Debug Trace Selectors */
#define IPC_TRACE_INIT		0	/* Init Debug */
#define IPC_TRACE_DEBUG		1	/* IPC Debug */
#define IPC_TRACE_POLL		2	/* Poll Debug */


/* IOCTLS May There Be Few */
#ifdef ESIF_ATTR_OS_LINUX
#define ESIF_IOCTL_IPC_NOOP _IO('A', 0)
#define ESIF_IOCTL_IPC      _IOWR('A', 1, u8*)
#endif

#ifdef ESIF_ATTR_OS_WINDOWS

#ifdef ESIF_ATTR_USER
/* Need to disable this otherwise RS4 EWDK 17074 causes warning about deprecated memcpy and RtlCopyMemory in Microsoft code (winioctl.h) */
#pragma warning(disable:4995)
#include <winioctl.h>
#pragma warning(default:4995)
#endif

#define ESIF_IPC_CODE(x) CTL_CODE(FILE_DEVICE_NETWORK, x, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define ESIF_IOCTL_IPC_NOOP			ESIF_IPC_CODE(0)
#define ESIF_IOCTL_IPC				ESIF_IPC_CODE(1)
#define ESIF_IOCTL_IPC_IPC_SUSPEND      	ESIF_IPC_CODE(2)
#define ESIF_IOCTL_IPC_IPC_RESUME		ESIF_IPC_CODE(3)
#define ESIF_IOCTL_ENABLE_EVENT_DOORBELL	ESIF_IPC_CODE(4)
#define ESIF_IOCTL_IETM_ACPI_ACCESS_INFO	ESIF_IPC_CODE(5)

#endif

/* IPC Types May There Be Few */
enum esif_ipc_type {
	ESIF_IPC_TYPE_PRIMITIVE,/* ESIF Primitive        */
	ESIF_IPC_TYPE_EVENT,	/* ESIF Event            */
	ESIF_IPC_TYPE_COMMAND,	/* ESIF Command          */
	ESIF_IPC_TYPE_NOOP,	/* No Operation For Test */
	ESIF_IPC_TYPE_MAX
};

/* OS Agnostic ESIF IPC */
#define ESIF_HMAC_LEN    20
#define ESIF_IPC_VERSION 1

/*
 * IPC
 */

/* USE Native Data Types With Packed Structures */
#pragma pack(push, 1)
struct esif_ipc {
	u8  version;				/* Version Of IPC */
#ifdef ESIF_ATTR_HMAC
	u8  hmac[ESIF_HMAC_LEN];/* Optional HMAC Authentication If Any */
#endif
	enum esif_ipc_type  type;	 /* Type Of IPC */
	u32 data_len;			 /* Data Length */
	enum esif_rc        return_code; /* Result Of Primitive Execution */
	/* Data Is Here ... */
};

#if defined(ESIF_ATTR_OS_WINDOWS)
struct esif_ipc_event_enable {
	HANDLE eventDoorbell;
	HANDLE eventLfExit;
};

struct esif_ipc_ietm_access_info {
	size_t size;
	size_t pdoNameOffset;
	size_t pathOffset;
	// PDO name and path data begins here...
};

#endif

#pragma pack(pop)

#ifdef ESIF_ATTR_USER
#if defined(ESIF_ATTR_OS_WINDOWS)
typedef struct esif_ipc_event_enable EsifIpcEventEnable, *EsifIpcEventEnablePtr;
typedef struct esif_ipc_ietm_access_info EsifIpcIetmAccessInfo, *EsifIpcIetmAccessInfoPtr;
#endif
typedef struct esif_ipc EsifIpc, *EsifIpcPtr, **EsifIpcPtrLocation;
#endif

/*
 * IPC Primitive
 */

/* USE Native Data Types With Packed Structures */
#pragma pack(push, 1)
struct esif_ipc_primitive {
	u8   version;		/* Version Of Primitive IPC */
	u32  id;		/* Primitive ID*/
	u16  domain;		/* Primitive Domain e.g. CPU, GFX, Etc.*/
	u8   instance;		/* Instance e.g.  _AC0, _AC1, Etc. */
	u8   src_id;		/* Source Of Primitive */
	u8   dst_id;		/* Dest Of Primitive UCAST, MCAST, BCAST */
	enum esif_rc return_code;	/* Result Of Primitive Execution */
	u16  kern_action;	/* i-th Kernel Action To Run */
	enum esif_action_type action_type;
	u32  payload_len;	/* Size not including this header */
	enum esif_data_type req_data_type;	/* Request Data Type */
	u32  req_data_offset;	/* Start Of Request Data Usually Zero */
	u32  req_data_len;	/* Request Data  May Be Zero No Data */
	enum esif_data_type rsp_data_type;	/* Respond Data Type */
	u32  rsp_data_offset;	/* Respond Data Offset */
	u32  rsp_data_len;	/* Respond Data Length */
	/* Data Is Here ... */
};

#pragma pack(pop)

#ifdef ESIF_ATTR_USER
typedef struct esif_ipc_primitive EsifIpcPrimitive, *EsifIpcPrimitivePtr,
	**EsifPrimitivePtrLoction;
#endif

/* Primitive Source Is Always Upper Framework Or UNICAST */
static ESIF_INLINE esif_string esif_primitive_src_str(u8 src_id)
{
	if (ESIF_INSTANCE_UF == src_id)
		return (esif_string)"ESIF_UF";

	return (esif_string)"UNICAST";
}


/* Privite Destination Can Be UNICAST, MULTICAST Or BROADCAST */
static ESIF_INLINE esif_string esif_primitive_dst_str(u8 dst_id)
{
	if (dst_id < 100)
		return (esif_string)"UNICAST";
	else
		return (esif_string)"MULTICAST";
}


/*
 * IPC Event
 */

/* USE Native Data Types With Packed Structures */
/* Version 2 added data_type */
#pragma pack(push, 1)
struct esif_ipc_event {
	u8   version;		/* Version Of Primitive Event */
	u64  id;		/* Event Transaction ID  */
	u64  timestamp;		/* Event Timestamp In msec */
	enum esif_event_type type; /* Event Type */
	enum esif_event_priority priority;	/* Event Priority */
	u8   src_id;		/* Source Of Event            */
	u8   dst_id;		/* Dest Of Event              */
	u16  dst_domain_id;	/* Destination Domain ID      */
	enum esif_data_type data_type; /* Event data type */
	u32  data_len;		/* Data Length May Be Zero    */
	/* Data Is Here ... */
};

#pragma pack(pop)

/*
 * IPC Command
 */

/* USE Native Data Types With Packed Structures */
#pragma pack(push, 1)
struct esif_ipc_command {
	u8  version;		/* Version Of Primitive Event */
	enum esif_command_type      type;	/* Command Type */
	enum esif_command_priority  priority;	/* Event Priority */
	enum esif_rc         return_code;	/* Result Of Primitive
						   Execution */
	u32                  payload_len;	/* Size not including this header */
	enum esif_data_type  req_data_type;	/* Request Data Type */
	u32                  req_data_offset;	/* Offset after this
						   structure */
	u32                  req_data_len;	/* Request Data Length*/
	enum esif_data_type  rsp_data_type;	/* Request Data Type */
	u32                  rsp_data_offset;	/* Request Data Offset */
	u32                  rsp_data_len;	/* Request Data Length */
	/* Data Is Here ... */
};

#pragma pack(pop)

/* IPC Allocation */

#ifdef __cplusplus
extern "C" {
#endif

struct esif_ipc *esif_ipc_alloc_command(
	struct esif_ipc_command **command_ptr_ptr,
	u32 data_len
	);

struct esif_ipc *esif_ipc_alloc_primitive(
	struct esif_ipc_primitive **primitive_ptr_ptr,
	u32 data_len
	);

struct esif_ipc *esif_ipc_alloc_event(
	struct esif_ipc_event **event_ptr_ptr,
	enum esif_data_type data_type,
	u32 data_len
	);

void esif_ipc_free(struct esif_ipc *ipc_ptr);

esif_os_handle_t esif_ipc_connect(char *session_id);
void esif_ipc_disconnect(esif_os_handle_t handle);
enum esif_rc esif_ipc_execute(esif_os_handle_t handle, struct esif_ipc *ipc_ptr);


/* IPC Connect */
esif_os_handle_t esif_os_ipc_connect(char *session_id);

/* IPC Disconnect */
void esif_os_ipc_disconnect(esif_os_handle_t handle);

/* IPC Execute */
enum esif_rc esif_os_ipc_execute(esif_os_handle_t handle,
				 struct esif_ipc *ipc_ptr);


#ifdef __cplusplus
}
#endif

#ifdef ESIF_ATTR_KERNEL

/*
 * Kernel Decleration
 */

/* Receive IPC */
struct esif_ipc *esif_ipc_process(struct esif_ipc *ipc_ptr);

/* Init / Exit */
enum esif_rc esif_ipc_init(esif_device_t device);
enum esif_rc esif_os_ipc_init(esif_device_t device);

void esif_ipc_exit(esif_device_t device);
void esif_os_ipc_exit(esif_device_t device);

#endif	/* ESIF_ATTR_KERNEL */
#endif	/* _ESIF_IPC_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
