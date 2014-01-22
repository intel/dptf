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

#ifndef _ESIF_DSP_H_
#define _ESIF_DSP_H_

#define ESIF_DSP_NAME_LEN (12 + 1)

#include "esif_participant.h"

/* EDP Struct Referred By Both LF and UF */
struct edp_dir {
	unsigned int  signature;
	unsigned int  version;
	unsigned int  cpc_offset;
	unsigned int  fpc_offset;
};


#ifdef ESIF_ATTR_KERNEL

/* Compact Primitive Catalog */

#include "esif.h"
#include "esif_cpc.h"

#define THIS const struct esif_lp_dsp *THIS

/* Lower Framework DSP
 * Kernel version of DSP is a subset of the full DSP and is known as a Compact
 * Primitive Catalog. The CPC contains only the abosulte essentials for the
 * kernel driver.
 */
struct esif_lp_dsp {
/* private: */

	/* Pointers Will Point Into Raw CPC Block Of Memory */
	esif_flags_t   *capability_ptr;		/* Enhanced Capability */
	esif_string    code_ptr;		/* Code Short Name     */
	u8             *domain_count_ptr;	/* Domain Count        */
	struct domain  *domains_ptr;		/* Domains             */
	u8  *ver_major_ptr;	/* DSP Content Major   */
	u8  *ver_minor_ptr;	/* DSP Content Minor   */

	/* Raw Cannonical Data For DSP Pointeers  */
	struct esif_lp_cpc      *cpc_ptr;
	/* DSP Hash Table Will Contain Pointers Into CPC */
	struct esif_hash_table  *ht_ptr;
	struct esif_link_list   *algo_ptr;	/* Algorithm */
	struct esif_link_list   *evt_ptr;	/* Event */
	esif_ccb_lock_t         lock;		/* DSP Lock */

	void *table;    /* Add'l Static Or Dynamic Table(s) */ 
	u32 table_size; /* Table(s) Size Of Each */

/* public: */

	u32   (*get_id)(THIS);
	void  (*set_id)(THIS, u32 id);

	esif_string   (*get_code)(THIS);
	u8            (*get_domain_count)(THIS);
	esif_flags_t  (*get_domain_capability)(THIS, u8 domain_index);
	enum esif_domain_type (*get_domain_type)(THIS, u8 domain_index);
	u16           (*get_domain_id)(THIS, u8 domain_index);
	esif_string   (*get_domain_desc)(THIS, u8 domain_index);
	esif_string   (*get_domain_name)(THIS, u8 domain_index);

	u8            (*get_ver_major)(THIS);
	u8            (*get_ver_minor)(THIS);

	u32           (*get_temp_tc1) (THIS,
				       const enum esif_action_type action);
	u32           (*get_temp_tc2)(THIS, const enum esif_action_type action);

	enum esif_rc  (*insert_primitive)(THIS,
					  struct esif_cpc_primitive *
					  primitive_ptr);
	enum esif_rc  (*insert_algorithm)(THIS,
					  struct esif_cpc_algorithm *
					  algorithm_ptr);
	enum esif_rc  (*insert_event)(THIS, struct esif_cpc_event *event_ptr);
	struct esif_lp_primitive  * (*get_primitive)(THIS,
						     const struct
						     esif_primitive_tuple *
	tuple_ptr);
	struct esif_lp_action  * (*get_action)(THIS,
					       struct esif_lp_primitive *
					       primitive_ptr, u8 index);
	struct esif_cpc_algorithm  * (*get_algorithm)(THIS,
						      const enum
						      esif_action_type
						      action_type);

	u32 (*dsp_has_algorithm)( THIS, const enum esif_algorithm_type);

	struct esif_cpc_event  * (*get_event)(THIS, u32 event);
};

#undef THIS

/* Load DSP */
enum esif_rc esif_dsp_load (struct esif_lp *lp_ptr,
			    const struct esif_data *cpc_ptr);
void esif_dsp_unload (struct esif_lp *lp_ptr);

/* Init / Exit */
enum esif_rc esif_dsp_init (void);
void esif_dsp_exit (void);

#endif /* ESIF_ATTR_KERNEL */


#ifdef ESIF_ATTR_USER
#include "esif_uf_fpc.h"
#define THIS struct esif_up_dsp *THIS

#define MAX_DSP_MANAGER_ENTRY 128
#define ESIF_DSP_NAMESPACE              "dsp"	/* DSP DataVault Namespace
						 * (namespace.dv) */
#define ESIF_EDP_DV_PRIORITY    0		/* 0=EDP on disk overrides EDP
						 * in DataVault, 1=EDP in
						 * DataVault overrides EDP on
						 * disk */

/* DSP Manager Entry */
struct esif_uf_dme {
	struct esif_up_dsp    *dsp_ptr;	/* DSP Instance       */
	struct esif_ccb_file  *file_ptr;/* Optional ESIF File Instance */
	struct esif_fpc       *fpc_ptr; /* FPC buffer (0 if static) */
};

/* DSP Manager */
struct esif_uf_dm {
	UInt8  dme_count;			/* Current Reference Count */
	struct esif_uf_dme  dme[MAX_DSP_MANAGER_ENTRY];	/* Maximum Participants
							 *   */
	esif_ccb_lock_t     lock;			/* Package Manager Lock
							 *   */
};

/* Upper Framework DSP */
struct esif_up_dsp {
	/*
	 * PRIVATE
	 */

