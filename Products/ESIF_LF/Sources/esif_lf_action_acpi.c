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

/*
 * ACPI_TYPE_INTEGER in Linux:
 *   struct  {u32 type, u64 value}
 *   Example {INTEGER     X  X  X  X  X  X  X  X}  32/64-bit Lin
 *   Byte     0  1  2  3  4  5  6  7  8  9 10 11
 * ACPI_METHOD_ARGUMENT_INTEGER in Windows
 *   struct  {u16 Type, u16 DataLength, ULONG argument}
 *   Example {INTER Lenth X  X  X  X }             32-bit Win
 *   Example {INTER Lenth X  X  X  X  X  X  X  X}  64-bit Win
 *   Byte     0  1  2  3  4  5  6  7  8  9 10 11
 *
 * ACPI_TYPE_STRING in Linux:
 *   struct  {u32 type, u32 length, u8 *pointer}
 *   Example {STRG        6  0  0  0  *----------}-----------> "H e l l o"
 *   Byte     0  1  2  3  4  5  6  7  8  9  10  11
 * ACPI_METHOD_ARGUMENT_STRING in Windows:
 *   struct  {u16 Type, u16 DataLength, u8 Data[]}
 *   Example {STRG  6  0 "H  e  l  l  o \0"}
 *   Byte     0  1  2  3  4  5  6  7  8  9
 *
 * ACPI_TYPE_PACKAGE in Linux
 *   struct  {u32 type, u32 count, acpi_object *element}
 *   Example {PKG         2  0  0  0  *           }
 *   Byte     0  1  2  3  4  5  6  7  8  9  10  11
 *                 +---------------+
 *                 |
 *             [0] +---------->  {INTEGER     X  X  X  X  X  X  X  X}
 *                 |     Byte     0  1  2  3  4  5  6  7  8  9  10 11
 *             [1] +---------->  {INTEGER     X  X  X  X  X  X  X  X}
 *                       Byte     0  1  2  3  4  5  6  7  8  9  10 11
 *   struct  {u32 type, u32 count, acpi_object *element}
 *   Example {PKG         2  0  0  0  *           }
 *   Byte     0  1  2  3  4  5  6  7  8  9  10  11
 *             +---------------+
 *             |
 *         [0] +------->  {STRING      6  0  0  0  *------------}----> " Hello"
 *             |  Byte     0  1  2  3  4  5  6  7  8  9  10 11
 *         [1] +------->  {STRING      6  0  0  0  +------------}----> " World"
 *                Byte     0  1  2  3  4  5  6  7  8  9  10 11
 *
 * ACPI_METHOD_ARGUMENT_PACKAGE in Windows
 *   struct  {u16 Type, u16 DataLength, u8 Data[]}
 *   Example {PKG  20  0  STR   6  0 "H  i  \0" STR   6  0  "W  o  r  l  d \0"}
 *   Byte     0  1  2  3  4  5  6  7  8  9  10  11 12 13 14 15 16 17 18 29 20
 *
 *   struct  {u16 Type, u16 DataLength, u8 Data[]}
 *   Example {PKG   24 0  INT   8  0  X  X  X  X  X  X  X  X  INT   8  0  X  X  X  X  X  X  X  X}
 *   Byte     0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27
 *
 * Example of Windows ACPI object _FPS
 *   In ACPI_EVAL_OUTPUT_BUFFER {Length=548, Count=13, Argument=}
 *   In ACPI_METHOD_ARGUMENT {Type, DataLength Argument/Data}
 *     count=1  <INT 4 val>
 *     count=2  <PKG 40 <INT 4 val> <INT 4 val> <INT 4 val> <INT 4 val> <INT 4 val>>
 *     count=3  <PKG 40 <INT 4 val> <INT 4 val> <INT 4 val> <INT 4 val> <INT 4 val>>
 *                   ...
 *     |<-----------------    Data[40]  ------------------------>|
 *     |<- ARG ->| |<- ARG ->| |<- ARG ->| |<- ARG ->| |<- ARG ->|
 *     count=13 <PKG 40 <INT 4 val> <INT 4 val> <INT 4 val> <INT 4 val> <INT 4 val>>
 *                       Control     TripPoint  Speed       Acoustic    Power
 */

#include "esif_action.h"

#ifdef ESIF_ATTR_OS_WINDOWS

/*
 * The Windows banned-API check header must be included after all other headers,
 * or issues can be identified against Windows SDK/DDK included headers which we
 * have no control over.
 */
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/* Debug Logging Defintions */

#define ESIF_DEBUG_MODULE ESIF_DEBUG_MOD_ACTION_ACPI

#define INIT_DEBUG       0
#define GET_DEBUG        1
#define SET_DEBUG        2
#define UNPACK_DEBUG     3

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_ACPI, \
		       INIT_DEBUG, \
		       format, \
		       ##__VA_ARGS__)
#define ESIF_TRACE_DYN_SET(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_ACPI, \
		       SET_DEBUG, \
		       format, \
		       ##__VA_ARGS__)
#define ESIF_TRACE_DYN_GET(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_ACPI, \
		       GET_DEBUG, \
		       format, \
		       ##__VA_ARGS__)
#define ESIF_TRACE_DYN_UNPACK(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_ACPI, \
		       UNPACK_DEBUG, \
		       format, \
		       ##__VA_ARGS__)

/* Binary Object For ACPI Parsing*/
union esif_binary_object {
	u8  *buf_ptr;
	union esif_data_variant *variant_ptr;
};

