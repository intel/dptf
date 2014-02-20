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

#ifndef _ESIF_CCB_ACPI_H_
#define _ESIF_CCB_ACPI_H_

#include "esif_data.h"		/* ESIF Data Buffer                 */
#include "esif_debug.h"

#ifdef ESIF_ATTR_KERNEL

#ifdef ESIF_ATTR_OS_LINUX
#define esif_ccb_acpi_get_name acpi_get_name
#endif
#ifdef ESIF_ATTR_OS_WINDOWS
#define ACPI_TYPE_INTEGER          ACPI_METHOD_ARGUMENT_INTEGER	/* 0 */
#define ACPI_TYPE_STRING           ACPI_METHOD_ARGUMENT_STRING	/* 1 */
#define ACPI_TYPE_BUFFER           ACPI_METHOD_ARGUMENT_BUFFER	/* 2 */
#define ACPI_TYPE_PACKAGE          ACPI_METHOD_ARGUMENT_PACKAGE	/* 3 */
#define ACPI_TYPE_LOCAL_REFERENCE  0xfffffff1	/* Non-existing value */
#define ACPI_TYPE_POWER            0xfffffff2	/* Non-existing value */
#define ACPI_TYPE_PROCESSOR        0xfffffff3	/* Non-existing value */

#define AE_OK                      0x0000 /* Follow Linux AE Error Code */
#define AE_ERROR                   0x0001
#define AE_NOT_FOUND               0x0005

#define do_div(x, y)   x = x / y
#define esif_ccb_acpi_get_name(handle, pathname, ref) \
		(ESIF_E_UNSUPPORTED_ACTION_TYPE)

#define ACPI_FULL_PATHNAME

#include <acpiioct.h>	/* ACPI  */

typedef WDFDEVICE acpi_handle;	/* We base ACPI on Intel */
typedef u64 acpi_size;		/* ACPI Size             */
typedef u32 acpi_status;	/* ACPI Status Value     */
typedef char *acpi_string;	/* ACPI String           */

typedef u16 acpi_object_type;	/* General Type */
typedef u16 esif_acpi_p_type;	/* Package Type and Length */

/* Emulate Intel ACPI-CA acpi_object */
#define ACPI_ALLOCATE_BUFFER (-1)

struct acpi_buffer {
	int   length; /* Length in bytes of the buffer -1 to allocate memory */
	void  *pointer;	/* pointer to buffer */
};

	#pragma pack(push, 1)
union acpi_object {
	acpi_object_type  type;	/* MS ARGUMENT:Type */
	struct {
		acpi_object_type  type; /* MS ARGUMENT:Type */
		u16  length;	    /* MS ARGUMENT:DataLength */
		u32  value;	/* MS ARGUMENT:UNION.Argument */
	} integer;

	struct {
		acpi_object_type  type;	/* MS ARGUMENT:Type */
		u16  length; /* MS ARGUMENT:DataLength */
		u8   *pointer; /* MS ARGUMENT:Data  */
	} string;

	struct {
		acpi_object_type  type; /* MS ARGUMENT:Type */
		u16  length; /* MS ARGUMENT:DataLength */
		u8   *pointer; /* MS ARGUMENT:Data */
	} buffer;

	struct {
		acpi_object_type  type;	 /* MS ARGUMENT:Type */
		u16  count; /* MS ARGUMENT:DataLength NOT COUNT */
		union acpi_object  *elements;	/* MS ARGUMENT:Data
						 *                */
	} package;

	struct { /* MS Not Used */
		acpi_object_type  type;
		acpi_object_type  actual_type;
		acpi_handle       handle;
	} reference;

	struct { /* MS Not Used */
		acpi_object_type  type;
		u32  proc_id;
		u64  pblk_address;
		u32  pblk_length;
	} processor;

	struct {/* MS Not Used */
		acpi_object_type  type;
		u32  system_level;
		u32  resource_order;
	} power_resource;
};

	#pragma pack(pop)

struct acpi_object_list {
	u32  count;
	union acpi_object *pointer;
};

#include "win/esif_lf_os_win.h"	 /* ACPI Helpers        */
#endif