	/* Pointers Will Point Into PC Block Of Memory */
	esif_string  code_ptr;		/* Code Short Name          */
	esif_string  desc_ptr;		/* Description              */
	esif_string  type;		/* Unique Identifier        */
	UInt8        *bus_enum;		/* Bus Enumeration Type     */
	UInt8        *ver_major_ptr;	/* Content Major            */
	UInt8        *ver_minor_ptr;	/* Content Minor            */

	/* ACPI */
	esif_string  acpi_device;	/* ACPI Device e.g. INT3400 */
	esif_string  acpi_scope;	/* _SB.PCI0.B0D4            */
	esif_string  acpi_type;		/* ACPI Type                */

	/* PCI */
	esif_string  vendor_id;		/* Vendor ID 8086 For Intel */
	esif_string  device_id;		/* Device ID                */
	UInt8        *pci_bus;		/* PCI Bus                  */
	UInt8        *pci_bus_device;	/* PCI Device               */
	UInt8        *pci_function;	/* PCI Function             */

	/* Attributes */
	/* UInt32  *capability_ptr; / * Enhanced Capability * / */
	UInt32  *domain_count;	/* Domain Count */

	/* Raw Cannonical Data For DSP Pointeers  */
	struct esif_uf_pc       *pc_ptr;
	/* DSP Hash Table Will Contain Pointers Into CPC*/
	struct esif_hash_table  *ht_ptr;
	struct esif_link_list   *domain_ptr; /* Domain */
	struct esif_link_list   *cap_ptr;	/* Capability */
	struct esif_link_list   *algo_ptr;	/* Algorithm */
	struct esif_link_list   *evt_ptr;	/* Events */

	/* Lock */
	esif_ccb_lock_t  lock;		/* DSP Lock*/


	/*
	 * PUBLIC INTRERFACE
	 */
	UInt32  (*get_id)   (THIS);
	void    (*set_id)   (THIS, u32 id);
	UInt32  (*get_domain_count)(THIS);

	/* Description */
	esif_string  (*get_code)         (THIS);
	esif_string  (*get_desc)         (THIS);
	esif_guid_t  * (*get_type)         (THIS);
	UInt8        (*get_bus_enum)     (THIS);
	UInt8        (*get_ver_major)    (THIS);
	UInt8        (*get_ver_minor)    (THIS);

	/* ACPI */
	esif_string  (*get_acpi_device)  (THIS);
	esif_string  (*get_acpi_scope)   (THIS);
	UInt8        (*get_acpi_type)    (THIS);

	/* PCI */
	UInt32  (*get_device_id)        (THIS);
	UInt32  (*get_vendor_id)        (THIS);
	UInt8   (*get_pci_bus)          (THIS);
	UInt8   (*get_pci_bus_device)   (THIS);
	UInt8   (*get_pci_function)     (THIS);

	/* Attributes */
	UInt32        (*get_temp_tc1) (THIS, enum esif_action_type action);
	UInt32        (*get_temp_tc2) (THIS, enum esif_action_type action);

	enum esif_rc  (*insert_primitive)(THIS,
					  EsifFpcPrimitivePtr primitive_ptr);
	enum esif_rc  (*insert_algorithm)(THIS,
					  struct esif_fpc_algorithm *
					  algorithm_ptr);
	enum esif_rc  (*insert_domain)   (THIS,
					  struct esif_fpc_domain *domain_ptr);
	enum esif_rc  (*insert_event)   (THIS,
					 struct esif_fpc_event *event_ptr);

	EsifFpcPrimitivePtr  (*get_primitive)(THIS,
					      const struct esif_primitive_tuple
					      *tuple_ptr);
	EsifFpcActionPtr  (*get_action)   (THIS,
					   EsifFpcPrimitivePtr primitive_ptr,
					   u8 index);

	struct esif_fpc_algorithm  * (*get_algorithm)(THIS,
						      const enum
						      esif_action_type
	action_type);
	struct esif_fpc_event  * (*get_event_by_type)(THIS,
						      const enum
						      esif_event_type event_type);
	struct esif_fpc_event   * (*get_event_by_guid)(THIS,
						       const esif_guid_t guid);
	struct esif_fpc_domain  * (*get_domain)   (THIS, const u32 index);
};

#undef THIS
typedef struct esif_up_dsp EsifDsp, *EsifDspPtr, **EsifDspPtrLocation;


enum esif_uf_dm_query_type {
	ESIF_UF_DM_QUERY_TYPE_ACPI = 0,
	ESIF_UF_DM_QUERY_TYPE_PCI,
	ESIF_UF_DM_QUERY_TYPE_PLATFORM
};

/* Fields may be blank to indicate don't cares */
struct esif_uf_dm_query_acpi {
	esif_string  acpi_device;	/* Weight 8 */
	esif_string  acpi_type;		/* Weight 4 */
	esif_string  acpi_uid;		/* Weight 2 */
	esif_string  acpi_scope;	/* Weight 1 */
};

/* Fields may be blank to indicate don't cares */
struct esif_uf_dm_query_plat {
	esif_string  plat_type;	/* Weight 2 */
	esif_string  plat_name;	/* Weight 1 */
};

/* Query */
esif_string esif_uf_dm_query (enum esif_uf_dm_query_type query_type,
			      void *qry_ptr);
struct esif_up_dsp *esif_uf_dm_select_dsp_by_code (esif_string code);
esif_string esif_uf_dm_select_dsp (eEsifParticipantOrigin origin, void *piPtr);

/* Init / Exit */
enum esif_rc EsifDspInit (void);
void EsifDspExit (void);

/* DSP Manager Init */
enum esif_rc EsifDspMgrInit (void);

/* DSP Manager Exit */
void EsifDspMgrExit (void);

#endif	/* ESIF_ATTR_USER */
#endif /* _ESIF_DSP_H_ */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

