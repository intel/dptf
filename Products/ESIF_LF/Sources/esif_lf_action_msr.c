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
#define INIT_DEBUG       0
#define GET_DEBUG        1
#define SET_DEBUG        2

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_MSR, \
		       INIT_DEBUG, \
		       format, \
		       ##__VA_ARGS__)
#define ESIF_TRACE_DYN_GET(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_MSR, \
		       GET_DEBUG, \
		       format, \
		       ##__VA_ARGS__)
#define ESIF_TRACE_DYN_SET(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_MSR, \
		       SET_DEBUG, \
		       format, \
		       ##__VA_ARGS__)

/* Lock */
static esif_ccb_lock_t g_esif_action_msr_lock;


/* Binary Object - Use the Same Logic As ACPI */
union esif_binary_object {
	u8  *buf_ptr;
	union esif_data_variant *variant_ptr;
};


/* Compare */
static ESIF_INLINE u8 msr_has_same_val(
	const u32 msr,
	const u64 bit_mask
	)
{
	u32 cpu       = 0;
	u32 rc        = 0;
	u32 once      = TRUE;
	u64 curr_val  = ~(0UL);
	u64 first_val = ~(0UL);
	u64 cpu_mask  = 0;

	cpu_mask = esif_ccb_get_online_cpu();

	/* Compare If All Active CPUs Have A Same value */
	for (cpu = 0; cpu < sizeof(cpu) * 8; cpu++)
		if ((cpu_mask & (((u64)0x1UL) << cpu))) {
			rc = esif_ccb_read_msr((u8)cpu, msr, &curr_val);
			if (ESIF_OK != rc)
				return FALSE;

			curr_val = curr_val & bit_mask;
			if (once) {
				first_val = curr_val;
				once      = FALSE;
			} else if (first_val != curr_val) {
				return FALSE;
			}
		}

	/* Ok To Be True Even No cpu_mask. MSR Read Func Will Handle Error */
	return TRUE;
}

