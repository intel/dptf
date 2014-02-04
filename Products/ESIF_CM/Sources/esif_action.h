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

#ifndef _ESIF_ACTION_H_
#define _ESIF_ACTION_H_

#ifdef ESIF_ATTR_KERNEL

/* include enum esif_action_type, and lookup */
#include "esif.h"

/* Action */
struct esif_lp_action {
	enum esif_action_type  type;	/* Execution Method For This Primitive
					 * */
	u32  *p1_ptr;	/* Parameter 1                          */
	u32  *p2_ptr;	/* Parameter 2                          */
	u32  *p3_ptr;	/* Parameter 3                          */
	u32  *p4_ptr;	/* Parameter 4                          */
	u32  *p5_ptr;	/* Parameter 5                          */

	enum esif_action_type  (*get_type)(const struct esif_lp_action *
					   action_ptr);
	u32  (*get_p1_u32)(const struct esif_lp_action *action_ptr);
	u32  (*get_p2_u32)(const struct esif_lp_action *action_ptr);
	u32  (*get_p3_u32)(const struct esif_lp_action *action_ptr);
	u32  (*get_p4_u32)(const struct esif_lp_action *action_ptr);
	u32  (*get_p5_u32)(const struct esif_lp_action *action_ptr);
};

/* Forward Decleration */
struct esif_lp_primitive;
struct esif_lp;

enum esif_rc esif_execute_action(struct esif_lp_primitive *primitive_ptr,
				 const struct esif_lp_action *action_ptr,
				 struct esif_lp *lp_ptr,
				 struct esif_data *req_data_ptr,
				 struct esif_data *rsp_data_ptr);

/* ACPI */
enum esif_rc esif_action_acpi_init(void);
void esif_action_acpi_exit(void);

enum esif_rc esif_get_action_acpi(acpi_handle acpi_handle,
				  const u32 acpi_method,
				  const struct esif_data *req_data_ptr,
				  struct esif_data *rsp_data_ptr);

enum esif_rc esif_set_action_acpi(acpi_handle acpi_handle,
				  const u32 acpi_method,
				  struct esif_data *req_data_ptr);

/* CONSTANT */
enum esif_rc esif_action_const_init(void);
void esif_action_const_exit(void);

enum esif_rc esif_get_action_const(const u32 value,
				   const struct esif_data *req_data_ptr,
				   struct esif_data *rsp_data_ptr);

/* CODE */
struct esif_primitive_tuple;

enum esif_rc esif_action_code_init(void);
void esif_action_code_exit(void);

enum esif_rc esif_get_action_code(struct esif_lp *lp_ptr,
				  const struct esif_primitive_tuple *tuple_ptr,
				  const struct esif_lp_action *action_ptr,
				  const struct esif_data *req_data_ptr,
				  struct esif_data *rsp_data_ptr);

enum esif_rc esif_set_action_code(struct esif_lp *lp_ptr,
				  const struct esif_primitive_tuple *tuple_ptr,
				  const struct esif_lp_action *action_ptr,
				  const struct esif_data *req_data_ptr);

/* MBI */
enum esif_rc esif_action_mbi_init(void);
void esif_action_mbi_exit(void);

enum esif_rc esif_get_action_mbi(const u8 port,
				 const u8 reg,
				 const u8 bit_start,
				 const u8 bit_stop,
				 const struct esif_data *req_data_ptr,
				 struct esif_data *rsp_data_ptr);

enum esif_rc esif_set_action_mbi(const u8 port,
				 const u8 reg,
				 const u8 bit_start,
				 const u8 bit_stop,
				 const struct esif_data *req_data_ptr);

/* MMIO */
enum esif_rc esif_action_mmio_init(void);
void esif_action_mmio_exit(void);

enum esif_rc esif_get_action_mmio(const void __iomem *base_addr,
				  const u32 offset,
				  const u8 bit_start,
				  const u8 bit_stop,
				  const struct esif_data *req_data_ptr,
				  struct esif_data *rsp_data_ptr);

enum esif_rc esif_set_action_mmio(const void __iomem *base_addr,
				  const u32 offset,
				  const u8 bit_start,
				  const u8 bit_stop,
				  const struct esif_data *req_data_ptr);

/* MSR */
enum esif_rc esif_action_msr_init(void);
void esif_action_msr_exit(void);

enum esif_rc esif_get_action_msr(const u32 msr,
				 const u8 bit_start,
				 const u8 bit_stop,
				 const u32 cpus,
				 const u32 hint,
				 const struct esif_data *req_data_ptr,
				 struct esif_data *rsp_data_ptr);

enum esif_rc esif_set_action_msr(const u32 msr,
				 const u8 bit_start,
				 const u8 bit_stop,
				 const u32 cpus,
				 const u32 hint,
				 const struct esif_data *req_data_ptr);

/* VAR */
enum esif_rc esif_action_var_init(void);
void esif_action_var_exit(void);

enum esif_rc esif_get_action_var(const struct esif_data *var,
				 const struct esif_data *req_data_ptr,
				 struct esif_data *rsp_data_ptr);

enum esif_rc esif_set_action_var(struct esif_data *var,
				 const struct esif_data *req_data_ptr);

/* SYSTEM IO */
enum esif_rc esif_action_systemio_init(void);
void esif_action_systemio_exit(void);
enum esif_rc esif_get_action_systemio(const struct esif_data *req_data_ptr,
				      struct esif_data *rsp_data_ptr);

enum esif_rc esif_set_action_systemio(const struct esif_data *req_data_ptr,
				      struct esif_data *rsp_data_ptr);

#endif /* ESIF_ACTION_KERNEL */
#endif /* _ESIF_ACTION_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

