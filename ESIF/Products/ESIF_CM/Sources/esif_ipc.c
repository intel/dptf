/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

# define ESIF_TRACE_ID	ESIF_TRACEMODULE_IPC

#include "esif_ipc.h"
#include "esif_primitive.h"
#include "esif_event.h"
#include "esif_trace.h"
#include "esif_ccb_time.h"



/*
** User Implementation
*/

/* IPC Connect */
esif_os_handle_t esif_ipc_connect(
	char *session_id
	)
{
	ESIF_TRACE_DEBUG("IPC session_id = %s\n", session_id);
	return esif_os_ipc_connect(session_id);
}


/* IPC Disconnect */
void esif_ipc_disconnect(
	esif_os_handle_t handle
	)
{
	ESIF_TRACE_DEBUG("IPC handle = " OS_HANDLE_FMT "\n", esif_ccb_os_handle2llu(handle));
	esif_os_ipc_disconnect(handle);
}


/* IPC Execute */
enum esif_rc esif_ipc_execute(
	esif_os_handle_t handle,
	struct esif_ipc *ipc_ptr
	)
{
	return esif_os_ipc_execute(handle, ipc_ptr);
}




/*
** Agnostic Implementation
*/

/* Allocate IPC */
static struct esif_ipc *esif_ipc_alloc(
	enum esif_ipc_type type,
	u32 data_len
	)
{
	struct esif_ipc *ipc_ptr = NULL;
	u32 ipc_size = sizeof(*ipc_ptr) + data_len;

	ipc_ptr = (struct esif_ipc *)esif_ccb_malloc(ipc_size);
	if (NULL == ipc_ptr)
		return NULL;

	ipc_ptr->version     = ESIF_IPC_VERSION;
	ipc_ptr->type        = type;
	ipc_ptr->data_len    = data_len;
	ipc_ptr->return_code = ESIF_OK;

	ESIF_TRACE_VERBOSE("IPC = %p, type = %d, size = %d data_len = %d\n",
		ipc_ptr, type,
		(int)ipc_size,
		(int)data_len);

	return ipc_ptr;
}


/* Allocate Command IPC */
struct esif_ipc *esif_ipc_alloc_command(
	struct esif_ipc_command **command_ptr_ptr,
	u32 data_len
	)
{
	struct esif_ipc *ipc_ptr = NULL;

	ESIF_ASSERT(command_ptr_ptr != NULL);

	ipc_ptr = esif_ipc_alloc(ESIF_IPC_TYPE_COMMAND,
		data_len + sizeof(**command_ptr_ptr));

	if (NULL == ipc_ptr) {
		*command_ptr_ptr = NULL;
	} else {
		struct esif_ipc_command *command_ptr = NULL;
		command_ptr = (struct esif_ipc_command *)(ipc_ptr + 1);

		command_ptr->version     = ESIF_COMMAND_VERSION;
		command_ptr->priority    = ESIF_COMMAND_PRIORITY_NORMAL;
		command_ptr->payload_len = data_len;
		*command_ptr_ptr         = command_ptr;
	}
	return ipc_ptr;
}


/* Allocate Primitive IPC */
struct esif_ipc *esif_ipc_alloc_primitive(
	struct esif_ipc_primitive **primitive_ptr_ptr,
	u32 data_len
	)
{
	struct esif_ipc *ipc_ptr = NULL;

	ESIF_ASSERT(primitive_ptr_ptr != NULL);

	ipc_ptr = esif_ipc_alloc(ESIF_IPC_TYPE_PRIMITIVE,
		data_len + sizeof(**primitive_ptr_ptr));

	if (NULL == ipc_ptr) {
		*primitive_ptr_ptr = NULL;
	} else {
		struct esif_ipc_primitive *primitive_ptr = NULL;
		primitive_ptr =	(struct esif_ipc_primitive *)(ipc_ptr + 1);

		primitive_ptr->version     = ESIF_PRIMITIVE_VERSION;
		primitive_ptr->payload_len = data_len;
		*primitive_ptr_ptr         = primitive_ptr;
	}
	return ipc_ptr;
}


/* Allocate Event IPC */
struct esif_ipc *esif_ipc_alloc_event(
	struct esif_ipc_event **event_ptr_ptr,
	enum esif_data_type data_type,
	u32 data_len
	)
{
	struct esif_ipc *ipc_ptr = NULL;

	ESIF_ASSERT(event_ptr_ptr != NULL);

	ipc_ptr = esif_ipc_alloc(ESIF_IPC_TYPE_EVENT,
		data_len + sizeof(**event_ptr_ptr));

	if (NULL == ipc_ptr) {
		*event_ptr_ptr = NULL;
	} else {
		esif_ccb_time_t timestamp = {0};
		struct esif_ipc_event *event_ptr = NULL;
		event_ptr = (struct esif_ipc_event *)(ipc_ptr + 1);

		event_ptr->version  = ESIF_EVENT_VERSION;
		event_ptr->priority = ESIF_EVENT_PRIORITY_NORMAL;

		event_ptr->data_type = data_type;
		event_ptr->data_len = data_len;

		esif_ccb_system_time(&timestamp);
		event_ptr->timestamp = (u64)timestamp;

		*event_ptr_ptr = event_ptr;
	}
	return ipc_ptr;
}

	
/* Free IPC */
void esif_ipc_free(struct esif_ipc *ipc_ptr)
{
	esif_ccb_free(ipc_ptr);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