/* ACPI Type String */
static ESIF_INLINE char *esif_acpi_type_str(u32 type)
{
#define CREATE_ACPI_TYPE(t, str)  case t: str = #t; break;

	char *str = ESIF_NOT_AVAILABLE;
	switch (type) {
		CREATE_ACPI_TYPE(ACPI_TYPE_INTEGER, str);
		CREATE_ACPI_TYPE(ACPI_TYPE_STRING, str);
		CREATE_ACPI_TYPE(ACPI_TYPE_BUFFER, str);
		CREATE_ACPI_TYPE(ACPI_TYPE_PACKAGE, str);
		CREATE_ACPI_TYPE(ACPI_TYPE_POWER, str);
		CREATE_ACPI_TYPE(ACPI_TYPE_PROCESSOR, str);
#ifdef ESIF_ATTR_OS_LINUX
		CREATE_ACPI_TYPE(ACPI_TYPE_ANY, str);
		CREATE_ACPI_TYPE(ACPI_TYPE_FIELD_UNIT, str);
		CREATE_ACPI_TYPE(ACPI_TYPE_DEVICE, str);
		CREATE_ACPI_TYPE(ACPI_TYPE_EVENT, str);
		CREATE_ACPI_TYPE(ACPI_TYPE_METHOD, str);
		CREATE_ACPI_TYPE(ACPI_TYPE_MUTEX, str);
		CREATE_ACPI_TYPE(ACPI_TYPE_REGION, str);
		CREATE_ACPI_TYPE(ACPI_TYPE_THERMAL, str);
		CREATE_ACPI_TYPE(ACPI_TYPE_BUFFER_FIELD, str);
		CREATE_ACPI_TYPE(ACPI_TYPE_DDB_HANDLE, str);
		CREATE_ACPI_TYPE(ACPI_TYPE_DEBUG_OBJECT, str);
#endif
	}
	return str;
}


static ESIF_INLINE acpi_status esif_ccb_acpi_evaluate_object(
	acpi_handle handle,
	acpi_string acpi_method,
	struct acpi_object_list *in_params_ptr,
	struct acpi_buffer *return_buffer_ptr
	)
{
	acpi_status rc = 0;

	int orig_length = 0;
	if (NULL != return_buffer_ptr)
		orig_length = return_buffer_ptr->length;

	rc = acpi_evaluate_object(handle,
				  acpi_method,
				  in_params_ptr,
				  return_buffer_ptr);

	if (NULL != return_buffer_ptr && NULL != return_buffer_ptr->pointer &&
	    ACPI_ALLOCATE_BUFFER == orig_length) {
		memstat_inc(&g_memstat.allocs); /* Increment in Linux Only Not *Windows! */
	}
#ifdef ESIF_ATTR_OS_LINUX
	return rc;

#endif

/* Align Windows NTSTATUS to Linux AE error code */
#ifdef ESIF_ATTR_OS_WINDOWS
	if (NT_SUCCESS(rc))
		return AE_OK;

	switch (rc) {
	case STATUS_OBJECT_NAME_NOT_FOUND:
		return AE_NOT_FOUND;

	/*
	 * TODO: Need a better error mapping, for now, have all Win ACPI errors
	 * map to *AE_ERROR
	 */
	case STATUS_ACPI_INVALID_OPCODE:
	case STATUS_ACPI_STACK_OVERFLOW:
	case STATUS_ACPI_ASSERT_FAILED:
	case STATUS_ACPI_INVALID_INDEX:
	case STATUS_ACPI_INVALID_ARGUMENT:
	case STATUS_ACPI_FATAL:
	case STATUS_ACPI_INVALID_ARGTYPE:
	case STATUS_ACPI_INVALID_OBJTYPE:
	case STATUS_ACPI_INVALID_TARGETTYPE:
	case STATUS_ACPI_INCORRECT_ARGUMENT_COUNT:
	case STATUS_ACPI_ADDRESS_NOT_MAPPED:
	case STATUS_ACPI_INVALID_EVENTTYPE:
	case STATUS_ACPI_HANDLER_COLLISION:
	case STATUS_ACPI_INVALID_DATA:
	case STATUS_ACPI_INVALID_REGION:
	case STATUS_ACPI_INVALID_ACCESS_SIZE:
	case STATUS_ACPI_ACQUIRE_GLOBAL_LOCK:
	case STATUS_ACPI_ALREADY_INITIALIZED:
	case STATUS_ACPI_NOT_INITIALIZED:
	case STATUS_ACPI_INVALID_MUTEX_LEVEL:
	case STATUS_ACPI_MUTEX_NOT_OWNED:
	case STATUS_ACPI_RS_ACCESS:
	case STATUS_ACPI_INVALID_TABLE:
	case STATUS_ACPI_REG_HANDLER_FAILED:
	case STATUS_ACPI_POWER_REQUEST_FAILED:
	default:
		return AE_ERROR;
	}
#endif
}