/* Get */
enum esif_rc esif_get_action_msr(
	const u32 msr,
	const u8 bit_from,
	const u8 bit_to,
	const u32 cpus,
	const u32 hint,
	const struct esif_data *req_data_ptr,
	struct esif_data *rsp_data_ptr
	)
{
	union esif_binary_object bin;
	enum esif_rc rc = ESIF_OK;
	u64 val         = 0, bit_mask = 0, cpu_mask = 0;
	u32 i, cpu, has_ok = 0, has_failed = 0, needed_len = 0;
	u8 cpu_str[8];

	UNREFERENCED_PARAMETER(req_data_ptr);
	ESIF_TRACE_DYN_GET(
		"%s: read msr %d bit_from %d to %d cpus 0x%x hint %x "
		"req_type %s len %d rsp_type %s len %d\n",
		ESIF_FUNC,
		msr,
		bit_from,
		bit_to,
		cpus,
		hint,
		esif_data_type_str(req_data_ptr->type),
		req_data_ptr->buf_len,
		esif_data_type_str(rsp_data_ptr->type),
		rsp_data_ptr->buf_len);

	bin.buf_ptr = (u8 *)rsp_data_ptr->buf_ptr;

	/*
	 * cpus =  0, Single CPU (CPU-0)
	 * cpus =  7, Multiple CPUs (CPU-0, 1 and 2)
	 * cpus = -1, Multiple CPUs (Use all active ones)
	 */
	if (cpus == 0) {
		/* Mask Bits */
		for (bit_mask = 0ULL, i = bit_from; i <= bit_to; i++)
			bit_mask |= (1ULL << i);

		/*
		 * When Hint And UINT Response Value Are Given, Make Sure All
		 * CPUs Have The Same Value
		 */
		if (((int) hint > 0) && !msr_has_same_val(msr, bit_mask))
			return ESIF_E_MSR_MULTI_VALUES;

		/* Use CPU0 By Default To Read 64-Bit Current MSR Value */
		rc = esif_ccb_read_msr(0, msr, &val);
		if (ESIF_OK != rc)
			return ESIF_E_MSR_IO_FAILURE;

		val = val & bit_mask;
		val = val >> bit_from;

		switch (rsp_data_ptr->type) {
		/*
		 * Expect UF placed a generic reponse type for all similar
		 * ones, sych as TIME, BIT -> U32, so there is no need to
		 * list them all here.
		 */
		case ESIF_DATA_UINT8:
			rsp_data_ptr->data_len = sizeof(u8);
			if (rsp_data_ptr->buf_len >= sizeof(u8))
				*((u8 *)rsp_data_ptr->buf_ptr) = (u8)val;
			else
				rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
			break;

		case ESIF_DATA_UINT16:
			rsp_data_ptr->data_len = sizeof(u16);
			if (rsp_data_ptr->buf_len >= sizeof(u16))
				*((u16 *)rsp_data_ptr->buf_ptr) = (u16)val;
			else
				rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
			break;

		case ESIF_DATA_UINT32:
			rsp_data_ptr->data_len = sizeof(u32);
			if (rsp_data_ptr->buf_len >= sizeof(u32))
				*((u32 *)rsp_data_ptr->buf_ptr) = (u32)val;
			else
				rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
			break;

		case ESIF_DATA_UINT64:
			rsp_data_ptr->data_len = sizeof(u64);
			if (rsp_data_ptr->buf_len >= sizeof(u64))
				*((u64 *)rsp_data_ptr->buf_ptr) = (u64)val;
			else
				rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
			break;

		case ESIF_DATA_BINARY:
			needed_len += sizeof(union esif_data_variant);
			if (needed_len <= rsp_data_ptr->buf_len) {
				bin.variant_ptr->integer.type  =
					ESIF_DATA_UINT64;
				bin.variant_ptr->integer.value = (u64)val;
				bin.buf_ptr = bin.buf_ptr +
					sizeof(union esif_data_variant);
			} else {
				rc = ESIF_E_NEED_LARGER_BUFFER;
			}

			/* Update Respond Data Len */
			rsp_data_ptr->data_len = needed_len;
			break;

		default:
			rc = ESIF_E_UNSUPPORTED_RESULT_DATA_TYPE;
			break;
		}
		ESIF_TRACE_DYN_GET(
			"%s: Single read msr 0x%x bit_from %d bit_to %d "
			"len %d rc %d val 0x%llx\n",
			ESIF_FUNC,
			msr,
			bit_from,
			bit_to,
			needed_len,
			rc,
			val);
	} else {
		/* The Only Accepted Rsp Data Type For Multi-Thread MSR Read Is
		 *BINARY */
		if (rsp_data_ptr->type != ESIF_DATA_BINARY)
			return ESIF_E_NEED_BINARY_BUFFER;

		if ((int)cpus > 0)
			cpu_mask = cpus;
		else
			cpu_mask = esif_ccb_get_online_cpu();

		/* Multi-Thread Affinity Read */
		for (cpu = 0; cpu < sizeof(cpus) * 8; cpu++)
			if ((cpu_mask & (((u64)0x1UL) << cpu))) {
				/* On CPU i, Read The Current MSR Value */
				rc = esif_ccb_read_msr((u8)cpu, msr, &val);

				if (ESIF_OK == rc) {
					for (bit_mask = 0ULL, i = bit_from; i <= bit_to; i++)
						bit_mask |= (1ULL << i);

					val = val & bit_mask;
					val = val >> bit_from;

					/* Need Two Variant Data, One For CPU
					 * Name, The Other For Value. */
					needed_len +=
						(sizeof(union esif_data_variant) * 2) +
						       sizeof(cpu_str);
					if (needed_len <= rsp_data_ptr->buf_len) {
						/* Format: {{U64, cpu-number},
						 *{U64, msr-val}} */
						bin.variant_ptr->integer.type = ESIF_DATA_UINT64;
						bin.variant_ptr->integer.value = (u64)cpu;
						bin.buf_ptr = bin.buf_ptr + sizeof(union esif_data_variant);
						bin.variant_ptr->integer.type  = ESIF_DATA_UINT64;
						bin.variant_ptr->integer.value = (u64)val;
						bin.buf_ptr = bin.buf_ptr + sizeof(union esif_data_variant);
					}
					has_ok++;
				} else {
					has_failed++;
				}
			}


		/* Update Respond Data Len */
		rsp_data_ptr->data_len = needed_len;

		/*
		 * Return OK only if all threads read are OK.
		 * IO_FAILURE if thread can read; otherwise, MSR_AFFINITY if
		 * some threads can do affinity read.
		 */
		/*
		 * TODO: Lifu - ESIF_I_MSR_AFFINITY will make esif_uf
		 * NOT to show any data
		 */
		if (needed_len <= rsp_data_ptr->buf_len)
			rc = (has_ok) ? ((has_failed == 0) ? ESIF_OK : ESIF_OK)
				      : ESIF_E_MSR_IO_FAILURE;
		else
			rc = ESIF_E_NEED_LARGER_BUFFER;

		ESIF_TRACE_DYN_GET(
			"%s: Multi read msr 0x%x bit_from %d bit_to %d "
			" len %d rc %d passed %d failed %d\n",
			ESIF_FUNC,
			msr,
			bit_from,
			bit_to,
			needed_len,
			rc,
			has_ok,
			has_failed);
	}

	return rc;
}


