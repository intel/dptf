/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
**
** Licensed under the Apache License, Version 2.0 (the "License"); you may not
** use this file except in compliance with the License.
**
** You may obtain a copy of the License at
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
** WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**
** See the License for the specific language governing permissions and
** limitations under the License.
**
******************************************************************************/

#pragma once

#if defined(ESIF_ATTR_USER) && defined(ESIF_ATTR_MEMTRACE)

#include "esif_ccb_rc.h"
#include "esif_ccb_atomic.h"
#include "esif_ccb_lock.h"

/* Enable detailed Memory Allocation Tracing? */

struct memalloc_s {
	struct memalloc_s	*next;
	void			*mem_ptr;
	size_t			size;
	const char		*func;
	const char		*file;
	int			line;
};
struct memtrace_s {
	esif_ccb_lock_t		lock;




	atomic_t		allocs;
	atomic_t		frees;
	Bool			free_leaks;
	Bool			enabled;
	struct memalloc_s	*allocated;
};

#ifdef __cplusplus
extern "C" {
#endif

extern struct memtrace_s g_memtrace;

extern esif_error_t  esif_memtrace_init(void);
extern void  esif_memtrace_exit(void);
extern void *esif_memtrace_alloc(
	void *old_ptr,
	size_t size,
	const char *func,
	const char *file,
	int line);
extern void  esif_memtrace_free(void *mem_ptr);
extern char *esif_memtrace_strdup(
	const char *str,
	const char *func,
	const char *file,
	int line);

#ifdef __cplusplus
}
#endif

/* Override Standard malloc/realloc/free functions */
#define esif_ccb_malloc(size) \
	esif_memtrace_alloc(0, size, ESIF_FUNC, __FILE__, __LINE__)
#define esif_ccb_realloc(old_ptr, size) \
	esif_memtrace_alloc(old_ptr, size, ESIF_FUNC, __FILE__, __LINE__)
#define esif_ccb_free(mem_ptr) \
	esif_memtrace_free(mem_ptr)
#define esif_ccb_strdup(str) \
	esif_memtrace_strdup(str, ESIF_FUNC, __FILE__, __LINE__)

#endif /* USER */