/* Get Argument Count */
static ESIF_INLINE u32 esif_ccb_acpi_arg_count(struct acpi_buffer *buf_ptr)
{
	u32 count = 0;
#ifdef ESIF_ATTR_OS_LINUX
	if (NULL == buf_ptr)
		return 0;

	count = 1;
#endif
#ifdef ESIF_ATTR_OS_WINDOWS
	ACPI_EVAL_OUTPUT_BUFFER *aeob_ptr = NULL;
	if (NULL == buf_ptr)
		return 0;

	aeob_ptr = (ACPI_EVAL_OUTPUT_BUFFER *)buf_ptr->pointer;
	if (aeob_ptr->Signature == ACPI_EVAL_OUTPUT_BUFFER_SIGNATURE)
		count = aeob_ptr->Count;
#endif
	return count;
}


/* Get ACPI Object */
static ESIF_INLINE union acpi_object
*esif_ccb_acpi_arg_first(struct acpi_buffer *buf_ptr)
{
#ifdef ESIF_ATTR_OS_LINUX
	if (NULL == buf_ptr)
		return NULL;

	return (union acpi_object *)buf_ptr->pointer;

#endif
#ifdef ESIF_ATTR_OS_WINDOWS
	ACPI_EVAL_OUTPUT_BUFFER *aeob_ptr = NULL;
	if (NULL == buf_ptr)
		return NULL;

	aeob_ptr = (ACPI_EVAL_OUTPUT_BUFFER *)buf_ptr->pointer;
	if (aeob_ptr->Signature == ACPI_EVAL_OUTPUT_BUFFER_SIGNATURE)
		return (union acpi_object *)&aeob_ptr->Argument[0];

	return NULL;

#endif
}


static ESIF_INLINE union acpi_object *esif_acpi_arg_next(
	union acpi_object *obj_ptr)
{
#ifdef ESIF_ATTR_OS_LINUX
	return NULL;

#endif

#ifdef ESIF_ATTR_OS_WINDOWS
	PACPI_METHOD_ARGUMENT ama_ptr = NULL;
	if (NULL == obj_ptr)
		return NULL;

	ama_ptr = (ACPI_METHOD_ARGUMENT *)obj_ptr;
	ama_ptr = ACPI_METHOD_NEXT_ARGUMENT(ama_ptr);
	return (union acpi_object *)ama_ptr;

#endif
}


static ESIF_INLINE enum esif_rc esif_handle_osc_buf(
	struct acpi_buffer *output,
	struct esif_data *req_data_ptr
	)
{
	struct esif_data_complex_osc *osc_ptr =
		(struct esif_data_complex_osc *)req_data_ptr->buf_ptr;
	acpi_status rc = 0;

#ifdef ESIF_ATTR_OS_LINUX
	union acpi_object *out_obj = output->pointer;

	osc_ptr->status = *((u32 *)out_obj->buffer.pointer);
	rc = *((u32 *)out_obj->buffer.pointer) & ~(1 << 0);
#endif
#ifdef ESIF_ATTR_OS_WINDOWS
	PACPI_EVAL_OUTPUT_BUFFER aeob_ptr = NULL;
	PACPI_METHOD_ARGUMENT method_arg  = NULL;
	PULONG temp = NULL;

	if (NULL == output)
		return ESIF_E_ACPI_EVAL_FAILURE;

	aeob_ptr        = (ACPI_EVAL_OUTPUT_BUFFER *)output->pointer;
	method_arg      = aeob_ptr->Argument;
	temp            = (PULONG)method_arg->Data;
	osc_ptr->status = *temp;
	rc = osc_ptr->status;
#endif
	if (rc) {
		/*
		 * OSC_REQUEST_ERROR(1), OSC_INVALID_UUID_ERROR(4),
		 *OSC_INVALID_REVISION_ERROR(8),
		 * OSC_CAPABILITIES_MASK_ERROR(16)
		 */
		ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_ACPI,
			       ESIF_TRACE_CATEGORY_ERROR,
			       "%s: _OSC set error, rc 0x%x\n",
			       ESIF_FUNC,
			       rc);
		if (rc & 0x2) {
			ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_ACPI,
				       ESIF_TRACE_CATEGORY_ERROR,
				       "_OSC error - Platform Support _OSC but unable to process _OSC request\n");
		}
		if (rc & 0x4) {
			ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_ACPI,
				       ESIF_TRACE_CATEGORY_ERROR,
				       "_OSC error - Platform failed _OSC reason: Unrecognized UUID\n");
		}
		if (rc & 0x8) {
			ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_ACPI,
				       ESIF_TRACE_CATEGORY_ERROR,
				       "_OSC error - Platform failed _OSC reason: Unrecognized revision\n");
		}
		if (rc & 0x10) {
			ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_ACPI,
				       ESIF_TRACE_CATEGORY_ERROR,
				       "_OSC error - Platform failed _OSC reason: Unrecognized capability mask\n");
		}
		return ESIF_E_ACPI_EVAL_FAILURE;
	} else {
		return ESIF_OK;
	}
}


