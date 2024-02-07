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

#pragma once

#include "esif_ccb_rc.h"
#include "esif_sdk_iface_kpe.h"
#include "esif_ccb_lock.h"
#include "esif_ccb_sem.h"
#include "esif_lf_actm.h"

/*
 * GENERIC DEFINITIONS
 */
/* Maximum Driver Entries */
// TESTING
#define DRVM_DRIVERS_MAX 32
#define DRVM_ITERATOR_MARKER 'DRVM'

#define drv_handle_t u8

#define drvm_iterator_t struct drvm_iterator

typedef enum esif_rc(*kpe_get_handler)( /* KPE Action GET handler */
	const void *context_ptr,
	const u32 p1,
	const u32 p2,
	const u32 p3,
	const u32 p4,
	const u32 p5,
	const struct esif_data *request_ptr,
	struct esif_data *response_ptr
	);

/*
 * TYPE DECLARATIONS
 */

struct drv_act_ctx {

	enum esif_rc(*kpe_get_handler)( /* KPE Action GET handler */
		const void *context_ptr,
		const u32 p1,
		const u32 p2,
		const u32 p3,
		const u32 p4,
		const u32 p5,
		const struct esif_data *request_ptr,
		struct esif_data *response_ptr
		);

	enum esif_rc(*kpe_set_handler)( /* KPE Action SET handler */
		const void *context_ptr,
		const u32 domain_handle,
		const u32 p1,
		const u32 p2,
		const u32 p3,
		const u32 p4,
		const u32 p5,
		const struct esif_data *request_ptr
		);

	void *kpe_ctx_ptr;
};

struct esif_drv_obj {
	struct esif_driver_iface kpe_iface;
	action_handle_t action_handle; /* Used to unregister the action */
	u8 action_registered;
	struct drv_act_ctx kpe_act_ctx;

	u32 ref_count;
	esif_ccb_low_priority_thread_lock_t drv_lock;
	u8 marked_for_delete;
	esif_ccb_event_t delete_event;
};

struct esif_drvm_obj {
	u32 entry_count;
	struct esif_drv_obj *drv_objs[DRVM_DRIVERS_MAX];
	esif_ccb_low_priority_thread_lock_t drvm_lock;
};

struct drvm_iterator {
	u8 handle;
	u8 ref_taken;
	int marker;
};

struct esif_drv_to_part_binding {
	enum esif_action_type action_type; /* Action exported by the KPE */
	u32   part_type;		   /* Participant Type */
};


/*
 * PUBLIC INTERFACE FUNCTION PROTOTYPES
 */
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
