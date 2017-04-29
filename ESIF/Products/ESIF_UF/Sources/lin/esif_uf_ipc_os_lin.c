/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
**
** Licensed under the Apache License, Version 2.0 (the "License"); you may not
** use this file except in compliance with the License.
**
** You may obtain a copy of the License at
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
** WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**
** See the License for the specific language governing permissions and
** limitations under the License.
**
******************************************************************************/
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_LINUX

#define ESIF_TRACE_DEBUG_DISABLED
#include "esif.h"
#include "esif_ipc.h"

/* IPC OS Connect */
esif_handle_t esif_os_ipc_connect(char *session_id)
{
	int fd = 0;
	char device[MAX_PATH] = { 0 };

	esif_ccb_sprintf(sizeof(device), device, "/dev/%s", IPC_DEVICE);
	fd = open(device, O_RDWR);

	ESIF_TRACE_DEBUG("linux_%s: session_id=%s device=%s handle=%d\n",
			 __func__, session_id, device, fd);
	return fd;
}


/* IPC OS Disconnect */
void esif_os_ipc_disconnect(esif_handle_t handle)
{
	ESIF_TRACE_DEBUG("linux_%s: IPC handle = %d\n", __func__, handle);
	close(handle);
}


/* IPC OS Execution */
enum esif_rc esif_os_ipc_execute(
	esif_handle_t handle,
	struct esif_ipc *ipc_ptr
	)
{
	int rc = 0;

	ESIF_TRACE_DEBUG("linux_%s: handle = %d, IPC = %p\n",
			 __func__,
			 handle,
			 ipc_ptr);

	/* use IOCTL or read here */
#ifdef ESIF_ATTR_OS_ANDROID
	rc = read(handle, ipc_ptr, ipc_ptr->data_len + sizeof(struct esif_ipc));
	ESIF_TRACE_DEBUG("linux_%s: READ handle = %d, IPC = %p rc = %d\n",
			 __func__, handle, ipc_ptr, rc);
#else
	rc = ioctl(handle, ESIF_IOCTL_IPC, ipc_ptr);
	ESIF_TRACE_DEBUG("linux_%s: IOCTL handle = %d, IPC = %p rc = %d\n",
			 __func__, handle, ipc_ptr, rc);
	if (rc)
		return ESIF_E_UNSPECIFIED;
#endif

	return ESIF_OK;
}