/*
 * _OSC Linux Format (32-bit)
 *  <0x00>  {03 00 00 00, 10 00 00 00, PO IN TE R0} 00 00 00 00  (4b padding)
 *  <0x10>  {01 00 00 00, 01 00 00 00  00 00 00 00} 00 00 00 00  (4b padding)
 *  <0x20>  {01 00 00 00, 01 00 00 00  00 00 00 00} 00 00 00 00  (4b padding)
 *  <0x30>  {01 00 00 00, 08 00 00 00  PO IN TE R0} 00 00 00 00  (4b padding)
 *
 * TODO: need to confirm!
 * _OSC Windows Format: (32-bite)
 *  <0x00>  {03 00, 10 00, 16 BY TE -- GU ID 00 00 00 00 00 00 00 00 00 00}
 *  <0x18>  {01 00, 04 00, 01 00 00 00}
 *  <0x20>  {04 00, 02 00 00 00}
 *  <0x28>  {03 00, 08 00, 00 00 00 00 00 00 00 00}
 *
 */
static ESIF_INLINE void esif_create_acpi_buffer(
	const u32 acpi_method,
	struct acpi_object_list *arg_list,
	struct esif_data *req_data_ptr
	)
{
	union acpi_object *acpi_obj = arg_list->pointer;

	switch (acpi_method) {
	case 'CSO_':
	{
		struct esif_data_complex_osc *osc =
			(struct esif_data_complex_osc *)req_data_ptr->buf_ptr;
/* First Argument - copy GUID from request buffer */
		acpi_obj->buffer.type    = ACPI_TYPE_BUFFER;
		acpi_obj->buffer.length  = 16;	/* Sizeof GUID */
#ifdef ESIF_ATTR_OS_LINUX
		acpi_obj->buffer.pointer = (u8 *)osc->guid;
		acpi_obj =
			(union acpi_object *)((u8 *)acpi_obj +
					     sizeof(union acpi_object));
#endif
#ifdef ESIF_ATTR_OS_WINDOWS

		/* On Windows, Use &acpi_obj->buffer.pointer Do NOT use
		 *esif_acpi_memcpy!!! */
		acpi_obj->buffer.pointer =
			((u8 *)acpi_obj + (sizeof(acpi_obj->buffer) - 
			                   sizeof(acpi_obj->buffer.pointer)));
		esif_ccb_memcpy(&acpi_obj->buffer.pointer, &osc->guid, 16);
		acpi_obj =
			(union acpi_object *)((u8 *)acpi_obj +
					     sizeof(acpi_obj->buffer) -
					     sizeof(acpi_obj->buffer.pointer)
					     + 16);
#endif

/* Second Argument - copy revision from request buffer */
		acpi_obj->integer.type  = ACPI_TYPE_INTEGER;
		acpi_obj->integer.value = osc->revision;
#ifdef ESIF_ATTR_OS_LINUX
		acpi_obj =
			(union acpi_object *)((u8 *)acpi_obj +
					     sizeof(union acpi_object));
#endif
#ifdef ESIF_ATTR_OS_WINDOWS

		acpi_obj->integer.length = sizeof(acpi_obj->integer.value);
		acpi_obj =
			(union acpi_object *)((u8 *)acpi_obj +
					     sizeof(acpi_obj->integer));
#endif

/* Third Argument - copy count from request buffer */
		acpi_obj->integer.type  = ACPI_TYPE_INTEGER;
		acpi_obj->integer.value = osc->count;
#ifdef ESIF_ATTR_OS_LINUX
		acpi_obj =
			(union acpi_object *)((u8 *)acpi_obj +
					     sizeof(union acpi_object));
#endif
#ifdef ESIF_ATTR_OS_WINDOWS
		acpi_obj->integer.length = sizeof(acpi_obj->integer.value);
		acpi_obj =
			(union acpi_object *)((u8 *)acpi_obj +
					     sizeof(acpi_obj->integer));
#endif

/* Forth Argument - copy capability from request buffer */
		acpi_obj->buffer.type   = ACPI_TYPE_BUFFER;
		acpi_obj->buffer.length = (u16)(osc->count * sizeof(u32));

#ifdef ESIF_ATTR_OS_LINUX
		acpi_obj->buffer.pointer = (u8 *)&osc->status;
#endif
#ifdef ESIF_ATTR_OS_WINDOWS

		/* On Windows, Use &acpi_obj->buffer.pointer!!! Do NOT use
		 *esif_acpi_memcpy!!! */
		acpi_obj->buffer.pointer =
			((u8 *)acpi_obj + (sizeof(acpi_obj->buffer) - 
			                   sizeof(acpi_obj->buffer.pointer)));
		esif_ccb_memcpy(&acpi_obj->buffer.pointer,
				&osc->status,
				acpi_obj->buffer.length);
#endif
	}
	break;

	default:
	{
		u8 *buf_ptr = (u8 *)req_data_ptr->buf_ptr;
		u32 i;

		/*
		 * buf_ptr points the requested ESIF data buffer in void*,
		 * counter tells how many, and in_args is real data
		 * presentation for underlying ACPI driver
		 */
		for (i = 0; i < arg_list->count; i++) {
			acpi_obj->integer.type = ACPI_TYPE_INTEGER;
#ifdef ESIF_ATTR_OS_WINDOWS
			acpi_obj->integer.length = sizeof(acpi_obj->integer.value);
#endif

			switch (req_data_ptr->type) {
			case ESIF_DATA_UINT8:
				acpi_obj->integer.value = *((u8 *)buf_ptr);
				buf_ptr += sizeof(u8);
				break;

			case ESIF_DATA_UINT16:
				buf_ptr += sizeof(u16);
				break;

			case ESIF_DATA_UINT32:
			case ESIF_DATA_TEMPERATURE:
				acpi_obj->integer.value = *((u32 *)buf_ptr);
				buf_ptr += sizeof(u32);
				break;

			case ESIF_DATA_UINT64:
			case ESIF_DATA_FREQUENCY:
				/* For 32-bit Windows, it's type ULONG 4-byte */
				acpi_obj->integer.value = *((u32 *)buf_ptr);
				buf_ptr += sizeof(u64);
				break;

			default:
				break;
			}

			/* Note: don't do arg_p++, it doesn't work out as
			 *expected on Windows */
			acpi_obj = (union acpi_object *)
				((u8 *)acpi_obj + sizeof(acpi_obj->integer));
		}
	}
	break;
	}
	return;
}