static void esif_unpack_acpi_object(union acpi_object*,
				    struct esif_action_data*,
				    const u32);

/* Lock */
static esif_ccb_lock_t esif_action_acpi_lock;

/* Move this to database and make data driven from acpi method excep table */
static enum esif_rc esif_acpi_xlate_error(
	acpi_status status,
	acpi_string method
	)
{
	if (status == AE_NOT_FOUND) {
		/* Exception List */
		if (!strncmp(method, "_AC", 3))
			return ESIF_I_ACPI_TRIP_POINT_NOT_PRESENT;
		if (!strncmp(method, "_PSV", 4))
			return ESIF_I_ACPI_TRIP_POINT_NOT_PRESENT;
		if (!strncmp(method, "_CRT", 4))
			return ESIF_I_ACPI_TRIP_POINT_NOT_PRESENT;
		if (!strncmp(method, "_HOT", 4))
			return ESIF_I_ACPI_TRIP_POINT_NOT_PRESENT;
		if (!strncmp(method, "_CR3", 4))
			return ESIF_I_ACPI_TRIP_POINT_NOT_PRESENT;
		if (!strncmp(method, "_STR", 4))
			return ESIF_I_ACPI_OBJECT_NOT_PRESENT;
		if (!strncmp(method, "_UID", 4))
			return ESIF_I_ACPI_OBJECT_NOT_PRESENT;
		return ESIF_E_ACPI_OBJECT_NOT_FOUND;
	}
	return ESIF_E_ACPI_EVAL_FAILURE;
}


static ESIF_INLINE
void esif_acpi_mem_dump(
	const u8 *ch,
	const u8 *what,
	const int size
	)
{
	int i;

	what = what;	/* Avoid Compiler Warnings */
	ESIF_TRACE_DYN_SET("%s: Dumping Memory for %s, size %d\n",
			   ESIF_FUNC,
			   what,
			   size);
	for (i = 0; i < size; i++, ch++)
		ESIF_TRACE_DYN_SET("<%p> 0x%02X\n", (u8 *)ch, (u8) *ch);
	ESIF_TRACE_DYN_SET("End of Memory Dump\n");
}


/*
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *
 *               A C P I     G E T     A C T I O N
 *
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 */
enum esif_rc esif_get_action_acpi(
	acpi_handle acpi_handle,
	const u32 acpi_method,
	const struct esif_data *req_data_ptr,
	struct esif_data *rsp_data_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	acpi_status acpi_status;
	char acpi_method_str[4 + 1];	/* _AC0 + NULL */
	struct acpi_buffer buf = {ESIF_ELEMENT(.length) ACPI_ALLOCATE_BUFFER};
	union  acpi_object *obj_ptr = NULL;
	struct esif_action_data *data_ptr = NULL;
	u32 i = 0;		/* Loop Counter   */
	u32 arg_count = 0;	/* Argument Count */

	esif_ccb_memcpy(&acpi_method_str, &acpi_method, 4);
	acpi_method_str[4] = 0;

	UNREFERENCED_PARAMETER(req_data_ptr);

	if (NULL == acpi_handle)
		return ESIF_E_NO_ACPI_SUPPORT;

	ESIF_TRACE_DYN_GET(
		"%s: handle %p method %s [request type %s len %d] "
		"[respond type %s len %d buf_ptr %p]\n",
		ESIF_FUNC,
		acpi_handle,
		acpi_method_str,
		esif_data_type_str(req_data_ptr->type),
		req_data_ptr->buf_len,
		esif_data_type_str(rsp_data_ptr->type),
		rsp_data_ptr->buf_len,
		rsp_data_ptr->buf_ptr);

	acpi_status = esif_ccb_acpi_evaluate_object(acpi_handle,
						    acpi_method_str,
						    NULL,
						    &buf);

	if (esif_ccb_has_acpi_failure(acpi_status, &buf)) {
		/* Translate AE error into ESIF error code */
		rc = esif_acpi_xlate_error(acpi_status, acpi_method_str);
		goto free_exit;
	}

	data_ptr = esif_ccb_malloc(sizeof(*data_ptr));
	if (NULL == data_ptr) {
		rc = ESIF_E_NO_MEMORY;
		goto free_exit;
	}

	/*
	 * 'Count' (nothing to do with the PACKAGE counter) to tell how many
	 * 'methods' are followed, this is different from Linux implementation.
	 * In Linux, only a PACKAGE can have a counter for multiple objects!
	 */

	/* Get ACPI Buffer Attributes */
	arg_count = esif_ccb_acpi_arg_count(&buf);
	obj_ptr   = esif_ccb_acpi_arg_first(&buf);

	/* Init Action With Default Value */
	data_ptr->rsp_data_ptr = rsp_data_ptr;
	data_ptr->needed_len   = 0;
	data_ptr->rc = ESIF_OK;
	data_ptr->rows         = (u16)arg_count;
	data_ptr->cols         = 1;	/* Default is 1 cols, not 0 */
	data_ptr->revision     = ESIF_TABLE_NO_REVISION;
	data_ptr->first_type   = ESIF_NO_ACPI_TYPE;
	data_ptr->second_type  = ESIF_NO_ACPI_TYPE;
	data_ptr->third_type   = ESIF_NO_ACPI_TYPE;

	/* Note Argument count will be 1 for non-Windows OS */
	for (i = 0; i < arg_count; i++) {
		esif_unpack_acpi_object(obj_ptr, data_ptr, acpi_method);
		obj_ptr = esif_acpi_arg_next(obj_ptr);
		if (NULL == obj_ptr)
			break;
	}

	/* Debug - Dump the returned buffer */
	rc = data_ptr->rc;
	rsp_data_ptr->data_len = data_ptr->needed_len;

	if (rsp_data_ptr->type == ESIF_DATA_TABLE) {
		struct esif_table_hdr *table =
			(struct esif_table_hdr *) rsp_data_ptr->buf_ptr;

		/* Table has a const length and not tracked by needed_len */
		rsp_data_ptr->data_len = sizeof(*table);

		if (rsp_data_ptr->buf_len >= rsp_data_ptr->data_len) {
			esif_acpi_check_revision(data_ptr);

			table->revision = (u8)data_ptr->revision;
			table->rows     = data_ptr->rows;
			table->cols     = data_ptr->cols;

			ESIF_TRACE_DYN_GET(
				"%s: For ESIF_DATA_TABLE acpi %s rev %d rows %d cols %d data_len %d\n",
				ESIF_FUNC,
				acpi_method_str,
				table->revision,
				table->rows,
				table->cols,
				rsp_data_ptr->data_len);
			if (table->rows > 1)
				rc = ESIF_OK;
			else
				rc = ESIF_E_NOT_TABLE;
		} else {
			rc = ESIF_E_NEED_LARGER_BUFFER;
		}
	}

	esif_ccb_free(data_ptr);

free_exit:
	if (NULL != buf.pointer)
		esif_ccb_free(buf.pointer);

	return rc;
}


