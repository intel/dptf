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

#include "esif_ccb.h"
#include "esif_ccb_rc.h"
#include "esif_sdk_iface_kpe.h"
#include "esif_sdk_action_type.h"


/*
 * GENERAL DEFINITIONS
 */
#define KPE_DOMAIN_D0 '0D'

/*
 * FUNCTION PROTOTYPES
 */
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Called by KPE when ready to receive accessor/event calls and before the
 * registration call to DPTF is performed
 */
enum esif_rc kpe_init(
	void *drvr_ctx_ptr,
	void **kpe_handle_ptr
	);

/*
 * Disables and unitializes the KPE after all current access have completed
 */
void kpe_uninit(void *kpe_handle);

/*
 * Called by the KPE to indicate the device is exiting D0 or the
 * interface should be disabled for a reason specific to the driver
 */
enum esif_rc kpe_suspend(
	void *kpe_handle
	);

/*
 * Called by the KPE to indicate the device is returning to D0 or the
 * interface should be re-enabled after being suspended
 */
enum esif_rc kpe_resume(
	void *kpe_handle
	);

/*
 * Used to send events to DPTF
 */
enum esif_rc kpe_send_event( 
	const void *kpe_handle,
	const enum esif_event_type event_type,
	const u32 domain_handle,
	const struct esif_data *data_ptr
	);

/*
 * Disables further events from going to DPTF
 */
void kpe_disable_send_events(
	void *kpe_handle
	);

/*
 * Returns a pointer to the KPE interface structure
 */
struct esif_driver_iface *kpe_get_iface(void *kpe_handle);

#ifdef __cplusplus
}
#endif