/*
 * ACPI Package Processing
 */

#define ESIF_NO_ACPI_TYPE       0xffff
#define ESIF_TABLE_NO_REVISION  0xffff

/* Context For Binary Data Parse / Recursive */
struct esif_action_data {
	struct esif_data  *rsp_data_ptr;
	u32  needed_len;
	u16  revision, rows, cols, need_revision;
	u16  first_type, second_type, third_type;
	enum esif_rc rc;
};


static ESIF_INLINE void esif_acpi_check_revision(
	struct esif_action_data *data_ptr)
{
/*
* Maybe it looks like a very strange logic, but this is all due to Linux and
* Windows have very different data layout and design, particularly in package
* type making it impossible to have one set of abstraction.
*/
#ifdef ESIF_ATTR_OS_WINDOWS

	if (data_ptr->first_type == ACPI_TYPE_INTEGER &&
	    data_ptr->second_type == ACPI_TYPE_PACKAGE) {
		data_ptr->rows--;
	} else {
		data_ptr->revision = ESIF_TABLE_NO_REVISION;
	}
#endif
#ifdef ESIF_ATTR_OS_LINUX

	if (data_ptr->first_type == ACPI_TYPE_PACKAGE &&
	    data_ptr->second_type == ACPI_TYPE_INTEGER &&
	    data_ptr->third_type == ACPI_TYPE_PACKAGE) {
		data_ptr->rows--;
	} else {
		data_ptr->revision = ESIF_TABLE_NO_REVISION;
	}
#endif
}