/*
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *
 *                      S E T     A C T I O N
 *
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 */
enum esif_rc esif_set_action_acpi(
	acpi_handle acpi_handle,
	const u32 acpi_method,
	struct esif_data *req_data_ptr
	)
{
	union acpi_object *out_obj;
	struct acpi_object_list arg_list = {0, NULL};
	struct acpi_buffer output        = {ACPI_ALLOCATE_BUFFER, NULL};
	enum esif_rc rc = ESIF_OK;
	char acpi_method_str[4 + 1];	/* _AC0 + NULL */
	acpi_status acpi_status;
	int size;
	struct esif_data_complex_osc *osc_ptr = NULL;
	struct esif_data_complex_scp *scp_ptr = NULL;

	if (NULL == acpi_handle)
		return ESIF_E_NO_ACPI_SUPPORT;

	if (NULL == req_data_ptr)
		return ESIF_E_NEED_LARGER_BUFFER;

	esif_ccb_memcpy(&acpi_method_str, &acpi_method, 4);
	acpi_method_str[4] = 0;

	ESIF_TRACE_DYN_SET(
		"%s: handle %p ACPI method %s (%x) [request type %s len %d]\n",
		ESIF_FUNC,
		acpi_handle,
		acpi_method_str,
		acpi_method,
		esif_data_type_str(req_data_ptr->type),
		req_data_ptr->buf_len);

	switch (acpi_method) {
	case 'CSO_':
	{
		if (ESIF_DATA_STRUCTURE != req_data_ptr->type ||
		    sizeof(struct esif_data_complex_osc) !=
		    req_data_ptr->buf_len) {
			rc = ESIF_E_INVALID_REQUEST_TYPE;
			goto exit;
		}

		req_data_ptr->type = ESIF_DATA_BINARY;
		osc_ptr = (struct esif_data_complex_osc *)req_data_ptr->buf_ptr;

		ESIF_TRACE_DYN_SET(
			"\nOSC Data <%p>\n"
			"------------------------\n"
			"GUID:         %x... TBD DUMP GUID\n"
			"Revision:     %u\n"
			"Count:        %u\n"
			"Status:       %u\n"
			"Capabilities: %x\n\n",
			osc_ptr,
			*(u32 *) &osc_ptr->guid,
			osc_ptr->revision,
			osc_ptr->count,
			osc_ptr->status,
			osc_ptr->capabilities);
	}
	break;	/* CSO_ */

	case 'PCS_':
	{
		if (ESIF_DATA_STRUCTURE != req_data_ptr->type ||
		    sizeof(struct esif_data_complex_scp) !=
		    req_data_ptr->buf_len) {
			rc = ESIF_E_INVALID_REQUEST_TYPE;
			goto exit;
		}

		req_data_ptr->type = ESIF_DATA_BINARY;
		scp_ptr = (struct esif_data_complex_scp *)req_data_ptr->buf_ptr;
		ESIF_TRACE_DYN_SET(
			"\nSCP Data <%p>\n"
			"------------------------\n"
			"Cooling Mode  : %u 0 = Active, 1 = Passive\n"
			"Acoustic Limit: %u 1 to 5 Inclusive\n"
			"Power Limit   : %u 1 to 5 Inclusive\n\n",
			scp_ptr,
			scp_ptr->cooling_mode,
			scp_ptr->acoustic_limit,
			scp_ptr->power_limit);
	}
	break;	/* PCS_ */

	default:
		break;
	}

	/* Count number of objects to set based request data length and type */
	switch (req_data_ptr->type) {
	case ESIF_DATA_UINT8:
		arg_list.count = req_data_ptr->buf_len / sizeof(u8);
		break;

	case ESIF_DATA_UINT16:
		arg_list.count = req_data_ptr->buf_len / sizeof(u16);
		break;

	case ESIF_DATA_UINT32:
	case ESIF_DATA_TEMPERATURE:
		arg_list.count = req_data_ptr->buf_len / sizeof(u32);
		break;

	case ESIF_DATA_UINT64:
	case ESIF_DATA_FREQUENCY:
		arg_list.count = req_data_ptr->buf_len / sizeof(u64);
		break;

	case ESIF_DATA_BINARY:
	{
		switch (acpi_method) {
		case 'CSO_':
			arg_list.count = 4;
			break;

		case 'PCS_':
			arg_list.count = 3;
			break;

		default:
			rc = ESIF_E_UNSUPPORTED_REQUEST_DATA_TYPE;
			goto exit;
		}
	}
	break;

	default:
		rc = ESIF_E_UNSUPPORTED_REQUEST_DATA_TYPE;
		goto exit;
	}

	if (0 == arg_list.count) {
		rc = ESIF_E_INVALID_REQUEST_TYPE;
		goto exit;
	}

	/* Allocate max size (OSC): 16: uuid, 2 * sizeof(u32): capability  */
	size =
		(sizeof(union acpi_object) *
		 arg_list.count) + 16 + (2 * sizeof(u32));
	arg_list.pointer = (union acpi_object *)esif_ccb_malloc(size);
	if (arg_list.pointer == NULL) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	esif_ccb_memset(arg_list.pointer, 0, size);

	esif_create_acpi_buffer(acpi_method, &arg_list, req_data_ptr);

	acpi_status = esif_ccb_acpi_evaluate_object(acpi_handle,
						    acpi_method_str,
						    &arg_list,
						    &output);

	ESIF_TRACE_DYN_SET("%s: acpi_method %s ACPI error status 0x%x\n",
			   ESIF_FUNC,
			   acpi_method_str,
			   acpi_status);

	if (esif_ccb_has_acpi_failure(acpi_status, NULL)) {
		/* Translate AE error into ESIF error code in SET */
		rc = ESIF_E_ACPI_EVAL_FAILURE;
		goto free_exit;
	}

	/* Special case for _OSC where we update the request data as return */
	if (acpi_method == 'CSO_') {
		ESIF_TRACE_DYN_SET("OSC: status 0x%x, output %p\n",
				   acpi_status,
				   &output);

		out_obj = output.pointer;
		rc      = esif_handle_osc_buf(&output, req_data_ptr);

		ESIF_TRACE_DYN_SET(
			"\nOSC Returned Data <%p>\n"
			"------------------------\n"
			"Status:       %u\n"
			"Capabilities: %x\n\n",
			osc_ptr,
			osc_ptr->status,
			osc_ptr->capabilities);
		if (rc != ESIF_OK)
			rc = ESIF_E_ACPI_EVAL_FAILURE;
	}

free_exit:
	if (NULL != arg_list.pointer)
		esif_ccb_free(arg_list.pointer);

	if (NULL != output.pointer)
		esif_ccb_free(output.pointer);
exit:
	ESIF_TRACE_DYN_SET("%s: ACPI method %s rc %x\n",
			   ESIF_FUNC,
			   acpi_method_str,
			   rc);
	return rc;
}


