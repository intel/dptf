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

#ifndef _ESIF_LF_H_
#define _ESIF_LF_H_

#include "esif.h"
#include "esif_participant.h"

/* Registration */
enum esif_rc esif_lf_register_participant(struct esif_participant_iface *pi_ptr);
enum esif_rc esif_lf_unregister_participant(
			struct esif_participant_iface *pi_ptr);
void esif_lf_unregister_all_participants(void);

/* Instrumentation */
enum esif_rc esif_lf_instrument_participant(struct esif_lp *lp_ptr);
enum esif_rc esif_lf_instrument_capability(struct esif_lp_domain *lpd_ptr);

void esif_lf_uninstrument_participant(struct esif_lp *lp_ptr);
void esif_lf_uninstrument_capability(struct esif_lp_domain *lpd_ptr);

/* Events */
enum esif_rc esif_lf_send_event(const struct esif_participant_iface *pi_ptr,
				const struct esif_event *event_ptr);
enum esif_rc esif_lf_send_all_events_in_queue_to_uf_by_ipc(void);

enum esif_rc esif_lf_event(struct esif_participant_iface *pi_ptr,
			   enum esif_event_type type,
			   u16 domain,
			   struct esif_data *data_ptr);

/* Init / Exit */
enum esif_rc esif_lf_init(esif_flags_t debug_mask);
void esif_lf_exit(void);

/* Optional OS Init/Exit. Call before esif_ccb_malloc & after esif_ccb_free */
void esif_lf_os_init(void);
void esif_lf_os_exit(void);

#endif /* _ESIF_LF_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
