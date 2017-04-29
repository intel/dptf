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

// Verify EDP Header
Bool esif_verify_edp(struct edp_dir *edp, size_t size)
{
	// Verify Header Size, Signature, and Major DSP Version
	return ((size == sizeof(struct edp_dir)) &&
		(edp->signature == *(unsigned int *)ESIF_EDP_SIGNATURE) &&
		(edp->version == (unsigned int)atoi(ESIF_DSP_VERSION)));
}

// Send DSP
enum esif_rc esif_send_dsp(
	char *filename,
	u8 dstId
	)
{
	enum esif_rc rc = ESIF_OK;
	int edpSize = 512;
	int cmdSize = 0;
	struct esif_ipc *ipcPtr = NULL;
	struct esif_ipc_command *commandPtr = NULL; 
	struct esif_command_send_dsp *dspCommandPtr = NULL;
	struct edp_dir edpDir;
	size_t bytesRead;
	char *edpName        = 0;
	IOStreamPtr ioPtr    = IOStream_Create();
	EsifDataPtr nameSpace = 0;
	EsifDataPtr key       = 0;
	EsifDataPtr value     = 0;

	if (ioPtr == NULL) {
		ESIF_TRACE_ERROR("Fail to create IOStream\n");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	//
	// If we have a filename provided use the contents of the file as the CPC
	// note this is opaque it is up to the receiver to verify that this is in fact
	// a CPC.
	//
	if (NULL == filename) {
		ESIF_TRACE_ERROR("Filename is null\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	// Use name portion of filename for the DataVault key (C:\path\file.edp = file.edp)
	edpName  = strrchr(filename, *ESIF_PATH_SEP);
	edpName  = (edpName ? ++edpName : filename);
	nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, ESIF_DSP_NAMESPACE, 0, ESIFAUTOLEN);
	key       = EsifData_CreateAs(ESIF_DATA_STRING, edpName, 0, ESIFAUTOLEN);
	value     = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);

	if (nameSpace == NULL || key == NULL || value == NULL) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	// Look for EDP file on disk first then DataVault (static or file)
	if (!esif_ccb_file_exists(filename) && EsifConfigGet(nameSpace, key, value) == ESIF_OK) {
		filename = edpName;
		IOStream_SetMemory(ioPtr, StoreStatic, (BytePtr)value->buf_ptr, value->data_len);
	} else {
		IOStream_SetFile(ioPtr, StoreStatic, filename, (char *)"rb");
	}

	/* FIND CPC within EDP file */
	if (IOStream_Open(ioPtr) != 0) {
		ESIF_TRACE_ERROR("File not found (%s)\n", filename);
		rc = ESIF_E_IO_OPEN_FAILED;
		goto exit;
	}
	bytesRead = IOStream_Read(ioPtr, &edpDir, sizeof(struct edp_dir));
	if (!esif_verify_edp(&edpDir, bytesRead)) {
		ESIF_TRACE_ERROR("Invalid EDP Header: Signature=%4.4s Version=%d\n", (char *)&edpDir.signature, edpDir.version);
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}
	edpSize = edpDir.fpc_offset - edpDir.cpc_offset;
	IOStream_Seek(ioPtr, edpDir.cpc_offset, SEEK_SET);

	if (edpSize > MAX_EDP_SIZE) {
		ESIF_TRACE_ERROR("The edp size %d is larger than maximum edp size\n", edpSize);
		rc = -ESIF_E_UNSPECIFIED;
		goto exit;
	}

	cmdSize = edpSize + sizeof(*dspCommandPtr);

	ipcPtr = esif_ipc_alloc_command(&commandPtr, cmdSize);
	if (NULL == ipcPtr || NULL == commandPtr) {
		ESIF_TRACE_ERROR("Fail to allocate esif_ipc/esif_ipc_command\n");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	commandPtr->type = ESIF_COMMAND_TYPE_SEND_DSP;
	commandPtr->req_data_type = ESIF_DATA_STRUCTURE;
	commandPtr->req_data_offset = 0;
	commandPtr->req_data_len = cmdSize;
	commandPtr->rsp_data_type = ESIF_DATA_VOID;
	commandPtr->rsp_data_offset = 0;
	commandPtr->rsp_data_len = 0;

	dspCommandPtr = (struct esif_command_send_dsp *)(commandPtr + 1);
	dspCommandPtr->id = dstId;
	dspCommandPtr->data_len = edpSize;

	bytesRead = IOStream_Read(ioPtr, dspCommandPtr + 1, edpSize);
	ESIF_TRACE_DEBUG("loaded file %s bytes %d\n", filename, (int)bytesRead);

	ESIF_TRACE_INFO("CPC file %s(%d) sent to participant %d\n", filename, edpSize, dstId);

	ipc_execute(ipcPtr);

	if (ESIF_OK != ipcPtr->return_code) {
		ESIF_TRACE_ERROR("ipc error code = %s(%d)\n", esif_rc_str(ipcPtr->return_code), ipcPtr->return_code);
		rc = ipcPtr->return_code;
		goto exit;
	}

	rc = commandPtr->return_code;
	if ((rc != ESIF_OK) && (rc != ESIF_E_DSP_ALREADY_LOADED)) {
		ESIF_TRACE_ERROR("primitive error code = %s(%d)\n", esif_rc_str(commandPtr->return_code), commandPtr->return_code);
		goto exit;
	}
exit:
	if (NULL != ipcPtr) {
		esif_ipc_free(ipcPtr);
	}
	if (NULL != ioPtr) {
		IOStream_Close(ioPtr);
		IOStream_Destroy(ioPtr);
	}
	EsifData_Destroy(nameSpace);
	EsifData_Destroy(key);
	EsifData_Destroy(value);
	return rc;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
