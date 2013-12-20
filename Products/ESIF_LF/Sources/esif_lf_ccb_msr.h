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

#ifndef _ESIF_CCB_MSR_H_
#define _ESIF_CCB_MSR_H_

#ifdef ESIF_ATTR_KERNEL

#ifdef ESIF_ATTR_OS_WINDOWS
/* Windows Safe Function HELPERS */

/* Read MSR Helper  returns 0 on success */
static ESIF_INLINE int esif_ccb_read_cpu_msr_safe(
	const u8 cpu,
	IN u32 p_msrAddress,
	u32 *l,
	u32 *h
	)
{
	__int64 int64Result = 0;
	int status = 0;
	KAFFINITY kaUserThreadAffinity = 0xff;

	/* Save active processors and set affinity */
	kaUserThreadAffinity = KeQueryActiveProcessors();
	KeSetSystemAffinityThreadEx(((ULONG_PTR)1 << cpu));

	/*
	 * EXCEPTION HANDLER
	 * If we read from an invalid MSR this will create an exception we catch
	 * it here to avoid the dreaded BSOD and use the exception to return an
	 * error code to our caller.
	 */
	try {
		int64Result = __readmsr((int)p_msrAddress);
	}
	except(EXCEPTION_EXECUTE_HANDLER) {
		status = 1;
		goto exit;
	}
	*l = (ULONG)(int64Result & 0xFFFFFFFF);
	*h = (LONG)((int64Result >> 32) & 0xFFFFFFFF);

exit:
	KeRevertToUserAffinityThreadEx(kaUserThreadAffinity);
	return status;
}


/* Write MSR Helper  returns 0 on success */
static ESIF_INLINE int esif_ccb_write_cpu_msr_safe(
	const u8 cpu,
	IN ULONG p_msrAddress,
	u32 l,
	u32 h
	)
{
	__int64 int64Content = 0;
	int status = 0;
	KAFFINITY kaUserThreadAffinity = 0xff;

	int64Content  = h;
	int64Content  = int64Content << 32;
	int64Content |= l;

	/* Save active processors and set affinity */
	kaUserThreadAffinity = KeQueryActiveProcessors();
	KeSetSystemAffinityThreadEx(((ULONG_PTR)1 << cpu));

	/*
	 * EXCEPTION HANDLER
	 * If we read from an invalid MSR this will create an exception we catch
	 * it here to avoid the dreaded BSOD and use the exception to return an
	 * error code to our caller.
	 */
	try {
		__writemsr(p_msrAddress, int64Content);
	}
	except(EXCEPTION_EXECUTE_HANDLER) {
		status = 1;
	}
	KeRevertToUserAffinityThreadEx(kaUserThreadAffinity);
	return status;
}


static ESIF_INLINE u64 esif_ccb_get_online_cpu(void)
{
	return (u64)KeQueryActiveProcessors();
}


#endif

#ifdef ESIF_ATTR_OS_LINUX
static ESIF_INLINE u64 esif_ccb_get_online_cpu(void)
{
	u64 online_cpus = 0;
	int i;
	for_each_online_cpu(i)
	online_cpus = online_cpus | (1UL << i);

	return online_cpus;
}


#endif

/* Read MSR */
static ESIF_INLINE int esif_ccb_read_msr(
	const u8 cpu,
	const u32 msr,
	u64 *val_ptr
	)
{
	int rc = 0;
	u32 l  = 0;
	u32 h  = 0;
#ifdef ESIF_ATTR_OS_LINUX
	rc = rdmsr_safe_on_cpu(cpu, msr, &l, &h);
#endif
#ifdef ESIF_ATTR_OS_WINDOWS
	rc       = esif_ccb_read_cpu_msr_safe(cpu, msr, &l, &h);
#endif
	*val_ptr = (((u64)h << 32) | l);
	return rc;
}


/* Write MSR */
static ESIF_INLINE int esif_ccb_write_msr(
	const u8 cpu,
	const u32 msr,
	const u64 val
	)
{
	u32 l = (u32)val, h = (u32)(val >> 32);
#ifdef ESIF_ATTR_OS_LINUX
	return wrmsr_safe_on_cpu(cpu, msr, l, h);

#endif
#ifdef ESIF_ATTR_OS_WINDOWS
	return esif_ccb_write_cpu_msr_safe(cpu, msr, l, h);

#endif
}


#endif /* ESIF_ATTR_KERNEL */
#endif /* _ESIF_CCB_MSR_H_ */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
