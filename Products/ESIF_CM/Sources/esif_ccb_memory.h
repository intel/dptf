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

#ifndef _ESIF_CCB_MEMORY_H_
#define _ESIF_CCB_MEMORY_H_

#include "esif_ccb_lock.h"

#ifdef ESIF_ATTR_USER
# include "esif.h"

/* Enable Detailed Memory Tracing? */
# ifdef ESIF_ATTR_MEMTRACE
#	include "esif_ccb_atomic.h"
	struct memalloc_s {
		void			*mem_ptr;
		size_t			size;
		const char		*func;
		const char		*file;
		int			line;
		struct memalloc_s	*next;
	};
	struct memtrace_s {
		esif_ccb_lock_t		lock;
		atomic_t		allocs;
		atomic_t		frees;
		struct memalloc_s	*allocated;
	};

	extern struct memtrace_s g_memtrace;
# endif
#endif

/* memstat counters interface */
#if defined(ESIF_ATTR_MEMSTAT_NOLOCK)
# define memstat_inc(intptr)		((*(intptr))++)
# define memstat_read(intptr)		(*(intptr))
# define memstat_set(intptr, val)	(*(intptr) = (val))
#elif defined(ESIF_ATTR_MEMSTAT_ATOMIC)
/* TODO: Replace Inlines with these after converting g_memstat u32 to atomic_t */
# define memstat_inc(intptr)		atomic_inc(intptr)
# define memstat_read(intptr)		atomic_read(intptr)
# define memstat_set(intptr, val)	atomic_set(intptr, val)
#else
extern esif_ccb_lock_t g_memstat_lock;

static ESIF_INLINE void memstat_inc(u32 *intptr)
{
	esif_ccb_write_lock(&g_memstat_lock);
	(*(intptr))++;
	esif_ccb_write_unlock(&g_memstat_lock);
}

static ESIF_INLINE u32 memstat_read(u32 *intptr)
{
	u32 rc;
	esif_ccb_read_lock(&g_memstat_lock);
	rc = *(intptr);
	esif_ccb_read_unlock(&g_memstat_lock);
	return rc;
}
static ESIF_INLINE void memstat_set(u32 *intptr, u32 val)
{
	esif_ccb_write_lock(&g_memstat_lock);
	*(intptr) = val;
	esif_ccb_write_unlock(&g_memstat_lock);
}
#endif

/* Memory Copy */
static ESIF_INLINE void esif_ccb_memcpy(
	void *dest_ptr,
	const void *src_ptr,
	size_t size
	)
{
	MEMORY_DEBUG("%s: dest = %p src = %p size = %u\n",
		     ESIF_FUNC,
		     dest_ptr,
		     src_ptr,
		     size);
#ifdef ESIF_ATTR_KERNEL
#ifdef ESIF_ATTR_OS_WINDOWS
	memcpy_s(dest_ptr, size, src_ptr, size);
#else
	memcpy(dest_ptr, src_ptr, size);
#endif
#endif

#ifdef ESIF_ATTR_USER
#ifdef ESIF_ATTR_OS_WINDOWS
	memcpy_s(dest_ptr, size, src_ptr, size);
#else
	memcpy(dest_ptr, src_ptr, size);
#endif
#endif
}


/* Memory Move */
static ESIF_INLINE void esif_ccb_memmove(
	void *dest_ptr,
	const void *src_ptr,
	size_t size
	)
{
	MEMORY_DEBUG("%s: dest = %p src = %p size = %u\n",
		     ESIF_FUNC,
		     dest_ptr,
		     src_ptr,
		     size);
#ifdef ESIF_ATTR_KERNEL
#ifdef ESIF_ATTR_OS_WINDOWS
	memmove_s(dest_ptr, size, src_ptr, size);
#else
	memmove(dest_ptr, src_ptr, size);
#endif
#endif

#ifdef ESIF_ATTR_USER
#ifdef ESIF_ATTR_OS_WINDOWS
	memmove_s(dest_ptr, size, src_ptr, size);
#else
	memmove(dest_ptr, src_ptr, size);
#endif
#endif
}


/* Memory Set */
static ESIF_INLINE void *esif_ccb_memset(
	void *s_ptr,
	int c,
	size_t count
	)
{
	MEMORY_DEBUG("%s: dest = %p value = %02x size = %u\n",
		     ESIF_FUNC,
		     s_ptr,
		     c,
		     count);
	return memset(s_ptr, c, count);
}

#ifdef ESIF_ATTR_MEMTRACE