/* Set */
enum esif_rc esif_set_action_msr(
	const u32 msr,
	const u8 bit_from,
	const u8 bit_to,
	const u32 cpus,
	const u32 hint,
	const struct esif_data *req_data_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	u64 saved_val   = 0, val = 0, orig_val = 0, bit_mask = 0, cpu_mask = 0;
	u32 i, has_ok = 0, has_failed = 0;

	UNREFERENCED_PARAMETER(hint);
	ESIF_TRACE_DYN_SET(
		"%s: write msr %x bit_from %d to %d cpu_mask 0x%x "
		" hint %x req_type %s len %d\n",
		ESIF_FUNC,
		msr,
		bit_from,
		bit_to,
		cpus,
		hint,
		esif_data_type_str(req_data_ptr->type),
		req_data_ptr->buf_len);


	switch (req_data_ptr->type) {
	case ESIF_DATA_UINT8:
		if (req_data_ptr->buf_len >= sizeof(u8))
			val = *((u8 *)req_data_ptr->buf_ptr);
		else
			rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		break;

	case ESIF_DATA_UINT16:
		if (req_data_ptr->buf_len >= sizeof(u16))
			val = *((u16 *)req_data_ptr->buf_ptr);
		else
			rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		break;

	case ESIF_DATA_UINT32:
	case ESIF_DATA_POWER:
	case ESIF_DATA_TEMPERATURE:
	case ESIF_DATA_TIME:
		if (req_data_ptr->buf_len >= sizeof(u32))
			val = *((u32 *)req_data_ptr->buf_ptr);
		else
			rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		break;

	case ESIF_DATA_UINT64:
		if (req_data_ptr->buf_len >= sizeof(u64))
			val = *((u64 *)req_data_ptr->buf_ptr);
		else
			rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		break;

	/*
	 * OK to have binary data - only for one item with u64 type.
	 * We don't yet support multiple data entries for different CPU threads.
	 */
	case ESIF_DATA_BINARY:
		if (req_data_ptr->buf_len >= sizeof(u64))
			val = *((u64 *)req_data_ptr->buf_ptr);
		else
			rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		break;

	default:
		rc = ESIF_E_UNSUPPORTED_REQUEST_DATA_TYPE;
		break;
	}

	if (ESIF_OK != rc)
		return rc;

	/* Mask Bits */
	for (bit_mask = 0ULL, i = bit_from; i <= bit_to; i++)
		bit_mask |= (1ULL << i);

	/*
	 * cpus =  0, Single CPU (CPU-0)
	 * cpus =  7, Multiple CPUs (CPU-0, 1 and 2)
	 * cpus = -1, Multiple CPUs (Use all active ones)
	 */
	if (cpus == 0) {
		/* Use CPU0 By Default To Read 64-Bit Current MSR Value */
		esif_ccb_write_lock(&g_esif_action_msr_lock);
		rc = esif_ccb_read_msr((u8)0, msr, &orig_val);

		if (ESIF_OK == rc) {
			/* Shift Bits, OR Other Original Bits */
			orig_val &= ~(bit_mask);
			val = (val << bit_from) | orig_val;

			/* Write MSR 64-Bit Always */
			rc = esif_ccb_write_msr((u8)0, msr, val);
			if (ESIF_OK != rc)
				rc = ESIF_E_MSR_IO_FAILURE;
		} else {
			rc = ESIF_E_MSR_IO_FAILURE;
		}
		esif_ccb_write_unlock(&g_esif_action_msr_lock);
		ESIF_TRACE_DYN_GET(
			"%s: Single write msr 0x%x bit_from %d to %d "
			"val 0x%llu rc %d\n",
			ESIF_FUNC,
			msr,
			bit_from,
			bit_to,
			val,
			rc);
	} else {
		if ((int)cpus > 0)
			cpu_mask = cpus;
		else
			cpu_mask = esif_ccb_get_online_cpu();

		/* Multi-Thread Affinity Write */
		for (i = 0, saved_val = val; i < sizeof(cpus) * 8; i++)
			if ((cpu_mask & (((u64)0x1UL) << i))) {
				/* On CPU i, Read Current 64-Bit MSR Value */
				rc = esif_ccb_read_msr((u8)i, msr, &orig_val);
				if (ESIF_OK == rc) {
					/* Shift Bits, OR Other Original Bits */

					/* Re-store Original Value */
					val = saved_val;
					orig_val &= ~(bit_mask);
					val = (val << bit_from) | orig_val;

					/* Write MSR 64-Bit Always */
					rc = esif_ccb_write_msr((u8)i,
								 msr,
								 val);
					if (ESIF_OK == rc)
						has_ok++;
					else
						has_failed++;
				} else {
					has_failed++;
				}
			}


		/*
		 * Return OK only if all threads read are OK.
		 * IO_FAILURE if thread can read; otherwise, MSR_AFFINITY if
		 * some threads can do affinity read.
		 */
		rc = (has_ok) ? ((has_failed == 0) ? ESIF_OK : ESIF_OK)
			      : ESIF_E_MSR_IO_FAILURE;

		ESIF_TRACE_DYN_SET(
			"%s: Multi write msr 0x%x bit_from %d to %d "
			"val 0x%llu rc %d pass %d fail %d\n",
			ESIF_FUNC,
			msr,
			bit_from,
			bit_to,
			val,
			rc,
			has_ok,
			has_failed);
	}
	return rc;
}


/* Init */
enum esif_rc esif_action_msr_init(void)
{
	ESIF_TRACE_DYN_INIT("%s: Initialize MSR Action\n", ESIF_FUNC);
	esif_ccb_lock_init(&g_esif_action_msr_lock);
	return ESIF_OK;
}


/* Exit */
void esif_action_msr_exit(void)
{
	esif_ccb_lock_uninit(&g_esif_action_msr_lock);
	ESIF_TRACE_DYN_INIT("%s: Exit MSR Action\n", ESIF_FUNC);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