static ESIF_INLINE void esif_acpi_set_cols(
	struct esif_action_data *data_ptr,
	const u32 count
	)
{
#ifdef ESIF_ATTR_OS_WINDOWS
	if (data_ptr->cols == 1)
		data_ptr->cols = (u16)count;
#endif
#ifdef ESIF_ATTR_OS_LINUX
	if (data_ptr->rows == 1)
		data_ptr->rows = (u16)count;
	else
		data_ptr->cols = (u16)count;
#endif
}


static ESIF_INLINE u32 esif_ccb_acpi_pkg_count(union acpi_object *obj_ptr)
{
#ifdef ESIF_ATTR_OS_LINUX
	return obj_ptr->package.count;

#endif
#ifdef ESIF_ATTR_OS_WINDOWS

	/*
	 * package.count in Windows is actually DataLength, not number of
	 * elements, so we need to walk thru the entire ACPI_METHOD_ARGUMENT to
	 * count how many are there
	 */
	union acpi_object *pkg_ptr =
		(union acpi_object *)&obj_ptr->package.elements;
	int pkg_len, obj_len = obj_ptr->package.count;
	u32 count = 0;

	while (obj_len > 0) {
		/* Calc the size of each package and move to the next one */
		/* make sure if the count is less than 4 to account for the */
		/* union. A good example would be a short string like ma\0 */
		pkg_len  = (2 * sizeof(USHORT)) + 
			(pkg_ptr->package.count < sizeof(ULONG) ? sizeof(ULONG): 
			pkg_ptr->package.count);

		pkg_ptr  = (union acpi_object *)((u8 *)pkg_ptr + pkg_len);
		obj_len -= pkg_len;
		count++;
		NO_ESIF_DEBUG(
			"%s: <object addr %p type %d len %d> "
			"<pkg addr %p type %d len %d> count %d\n",
			ESIF_FUNC,
			obj_ptr,
			obj_ptr->package.type,
			obj_ptr->package.count,
			pkg_ptr,
			pkg_ptr->package.type,
			pkg_ptr->package.count,
			count);
	}
	if (obj_len == 0) {
		return count;
	} else {
		/* Don't count the last one which could be extra padding */
		return count - 1;
	}
#endif
}


static ESIF_INLINE union acpi_object
*esif_ccb_acpi_pkg_next(
	union acpi_object *package,
	union acpi_object *method,
	u32 item
	)
{
#ifdef ESIF_ATTR_OS_LINUX
	return (union acpi_object *)&package->package.elements[item];

#endif
#ifdef ESIF_ATTR_OS_WINDOWS
	u32 offset;

	UNREFERENCED_PARAMETER(package);

	/*
	 * Windows: first element is actually Data[0], while the rest are
	 * (2 * sizeof(USHORT) + DataLength)
	 */
	if (0 == item)
		offset = 2 * sizeof(esif_acpi_p_type);
	else
		offset = (2 * sizeof(esif_acpi_p_type)) + method->string.length;

	NO_ESIF_DEBUG("%s - orignal obj addr %p offset %ld, new addr %p\n",
		      ESIF_FUNC,
		      method,
		      offset,
		      (u8 *)obj_ptr + offset);
	return (union acpi_object *)((u8 *)method + offset);

#endif
}


/*
 * ACPI Status/Failure
 */

static ESIF_INLINE int esif_ccb_has_acpi_failure(
	acpi_status status,
	struct acpi_buffer *buf_ptr
	)
{
#ifdef ESIF_ATTR_OS_WINDOWS
	if (!NT_SUCCESS(status))
		return ESIF_TRUE;

	/* buf_ptr could be NULL for SET operation */
	if (buf_ptr && (buf_ptr->pointer == NULL || buf_ptr->length <= 0))
		return ESIF_TRUE;

#endif
#ifdef ESIF_ATTR_OS_LINUX
	if (ACPI_FAILURE(status))
		return ESIF_TRUE;
#endif
	return ESIF_FALSE;
}

#endif /* ESIF_ATTR_KERNEL */
#endif /* _ESIF_CCB_ACPI_H_ */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
