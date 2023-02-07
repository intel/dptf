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

#ifndef _ESIF_LF_PRIMITIVE_H_
#define _ESIF_LF_PRIMITIVE_H_

#include "esif.h"

#define ESIF_PRIMITIVE_VERSION 1
#define ESIF_PRIMITIVE_DOMAIN_D0 '0D'
#define ESIF_PRIMITIVE_DOMAIN_D1 '1D'
#define ESIF_PRIMITIVE_DOMAIN_D2 '2D'

/* Primitive Opcodes */
enum esif_primitive_opcode {
	ESIF_PRIMITIVE_OP_GET = 1,
	ESIF_PRIMITIVE_OP_SET
};

/* Primitive Opcodes String */
static ESIF_INLINE esif_string esif_primitive_opcode_str(
	enum esif_primitive_opcode opcode)
{
	switch (opcode) {
	ESIF_CASE_ENUM(ESIF_PRIMITIVE_OP_GET);
	ESIF_CASE_ENUM(ESIF_PRIMITIVE_OP_SET);
	}
	return ESIF_NOT_AVAILABLE;
}


#pragma pack(push, 1)
struct esif_primitive_tuple {
	u16  id;	/* GET_TEMP                 */
	u16  domain;	/* DO, NA, ...              */
	u16  instance;	/* ff: no instance required */
};


struct esif_lp_primitive {
	struct esif_primitive_tuple  tuple;	/* PRIMITIVE Tuple */
	enum esif_primitive_opcode   opcode;	/* PRIMITIVE Operation Type */
	u8  action_count;	/* Number Of Actions For This Primitive */
};

#define ESIF_PRIM_ACT_SEL_FLAG_EXCLUDE		1
#define ESIF_PRIM_ACT_SEL_FLAG_INDEX_VALID	2
#define ESIF_PRIM_ACT_SEL_FLAG_TYPE_VALID	4

struct esif_primitive_action_selector {
	esif_flags_t flags;
	u8 index;
	esif_action_type_t type;
};

struct esif_specific_action_request {
	struct esif_primitive_tuple tuple;
	struct esif_primitive_action_selector selector;
	struct esif_data *req_ptr;
	struct esif_data *rsp_ptr;
};

#ifdef ESIF_ATTR_USER
typedef struct esif_primitive_tuple EsifPrimitiveTuple, *EsifPrimitiveTuplePtr;
typedef struct esif_primitive_action_selector EsifPrimitiveActionSelector, *EsifPrimitiveActionSelectorPtr;
typedef struct esif_specific_action_request EsifSpecificActionRequest, *EsifSpecificActionRequestPtr;
#endif


#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

struct esif_lp;
struct esif_ipc_primitive;

#ifdef ESIF_ATTR_KERNEL

void esif_execute_ipc_primitive(struct esif_ipc_primitive *prim_ptr);

enum esif_rc esif_execute_primitive(
	struct esif_lp *lp_ptr,
	const struct esif_primitive_tuple *tuple_ptr,
	const struct esif_data *req_data_ptr,
	struct esif_data *rsp_data_ptr,
	const u16 *action_index_ptr
);

/*
 * Simple helper functions to execute primitives that takes no special
 * parameters.
 */
enum esif_rc esif_get_simple_primitive(
	struct esif_lp *lp_ptr,
	u16 id,
	u16 domain,
	u16 instance,
	enum esif_data_type esif_type,
	void *buffer_ptr,
	u32 buffer_size
);

enum esif_rc esif_set_simple_primitive(
	struct esif_lp *lp_ptr,
	u16 id,
	u16 domain,
	u16 instance,
	enum esif_data_type esif_type,
	void *buffer_ptr,
	u32 buffer_size
);

char *esif_primitive_domain_str(
	u16 domain,
	char *str_ptr
	);

u32 esif_ipc_primitive_get_data_len(struct esif_ipc_primitive *primitive_ptr);

#else
char *esif_primitive_domain_str(
	u16 domain,
	char *str,
	u8 str_len
	);

#endif
#ifdef __cplusplus
}
#endif

#endif /* _ESIF_LF_PRIMITIVE_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