extern void *esif_memtrace_alloc(void *old_ptr, size_t size, const char *func, const char *file, int line);
extern void  esif_memtrace_free(void *mem_ptr);
extern void  esif_memtrace_init();
extern void  esif_memtrace_exit();

#define esif_ccb_malloc(size)			esif_memtrace_alloc(0, size, __FUNCTION__, __FILE__, __LINE__)
#define esif_ccb_realloc(old_ptr, size)	esif_memtrace_alloc(old_ptr, size, __FUNCTION__, __FILE__, __LINE__)
#define esif_ccb_free(mem_ptr)			esif_memtrace_free(mem_ptr)

#else

/* Memory Allocate */
static ESIF_INLINE void
*esif_ccb_malloc(size_t size)
{
	void *mem_ptr = NULL;
#ifdef ESIF_ATTR_KERNEL
	/* Linux */
    #ifdef ESIF_ATTR_OS_LINUX
	mem_ptr = kzalloc(size, GFP_ATOMIC);
    #endif
	/* Windows */
    #ifdef ESIF_ATTR_OS_WINDOWS
	mem_ptr = ExAllocatePoolWithTag(NonPagedPool,
					size,
					ESIF_MEMPOOL_FW_MALLOC);
	if (NULL != mem_ptr)
		esif_ccb_memset(mem_ptr, 0, size);
    #endif

    #ifndef ESIF_ATTR_OS_LINUX_DRIVER
	memstat_inc(&g_memstat.allocs);
	#endif
#endif /* ESIF_ATTR_KERNEL */
#ifdef ESIF_ATTR_USER
	mem_ptr = malloc(size);
	if (NULL != mem_ptr)
		esif_ccb_memset(mem_ptr, 0, size);
#endif /* ESIF_ATTR_USER */

	MEMORY_DEBUG("%s: buf %p size = %u\n", ESIF_FUNC, mem_ptr, size);
	return mem_ptr;
}


/* Memory Free */
static ESIF_INLINE
void esif_ccb_free(void *mem_ptr)
{
	MEMORY_DEBUG("%s: buf %p\n", ESIF_FUNC, mem_ptr);
#ifdef ESIF_ATTR_KERNEL
	/* Linux */
    #ifdef ESIF_ATTR_OS_LINUX
	kfree(mem_ptr);
    #endif

	/* Windows */
    #ifdef ESIF_ATTR_OS_WINDOWS
	ExFreePoolWithTag(mem_ptr, ESIF_MEMPOOL_FW_MALLOC);
	mem_ptr = NULL;
    #endif

    #ifndef ESIF_ATTR_OS_LINUX_DRIVER
	memstat_inc(&g_memstat.frees);
    #endif
#endif /* ESIF_ATTR_KERNEL */
#ifdef ESIF_ATTR_USER
	free(mem_ptr);
#endif /* ESIF_ATTR_USER */
}


/* Memory Reallocate */
static ESIF_INLINE void *esif_ccb_realloc(
	void *mem_ptr,
	size_t new_size
	)
{
#ifdef ESIF_ATTR_USER
	void *old_ptr = mem_ptr;
#endif
	MEMORY_DEBUG("%s: buf %p, new_size=%lld\n", ESIF_FUNC, mem_ptr,
		     (long long)new_size);
#ifdef ESIF_ATTR_KERNEL
	UNREFERENCED_PARAMETER(mem_ptr);
	UNREFERENCED_PARAMETER(new_size);
	return 0;	/* Not Implemented in Kernel space */

#endif /* ESIF_ATTR_KERNEL */
#ifdef ESIF_ATTR_USER
	mem_ptr = realloc(mem_ptr, new_size);
	if (NULL == old_ptr && NULL != mem_ptr)
		esif_ccb_memset(mem_ptr, 0, new_size);

	return mem_ptr;

#endif
}

#endif /* ESIF_ATTR_MEMTRACE */

/* Memory Dump */
static ESIF_INLINE void esif_ccb_mem_dump(
	const u8 *ch,
	const u8 *what,
	const int size
	)
{
	int i;
	UNREFERENCED_PARAMETER(what);
	MEMORY_DEBUG("%s: Dumping Memory for %s\n", ESIF_FUNC, what);
	for (i = 0; i < size; i++, ch++)
		MEMORY_DEBUG("<%p> 0x%02X\n", (u8 *)ch, (u8)*ch);
	MEMORY_DEBUG("End of Memory Dump\n");
}


#endif /* _ESIF_CCB_MEMORY_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

