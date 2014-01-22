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

#ifdef ESIF_ATTR_USER
# define ESIF_TRACE_DEBUG_DISABLED
#endif

#include "esif_ipc.h"
#include "esif_primitive.h"
#include "esif_debug.h"

#ifdef ESIF_ATTR_OS_WINDOWS
/*
 *
 * The Windows banned-API check header must be included after all other headers,
 * or issues can be identified
 * against Windows SDK/DDK included headers which we have no control over.
 *
 */
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#ifdef ESIF_ATTR_KERNEL

#define ESIF_DEBUG_MODULE ESIF_DEBUG_MOD_IPC

/* Debug Logging Defintions */
#define RESERVED0     0	/* Used By OS Version */
#define RESERVED1     1	/* Used By OS Version */
#define RESERVED2     2	/* Used By OS Version */
#define RESERVED4     3	/* Used By OS Version */
#define INIT_DEBUG    4	/* Init Debug */
#define IPC_DEBUG     5	/* IPC Debug */
#define RESERVED20    6	/* Reserved */
#define RESERVED40    7	/* Reserved */

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_IPC, INIT_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_IPC(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_IPC, IPC_DEBUG, format, ##__VA_ARGS__)

/*
 * Kernel Implementation
 */

/*
 ******************************************************************************
 * PUBLIC
 ******************************************************************************
 */

/* Process IPC */
struct esif_ipc *esif_ipc_process(
	struct esif_ipc *ipc_ptr
	)
{
	struct esif_ipc *ipc_ret_ptr = ipc_ptr;
	ESIF_TRACE_DYN_IPC("%s: START ipc %p\n", ESIF_FUNC, ipc_ptr);

	/*
	 * If we got this far we are guaranteed to have a valid IPC
	 * header now we need to check to see if we hav enough data
	 * for the type specified if so process it if not return to
	 * avoid what would surely result in undesired behavior.
	 */

	switch (ipc_ptr->type) {
	/* Command e.g. Get Participants, Etc. */
	case ESIF_IPC_TYPE_COMMAND:
		ESIF_TRACE_DYN_IPC("%s: COMMAND Received\n", ESIF_FUNC);
		if (ipc_ptr->data_len < sizeof(struct esif_ipc_command))
			ipc_ptr->return_code = ESIF_E_IPC_DATA_INVALID;
		else
			ipc_ret_ptr = esif_execute_ipc_command(ipc_ptr);
		break;

	/* Retireve A Signaled Event Or Check Event Queue */
	case ESIF_IPC_TYPE_EVENT:
		ESIF_TRACE_DYN_IPC("%s: EVENT Received\n", ESIF_FUNC);
		if (ipc_ptr->data_len < sizeof(struct esif_ipc_event_header))
			ipc_ptr->return_code = ESIF_E_IPC_DATA_INVALID;
		else
			ipc_ret_ptr = esif_event_queue_pull();
		break;

	/* Execute Primitive e.g. GET_TEMPERATURE */
	case ESIF_IPC_TYPE_PRIMITIVE:
		ESIF_TRACE_DYN_IPC("%s: PRIMITIVE Received\n", ESIF_FUNC);
		if (ipc_ptr->data_len < sizeof(struct esif_ipc_primitive))
			ipc_ptr->return_code = ESIF_E_IPC_DATA_INVALID;
		else
			ipc_ret_ptr = esif_execute_ipc_primitive(ipc_ptr);
		break;

	/* NOOP For Testing */
	case ESIF_IPC_TYPE_NOOP:
		ESIF_TRACE_DYN_IPC("%s: NOOP Received\n", ESIF_FUNC);
		ipc_ret_ptr = NULL;
		break;

	/* Unsupported or Unknown IPC Type Received */
	default:
		ESIF_TRACE_DYN_IPC("%s: Unknown IPC Type Received type=%u\n",
				   ESIF_FUNC,
				   ipc_ptr->type);
		ipc_ptr->return_code = ESIF_E_IPC_DATA_INVALID;
		break;
	}
	ESIF_TRACE_DYN_IPC("%s: FINISH return result: %s(%u)\n",
			   ESIF_FUNC,
			   esif_rc_str(ipc_ptr->return_code),
			   ipc_ptr->return_code);
	return ipc_ret_ptr;
}


/* Init */
enum esif_rc esif_ipc_init(
	esif_device_t device
	)
{
	ESIF_TRACE_DYN_INIT("%s: Initialize IPC\n", ESIF_FUNC);

	return esif_os_ipc_init(device);
}


/* Exit */
void esif_ipc_exit(
	esif_device_t device
	)
{
	esif_os_ipc_exit(device);

	ESIF_TRACE_DYN_INIT("%s: Exit IPC\n", ESIF_FUNC);
}


#endif /* ESIF_ATTR_KERNEL */
#ifdef ESIF_ATTR_USER

/*
** User Implementation
*/


#ifdef __cplusplus
extern "C" {
#endif

/* IPC Connect */
esif_handle_t esif_ipc_connect(
	esif_string session_id
	)
{
	ESIF_TRACE_DEBUG("%s: IPC session_id = %s\n",
			 ESIF_FUNC, session_id);
	return esif_os_ipc_connect(session_id);
}


/* IPC Disconnect */
void esif_ipc_disconnect(
	esif_handle_t handle
	)
{
	ESIF_TRACE_DEBUG("%s: IPC handle = %d\n",
			 ESIF_FUNC, handle);
	esif_os_ipc_disconnect(handle);
}


/* IPC Execute */
enum esif_rc esif_ipc_execute(
	esif_handle_t handle,
	struct esif_ipc *ipc_ptr
	)
{
	ESIF_TRACE_DEBUG("%s: handle = %d, IPC = %p\n",
			 ESIF_FUNC, handle, ipc_ptr);
	return esif_os_ipc_execute(handle, ipc_ptr);
}


#endif /* ESIF_ATTR_USER */


/*
** Agnostic Implementation
*/

/* Allocate IPC */
struct esif_ipc *esif_ipc_alloc(
	enum esif_ipc_type type,
	u32 data_len
	)
{
	u32 ipc_size = data_len + sizeof(struct esif_ipc);
	struct esif_ipc *ipc_ptr = (struct esif_ipc *)esif_ccb_malloc(ipc_size);
	if (NULL == ipc_ptr)
		return NULL;

	ipc_ptr->version     = ESIF_IPC_VERSION;
	ipc_ptr->type        = type;
	ipc_ptr->data_len    = data_len;
	ipc_ptr->return_code = ESIF_OK;

#ifdef ESIF_ATTR_HMAC
	esif_ccb_memcpy(ipc->hmac, x, ESIF_HMAC_LEN);
#endif
	ESIF_TRACE_DEBUG("%s: ipc = %p, type = %d, size = %d data_len = %d\n",
			 ESIF_FUNC, ipc_ptr, type, (int)ipc_size,
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

	ipc_ptr = esif_ipc_alloc(ESIF_IPC_TYPE_COMMAND,
				 data_len + sizeof(struct esif_ipc_command));

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
	ipc_ptr = esif_ipc_alloc(ESIF_IPC_TYPE_PRIMITIVE,
				 data_len + sizeof(struct esif_ipc_primitive));

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


/* Free IPC */
void esif_ipc_free(struct esif_ipc *ipc_ptr)
{
	ESIF_TRACE_DEBUG("%s: ipc = %p\n", ESIF_FUNC, ipc_ptr);
	esif_ccb_free(ipc_ptr);
}


#ifdef ESIF_ATTR_USER
#ifdef __cplusplus
}
#endif
#endif

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
