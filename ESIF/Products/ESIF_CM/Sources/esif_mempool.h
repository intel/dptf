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

#ifndef _ESIF_MEMPOOL_H_
#define _ESIF_MEMPOOL_H_

    #include "esif.h"

/* Memory Pools */
enum esif_mempool_type {
	ESIF_MEMPOOL_TYPE_DSP = 0,	/* Device Support Package */
	ESIF_MEMPOOL_TYPE_HASH,		/* Hash Table             */
	ESIF_MEMPOOL_TYPE_HASH2,	/* Hash Table2 TODO: This is temp   */
	ESIF_MEMPOOL_TYPE_PM,		/* Participant Manager    */
	ESIF_MEMPOOL_TYPE_MAX		/* Max                    */
};

/*
 * Windows Uses a 32 Bit Value To Represent A Tag. Linux allows for a name
 * we acommodate both heree in addition Windows does not have a number authority
 * so we resuse permutations of ESIF to avoid name space collisions.
 * For Windows use Poolmon or ESIF Tools to Monitor
 * For Linux use Slabtop or ESIF tools to Monitor (@ Hutts To Top Of Slabtop)
 * Note you may bave to boot the kernel with slub debug on or Linux will
 * consolidate same size cahches.
 */

/* Driver */
#define ESIF_MEMPOOL_DRIVER_LF        ('fise')	/* Bit 0000 Lower Framework */
#define ESIF_MEMPOOL_DRIVER_ACPI      ('Fise')	/* Bit 0001 ACPI Driver */
#define ESIF_MEMPOOL_DRIVER_CPU       ('fIse')	/* Bit 0010 CPU Driver */
#define ESIF_MEMPOOL_DRIVER_PCH       ('FIse')	/* Bit 0011 PCH Driver */
#define ESIF_MEMPOOL_DRIVER_PLAT      ('fiSe')	/* Bit 0100 PLAT Driver */
#define ESIF_MEMPOOL_DRIVER_RESERVED1 ('FiSe')	/* Bit 0101 Unused */
#define ESIF_MEMPOOL_DRIVER_RESERVED2 ('fISe')	/* Bit 0110 Unused */
#define ESIF_MEMPOOL_DRIVER_RESERVED3 ('FISe')	/* Bit 0111 Unused */

/* ESIF Framework */
#define ESIF_MEMPOOL_FW_PM            ('fisE')	/* Bit 1000 Participant */
#define ESIF_MEMPOOL_FW_AVAIL         ('FisE')	/* Bit 1001 Available for use */
#define ESIF_MEMPOOL_FW_DSP           ('fIsE')	/* Bit 1010 Dsp */
#define ESIF_MEMPOOL_FW_HASH          ('FIsE')	/* Bit 1011 Hash */
#define ESIF_MEMPOOL_FW_QUEUE         ('fiSE')	/* Bit 1100 Queue */
#define ESIF_MEMPOOL_FW_LIST          ('FiSE')	/* Bit 1101 List */
#define ESIF_MEMPOOL_FW_LIST_NODE     ('fISE')	/* Bit 1110 List Node */
#define ESIF_MEMPOOL_FW_MALLOC        ('FISE')	/* Bit 1111 Gen Malloc Pool */

static ESIF_INLINE char *esif_mempool_str(u32 pool_tag)
{
	switch (pool_tag) {
	ESIF_CASE(ESIF_MEMPOOL_DRIVER_LF, "@esif_lf_driver");
	ESIF_CASE(ESIF_MEMPOOL_DRIVER_ACPI, "@esif_acpi_driver");
	ESIF_CASE(ESIF_MEMPOOL_DRIVER_CPU, "@esif_cpu_driver");
	ESIF_CASE(ESIF_MEMPOOL_DRIVER_PCH, "@esif_pch_driver");
	ESIF_CASE(ESIF_MEMPOOL_DRIVER_PLAT, "@esif_plat_driver");
	ESIF_CASE(ESIF_MEMPOOL_DRIVER_RESERVED1, "@esif_rsv1_driver");
	ESIF_CASE(ESIF_MEMPOOL_FW_HASH, "@esif_hash_table_cache");
	ESIF_CASE(ESIF_MEMPOOL_FW_PM, "@esif_pm_cache");
	ESIF_CASE(ESIF_MEMPOOL_FW_AVAIL, "@esif_available");
	ESIF_CASE(ESIF_MEMPOOL_FW_DSP, "@esif_dsp_cache");
	ESIF_CASE(ESIF_MEMPOOL_FW_QUEUE, "@esif_queue_cache");
	ESIF_CASE(ESIF_MEMPOOL_FW_LIST, "@esif_list_cache");
	ESIF_CASE(ESIF_MEMPOOL_FW_LIST_NODE, "@esif_list_node_cache");
	ESIF_CASE(ESIF_MEMPOOL_FW_MALLOC, "@esif_malloc_cache");
	}
	return ESIF_NOT_AVAILABLE;
}


#ifdef __cplusplus
extern "C" {
#endif

extern struct esif_ccb_mempool *g_mempool[ESIF_MEMPOOL_TYPE_MAX];

#ifdef __cplusplus
}
#endif

#endif /* _ESIF_MEMPOOL_H_ */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