/* Init */
enum esif_rc esif_action_acpi_init(void)
{
	ESIF_TRACE_DYN_INIT("%s: Initialize ACPI Action\n", ESIF_FUNC);
	esif_ccb_lock_init(&esif_action_acpi_lock);
	return ESIF_OK;
}


/* Exit */
void esif_action_acpi_exit(void)
{
	esif_ccb_lock_uninit(&esif_action_acpi_lock);
	ESIF_TRACE_DYN_INIT("%s: Exit ACPI Action\n", ESIF_FUNC);
}


static void esif_unpack_acpi_object(
	union acpi_object *obj_ptr,
	struct esif_action_data *data_ptr,
	const u32 acpi_method
	)
{
	union esif_binary_object bin;
	u32 offset;
	u32 str_len = 0;
	u32 i, is_unicode = 0;

	if (NULL == obj_ptr || NULL == data_ptr)
		return;

	offset      = data_ptr->needed_len;
	bin.buf_ptr = (u8 *)data_ptr->rsp_data_ptr->buf_ptr;

	ESIF_TRACE_DYN_UNPACK(
		"Entering %s: acpi_object %p, output buf %p, offset %d, "
		"type %d\n",
		ESIF_FUNC,
		obj_ptr,
		bin.buf_ptr,
		offset,
		obj_ptr->type);

	/* For Table */
	if (data_ptr->rsp_data_ptr->type == ESIF_DATA_TABLE) {
		if (data_ptr->first_type == ESIF_NO_ACPI_TYPE)
			data_ptr->first_type = obj_ptr->type;
		else if (data_ptr->second_type == ESIF_NO_ACPI_TYPE)
			data_ptr->second_type = obj_ptr->type;
		else if (data_ptr->third_type == ESIF_NO_ACPI_TYPE)
			data_ptr->third_type = obj_ptr->type;
		/* Save the first integer which may or may not be a revision */
		if (obj_ptr->type == ACPI_TYPE_INTEGER &&
		    (data_ptr->revision == ESIF_TABLE_NO_REVISION))
			data_ptr->revision = (u16)obj_ptr->integer.value;
	}

	/* Memory dump for debugging */

	/* Position for data store in buf_ptr */
	bin.buf_ptr = bin.buf_ptr + offset;

	switch (obj_ptr->type) {
	/* ACPI Package */
	case ACPI_TYPE_PACKAGE:
	{
		union acpi_object *element_ptr = obj_ptr;
		u32 count = esif_ccb_acpi_pkg_count(obj_ptr);

		ESIF_TRACE_DYN_UNPACK(
			"%s: Have ACPI Package Count = %d Element = %p "
			"Package = %p\n",
			ESIF_FUNC,
			count,
			element_ptr,
			obj_ptr);

		/* Start collecting TABLE header */
		if (data_ptr->rsp_data_ptr->type == ESIF_DATA_TABLE)
			esif_acpi_set_cols(data_ptr, count);

		for (i = 0; i < count; i++) {
			element_ptr = esif_ccb_acpi_pkg_next(obj_ptr,
							     element_ptr,
							     i);
			esif_unpack_acpi_object(element_ptr,
						data_ptr,
						acpi_method);
		}
		break;
	}	/* ACPI_TYPE_PACKAGE */

	case ACPI_TYPE_INTEGER:
	{
		ESIF_TRACE_DYN_UNPACK("%s: Have ACPI Integer = %p = %llu\n",
				      ESIF_FUNC, obj_ptr,
				      (u64)obj_ptr->integer.value);

		switch (data_ptr->rsp_data_ptr->type) {
		case ESIF_DATA_UINT8:
			data_ptr->needed_len += sizeof(u8);
			if (data_ptr->needed_len <= data_ptr->rsp_data_ptr->buf_len)
				*((u8 *)bin.buf_ptr) = (u8) obj_ptr->integer.value;
			else
				data_ptr->rc = ESIF_E_NEED_LARGER_BUFFER;
			break;

		case ESIF_DATA_UINT16:
			data_ptr->needed_len += sizeof(u16);
			if (data_ptr->needed_len <= data_ptr->rsp_data_ptr->buf_len)
				*((u16 *)bin.buf_ptr) = (u16) obj_ptr->integer.value;
			else
				data_ptr->rc = ESIF_E_NEED_LARGER_BUFFER;
			break;

		case ESIF_DATA_UINT32:
			data_ptr->needed_len += sizeof(u32);
			if (data_ptr->needed_len <= data_ptr->rsp_data_ptr->buf_len)
				*((u32 *)bin.buf_ptr) = (u32) obj_ptr->integer.value;
			else
				data_ptr->rc = ESIF_E_NEED_LARGER_BUFFER;
			break;

		case ESIF_DATA_UINT64:
		case ESIF_DATA_FREQUENCY:
			data_ptr->needed_len += sizeof(u64);
			if (data_ptr->needed_len <= data_ptr->rsp_data_ptr->buf_len)
				*((u64 *)bin.buf_ptr) = (u64) obj_ptr->integer.value;
			else
				data_ptr->rc = ESIF_E_NEED_LARGER_BUFFER;
			break;

		case ESIF_DATA_STRING:	/* _UID only */
			/*
			 * For _UID, can be implemented as either string or
			 * integer according to ACPI spec.
			 */
			if (acpi_method == 'DIU_') {
				/* Make Sure We Have Enough Space */
				if (data_ptr->rsp_data_ptr->buf_len >
				    ((sizeof(obj_ptr->integer.value) * 2) + 2)) {
					data_ptr->needed_len +=
						data_ptr->rsp_data_ptr->buf_len;
					esif_ccb_sprintf(
						data_ptr->rsp_data_ptr->buf_len,
						(char *)bin.buf_ptr,
						"0x%x",
						(u32)obj_ptr->integer.value);
				} else {
					data_ptr->rc = ESIF_E_NEED_LARGER_BUFFER;
				}
			} else {
				data_ptr->rc = ESIF_E_ACPI_REQUEST_TYPE;
			}
			break;

		case ESIF_DATA_BINARY:
		{
			data_ptr->needed_len += sizeof(union esif_data_variant);
			if (data_ptr->needed_len <=
			    data_ptr->rsp_data_ptr->buf_len) {
				bin.variant_ptr->integer.type  =
					ESIF_DATA_UINT64;
				bin.variant_ptr->integer.value =
					(u64)obj_ptr->integer.value;
			} else {
				data_ptr->rc = ESIF_E_NEED_LARGER_BUFFER;
			}
			ESIF_TRACE_DYN_UNPACK(
				"%s: ACPI_INTEGER type %d val %u, ESIF_BINARY output type %u value %llu, needed_len %u rc %u\n",
				ESIF_FUNC,
				obj_ptr->integer.type,
				(int)obj_ptr->integer.value,
				bin.variant_ptr->integer.type,
				(u64)bin.variant_ptr->integer.value,
				(int)data_ptr->needed_len,
				data_ptr->rc);
			break;
		}

		default:
			data_ptr->rc = ESIF_E_ACPI_REQUEST_TYPE;
		}
		break;
	}	/* ACPI_TYPE_INTEGER */

	/*
	** ACPI_TYPE_BUFFER should be
	**   1) Can be binary (GUID) or unicode (_STR, need unicode2ascii),
	**   2) Windows and Linux report the same buffer.length,
	**   3) Examples _STR, IDSP
	*/
	case ACPI_TYPE_BUFFER:
	{
		ESIF_TRACE_DYN_UNPACK("%s: Have ACPI Buffer = %p\n",
				      ESIF_FUNC,
				      obj_ptr);

		is_unicode = esif_ccb_is_unicode(acpi_method);

		switch (data_ptr->rsp_data_ptr->type) {
		/* Responding string for ACPI buffer type */
		case ESIF_DATA_STRING:
		{
			str_len = esif_acpi_get_strlen(obj_ptr->type,
						       is_unicode,
						       obj_ptr->string.length);
			data_ptr->needed_len += str_len;
			if (data_ptr->needed_len <
			    data_ptr->rsp_data_ptr->buf_len) {
				if (is_unicode) {
					/* Can convert unicode into ASCII */
					bin.variant_ptr->string.length =
						str_len;
					bin.variant_ptr->string.type   =
						ESIF_DATA_STRING;
					esif_acpi_uni2ascii(
						(char *)bin.variant_ptr,
						obj_ptr->string.length,
						(u8 *)obj_ptr->string.pointer,
						str_len);
				} else {
					/*
					 * Cannot convert binary into ASCII,
					 * just memcpy
					 */
					bin.variant_ptr->string.length =
						str_len;
					bin.variant_ptr->string.type   =
						ESIF_DATA_BINARY;
					esif_acpi_memcpy(bin.variant_ptr,
							 obj_ptr->string.pointer,
							 str_len);
				}
			} else {
				data_ptr->rc = ESIF_E_NEED_LARGER_BUFFER;
			}
			ESIF_TRACE_DYN_UNPACK(
				"%s: ACPI_BUFFER addr %p len %d %s, ESIF_STRING from addr %p buf len %u %s, needed_len %d rc %u\n",
				ESIF_FUNC,
				obj_ptr,
				obj_ptr->string.length,
				obj_ptr->string.pointer,
				bin.buf_ptr,
				str_len,
				bin.buf_ptr,
				data_ptr->needed_len,
				data_ptr->rc);
			break;
		}

		/* Responding binary for ACPI buffer type */
		case ESIF_DATA_BINARY:
		{
			str_len = esif_acpi_get_strlen(obj_ptr->type,
						       is_unicode,
						       obj_ptr->string.length);
			data_ptr->needed_len +=
				sizeof(union esif_data_variant) + str_len;

			if (data_ptr->needed_len <=
			    data_ptr->rsp_data_ptr->buf_len) {
				u8 *temp_ptr = NULL;
				bin.variant_ptr->string.type   =
					ESIF_DATA_BINARY;
				bin.variant_ptr->string.length = str_len;
				temp_ptr =
					(u8 *)((acpi_string)bin.variant_ptr +
					      sizeof(union esif_data_variant));
				if (is_unicode) {
					esif_acpi_uni2ascii((char *)temp_ptr,
							    obj_ptr->string.length,
							    (u8 *)obj_ptr->string.pointer,
							    str_len);
				} else {
					esif_acpi_memcpy(temp_ptr,
							 obj_ptr->string.pointer,
							 str_len);
				}
			} else {
				data_ptr->rc = ESIF_E_NEED_LARGER_BUFFER;
			}
			ESIF_TRACE_DYN_UNPACK(
				"%s: ACPI_BUFFER %p len %u, ESIF_BINARY "
				"addr %p %s len (%u + %u), needed_len %u rc %d\n",
				ESIF_FUNC,
				obj_ptr->string.pointer,
				(u32)sizeof(union esif_data_variant),
				bin.variant_ptr,
				(char *)bin.variant_ptr,
				(u32)sizeof(obj_ptr->string),
				str_len,
				data_ptr->needed_len,
				data_ptr->rc);
			break;
		}

		/* Responding unicode for ACPI buffer type */
		case ESIF_DATA_UNICODE:
		{
			str_len = obj_ptr->string.length;
			data_ptr->needed_len += str_len;
			if (data_ptr->needed_len <
			    data_ptr->rsp_data_ptr->buf_len) {
				if (is_unicode) {
					bin.variant_ptr->string.type   =
						ESIF_DATA_UNICODE;
					bin.variant_ptr->string.length =
						obj_ptr->string.length;
					esif_acpi_memcpy(bin.variant_ptr,
							 obj_ptr->string.pointer,
							 obj_ptr->string.length);
				} else {
					/* No sense to copy non-unicode */
					data_ptr->rc =
						ESIF_E_INVALID_REQUEST_TYPE;
				}
			} else {
				data_ptr->rc = ESIF_E_NEED_LARGER_BUFFER;
			}
			ESIF_TRACE_DYN_UNPACK(
				"%s: ACPI_BUFFER addr %p len %d %s, "
				"ESIF_UNICODE from addr %p buf len %u %s, "
				"needed_len %d rc %u\n",
				ESIF_FUNC,
				obj_ptr,
				obj_ptr->string.length,
				obj_ptr->string.pointer,
				bin.buf_ptr,
				str_len,
				bin.buf_ptr,
				data_ptr->needed_len,
				data_ptr->rc);
			break;
		}

		default:
			data_ptr->rc = ESIF_E_ACPI_REQUEST_TYPE;
		}
		break;
	}	/* ACPI_TYPE_BUFFER */
	break;

	/*
	** ACPI_TYPE_STRING should be
	**   1) Always ASCII code (NO unicode),
	**   2) Windows always has \0 in the end while Linux never has it,
	**   3) One example, AEXL
	*/
	case ACPI_TYPE_STRING:
	{
		ESIF_TRACE_DYN_UNPACK("%s: Have ACPI String = %p\n",
				      ESIF_FUNC,
				      obj_ptr);

		switch (data_ptr->rsp_data_ptr->type) {
		/* Responding integer for ACPI string type (_UID only) */
		case ESIF_DATA_UINT32:
		{
			/*
			 * Take care of _UID which can be implemented as either
			 * string or integer according to ACPI spec.
			 */
			if (acpi_method == 'DIU_') {
				str_len = esif_acpi_get_strlen(obj_ptr->type,
							       is_unicode,
							       obj_ptr->string.length);
				/* str_len is 1-byte '\0' which we don't care */
				if (data_ptr->rsp_data_ptr->buf_len == (str_len - 1)) {
					data_ptr->needed_len += data_ptr->rsp_data_ptr->buf_len;
					esif_acpi_memcpy(bin.buf_ptr,
							 obj_ptr->string.pointer,
							 data_ptr->rsp_data_ptr->buf_len);
				} else {
					data_ptr->rc = ESIF_E_NEED_LARGER_BUFFER;
				}
			} else {
				data_ptr->rc = ESIF_E_ACPI_REQUEST_TYPE;
			}
			break;
		}

		/* Responding string for ACPI string type */
		case ESIF_DATA_STRING:
		{
			str_len = esif_acpi_get_strlen(obj_ptr->type,
						       is_unicode,
						       obj_ptr->string.length);
			data_ptr->needed_len += str_len;
			if (data_ptr->needed_len <
			    data_ptr->rsp_data_ptr->buf_len) {
				bin.variant_ptr->string.type   =
					ESIF_DATA_STRING;
				bin.variant_ptr->string.length = str_len;
				esif_acpi_memcpy(bin.variant_ptr,
						 obj_ptr->string.pointer,
						 str_len);
			} else {
				data_ptr->rc = ESIF_E_NEED_LARGER_BUFFER;
			}
			ESIF_TRACE_DYN_UNPACK(
				"%s: ACPI_STRING addr %p len %d %s, ESIF_STRING from addr %p buf len %u %s, needed_len %d rc %u\n",
				ESIF_FUNC,
				obj_ptr,
				obj_ptr->string.length,
				obj_ptr->string.pointer,
				bin.buf_ptr,
				str_len,
				bin.buf_ptr,
				data_ptr->needed_len,
				data_ptr->rc);
			break;
		}

		/* Responding binary for ACPI string type */
		case ESIF_DATA_BINARY:
		{
			str_len = esif_acpi_get_strlen(obj_ptr->type,
						       is_unicode,
						       obj_ptr->string.length);
			data_ptr->needed_len +=
				sizeof(union esif_data_variant) +
				str_len;
			if (data_ptr->needed_len <=
			    data_ptr->rsp_data_ptr->buf_len) {
				/*
				 * Knowing this string is ascii code, it's nicer
				 * to have the return type as string, so UF can
				 * print out ascii rather than binary dump.
				 */
				u8 *temp_ptr = NULL;
				bin.variant_ptr->string.type   =
					ESIF_DATA_STRING;
				bin.variant_ptr->string.length = str_len;
				temp_ptr = (u8 *)((acpi_string)bin.variant_ptr +
						 sizeof(union esif_data_variant));
				esif_acpi_memcpy(temp_ptr,
						 obj_ptr->string.pointer,
						 str_len);
			} else {
				data_ptr->rc = ESIF_E_NEED_LARGER_BUFFER;
			}
			ESIF_TRACE_DYN_UNPACK(
				"%s: ACPI_STRING %p len %u, ESIF_BINARY "
				"addr %p %s len (%u + %u), needed_len %u "
				"rc %d\n",
				ESIF_FUNC,
				obj_ptr->string.pointer,
				(u32)sizeof(union esif_data_variant),
				bin.variant_ptr,
				(char *)bin.variant_ptr,
				(u32)sizeof(obj_ptr->string),
				str_len,
				data_ptr->needed_len,
				data_ptr->rc);
			break;
		}

		default:
			data_ptr->rc = ESIF_E_ACPI_REQUEST_TYPE;
		}
	}	/* ACPI_TYPE_STRING */
	break;

	/*
	* REFERENCE, PROCESSOR and POWER object are Linux only, Windows doesn't
	* have such. Treat as OCTAL String Same As Windows Reports
	*/
	case ACPI_TYPE_LOCAL_REFERENCE:
	{
		struct acpi_buffer acpi_ref = {ACPI_ALLOCATE_BUFFER};
		acpi_status acpi_status;

		switch (data_ptr->rsp_data_ptr->type) {
		case ESIF_DATA_BINARY:
		{
			acpi_status = esif_ccb_acpi_get_name(
					obj_ptr->reference.handle,
					ACPI_FULL_PATHNAME,
					&acpi_ref);
			if (ESIF_FALSE ==
			    esif_ccb_has_acpi_failure(acpi_status, NULL)) {
#ifdef ESIF_ATTR_OS_LINUX
				/* Include terminator */
				str_len = (u32)strlen(acpi_ref.pointer) + 1;
#endif
				data_ptr->needed_len +=
					sizeof(union esif_data_variant)
					+ str_len;
				if (data_ptr->needed_len <=
				    data_ptr->rsp_data_ptr->buf_len) {
					u8 *temp_ptr = NULL;
					bin.variant_ptr->string.type   =
						ESIF_DATA_STRING;
					bin.variant_ptr->string.length =
						str_len;
					temp_ptr =
						(u8 *)((acpi_string)bin.variant_ptr
						+ sizeof(union esif_data_variant));

					esif_acpi_memcpy(temp_ptr,
							 acpi_ref.pointer,
							 str_len);
				} else {
					data_ptr->rc =
						ESIF_E_NEED_LARGER_BUFFER;
				}

				ESIF_TRACE_DYN_GET("%s: ACPI_TYPE_LOCAL_REFERENCE [ACPI actual_type %s handle %p] memcpy from %d to %d (size = %d + %d)\n",
						   ESIF_FUNC,
						   esif_acpi_type_str(obj_ptr->reference.actual_type),
						   obj_ptr->reference.handle,
						   offset,
						   data_ptr->needed_len,
						   (int)sizeof(obj_ptr->reference),
						   str_len);

				memstat_inc(&g_memstat.allocs);
				esif_ccb_free(acpi_ref.pointer);
			}
			break;
		}

		default:
			data_ptr->rc = ESIF_E_ACPI_REQUEST_TYPE;
		}
		break;
	}	/* ACPI_TYPE_LOCAL_REFERENCE */

	case ACPI_TYPE_PROCESSOR:
	{
		ESIF_TRACE_DYN_UNPACK("%s: Have ACPI Processor = %p\n",
				      ESIF_FUNC,
				      obj_ptr);
		switch (data_ptr->rsp_data_ptr->type) {
		case ESIF_DATA_BINARY:
		{
			str_len = sizeof(*obj_ptr);
			data_ptr->needed_len +=
				(sizeof(union esif_data_variant) +
				 str_len);
			if (data_ptr->needed_len <=
			    data_ptr->rsp_data_ptr->buf_len) {
				u8 *temp_str = NULL;
				bin.variant_ptr->string.type   =
					ESIF_DATA_BINARY;
				bin.variant_ptr->string.length =
					sizeof(*obj_ptr);
				temp_str = (u8 *)((acpi_string)bin.variant_ptr +
						 sizeof(union esif_data_variant));
				esif_acpi_memcpy(temp_str, obj_ptr,
						 sizeof(*obj_ptr));

				ESIF_TRACE_DYN_UNPACK(
					"%s: ACPI PROCESSOR:\nObject "
					"Type: 0x%08x proc_id: 0x%08x "
					"pblk_address: 0x%xlu pblk_length: "
					"0x%08x\n",
					ESIF_FUNC,
					obj_ptr->type,
					obj_ptr->processor.proc_id,
					(int)obj_ptr->processor.pblk_address,
					obj_ptr->processor.pblk_length);
			} else {
				data_ptr->rc = ESIF_E_NEED_LARGER_BUFFER;
			}

			ESIF_TRACE_DYN_UNPACK(
				"%s: ACPI_PROCESSOR, ESIF_BINARY/STRUCTURE "
				"rsp type %d, needed_len %d, rc %u\n",
				ESIF_FUNC,
				data_ptr->rsp_data_ptr->type,
				data_ptr->needed_len,
				data_ptr->rc);
		}

		default:
			data_ptr->rc = ESIF_E_ACPI_REQUEST_TYPE;
		}
		break;
	}	/* ACPI_TYPE_PROCESSOR: */

	case ACPI_TYPE_POWER:
	{
		ESIF_TRACE_DYN_UNPACK("%s: Have ACPI Power Resource = %p\n",
				      ESIF_FUNC,
				      obj_ptr);
		switch (data_ptr->rsp_data_ptr->type) {
		case ESIF_DATA_BINARY:
		{
			str_len = sizeof(*obj_ptr);
			data_ptr->needed_len +=
				(sizeof(union esif_data_variant) +
				 str_len);
			if (data_ptr->needed_len <=
			    data_ptr->rsp_data_ptr->buf_len) {
				u8 *temp_str;
				bin.variant_ptr->string.type   =
					ESIF_DATA_BINARY;
				bin.variant_ptr->string.length =
					sizeof(*obj_ptr);
				temp_str = (u8 *)((acpi_string)bin.variant_ptr +
						 sizeof(union esif_data_variant));
				esif_acpi_memcpy(temp_str, obj_ptr,
						 sizeof(*obj_ptr));
				ESIF_TRACE_DYN_UNPACK(
					"%s: ACPI POWER RESOURCE: Object Type: "
					"0x%08x system_level: 0x%08x "
					"resource_order: 0x%08x\n",
					ESIF_FUNC,
					obj_ptr->type,
					obj_ptr->power_resource.system_level,
					obj_ptr->power_resource.resource_order);
			} else {
				data_ptr->rc = ESIF_E_NEED_LARGER_BUFFER;
			}

			ESIF_TRACE_DYN_UNPACK(
				"%s: ACPI_POWER, ESIF_BINARY rsp type %d, "
				"needed_len %d, rc %u\n",
				ESIF_FUNC,
				data_ptr->rsp_data_ptr->type,
				data_ptr->needed_len,
				data_ptr->rc);
		}

		default:
			data_ptr->rc = ESIF_E_ACPI_REQUEST_TYPE;
		}
		break;
	}	/* ACPI_TYPE_POWER */

	default:
		ESIF_TRACE_DYN_UNPACK("%s: Unknown ACPI Object Type %d\n",
				      ESIF_FUNC,
				      obj_ptr->type);
		data_ptr->rc = ESIF_E_ACPI_REQUEST_TYPE;
		break;
	}
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

