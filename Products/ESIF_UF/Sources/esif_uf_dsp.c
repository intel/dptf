/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_DSP

#include "esif_uf.h"	/* Upper Framework */
#include "esif_ipc.h"	/* IPC Abstraction */
#include "esif_uf_ipc.h"/* IPC */
#include "esif_dsp.h"	/* EDP Definition */
#include "esif_uf_cfgmgr.h"

#include "esif_lib_datavault.h"
#include "esif_lib_esifdata.h"

// Limits
#define MAX_EDP_SIZE    0x7fffffff

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

// Send DSP
int esif_send_dsp(
	char *filename,
	u8 dst_id
	)
{
	int edp_size = 512;
	struct esif_ipc_primitive *primitive_ptr = NULL;
	struct esif_ipc *ipc_ptr = NULL;
	int rc = 0;
	struct edp_dir edp_dir;
	size_t r_bytes;
	char *edp_name        = 0;
	IOStreamPtr io_ptr    = IOStream_Create();
	EsifDataPtr nameSpace = 0;
	EsifDataPtr key       = 0;
	EsifDataPtr value     = 0;

	if (io_ptr == NULL) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	//
	// If we have a filename provided use the contents of the file as the CPC
	// note this is opaque it is up to the receiver to verify that this is in fact
	// a CPC.
	//

	if (NULL == filename) {
		ESIF_TRACE_DEBUG("%s: filename is null\n", ESIF_FUNC);
		rc = -1;
		goto exit;
	}

	// Use name portion of filename for the DataVault key (C:\path\file.edp = file.edp)
	edp_name  = strrchr(filename, *ESIF_PATH_SEP);
	edp_name  = (edp_name ? ++edp_name : filename);
	nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, ESIF_DSP_NAMESPACE, 0, ESIFAUTOLEN);
	key = EsifData_CreateAs(ESIF_DATA_STRING, edp_name, 0, ESIFAUTOLEN);
	value     = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);

	// Look for EDP file either on disk or in a DataVault (static or file), depending on priority setting
	if ((ESIF_EDP_DV_PRIORITY == 1 || !esif_ccb_file_exists(filename)) && EsifConfigGet(nameSpace, key, value) == ESIF_OK) {
		filename = edp_name;
		IOStream_SetMemory(io_ptr, (BytePtr)value->buf_ptr, value->data_len);
	} else {
		IOStream_SetFile(io_ptr, filename, (char *)"rb");
	}

	/* FIND CPC within EDP file */
	if (IOStream_Open(io_ptr) == 0) {
		r_bytes  = IOStream_Read(io_ptr, &edp_dir, sizeof(struct edp_dir));
		edp_size = edp_dir.fpc_offset - edp_dir.cpc_offset;
		IOStream_Seek(io_ptr, edp_dir.cpc_offset, SEEK_SET);
	} else {
		ESIF_TRACE_DEBUG("%s: file not found (%s)\n", ESIF_FUNC, filename);
		rc = -1;
		goto exit;
	}
	if (edp_size > MAX_EDP_SIZE) {
		rc = -1;
		goto exit;
	}

	ipc_ptr = esif_ipc_alloc_primitive(&primitive_ptr, edp_size);
	if (NULL == ipc_ptr || NULL == primitive_ptr) {
		rc = -1;
		goto exit;
	}

	r_bytes = IOStream_Read(io_ptr, (primitive_ptr + 1), edp_size);
	ESIF_TRACE_DEBUG("%s: loaded file %s bytes %d\n", ESIF_FUNC, filename, (int)r_bytes, edp_size);

	primitive_ptr->id       = 200;
	primitive_ptr->domain   = 0;
	primitive_ptr->instance = 0;
	primitive_ptr->src_id   = ESIF_INSTANCE_UF;
	primitive_ptr->dst_id   = dst_id;
	primitive_ptr->req_data_type   = ESIF_DATA_DSP;
	primitive_ptr->req_data_offset = 0;
	primitive_ptr->req_data_len    = edp_size;
	primitive_ptr->rsp_data_type   = ESIF_DATA_VOID;
	primitive_ptr->rsp_data_offset = 0;
	primitive_ptr->rsp_data_len    = 0;

	ESIF_TRACE_DEBUG("%s: CPC file %s(%d) sent to participant %d\n", ESIF_FUNC, filename, edp_size, dst_id);

	ipc_execute(ipc_ptr);

	if (ESIF_OK != ipc_ptr->return_code) {
		ESIF_TRACE_ERROR("ipc error code = %s(%d)\n",
			   esif_rc_str(ipc_ptr->return_code), ipc_ptr->return_code);
		rc = -1;
		goto exit;
	}

	if (ESIF_OK != primitive_ptr->return_code) {
		ESIF_TRACE_ERROR("primitive error code = %s(%d)\n",
			   esif_rc_str(primitive_ptr->return_code), primitive_ptr->return_code);
		rc = -1;
		goto exit;
	}

	ESIF_TRACE_DEBUG("%s: COMPLETED\n", ESIF_FUNC);
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	if (NULL != io_ptr) {
		IOStream_Close(io_ptr);
		IOStream_Destroy(io_ptr);
	}
	EsifData_Destroy(nameSpace);
	EsifData_Destroy(key);
	EsifData_Destroy(value);
	return rc;
}


eEsifError EsifDspInit()
{
	eEsifError rc = ESIF_OK;
	ESIF_TRACE_DEBUG("%s: Init Device Support Package (DSP)", ESIF_FUNC);
	return rc;
}


void EsifDspExit()
{
	ESIF_TRACE_DEBUG("%s: Exit Device Support Package (DSP)", ESIF_FUNC);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
