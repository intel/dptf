/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "esif_ccb.h"
#include "esif_ccb_string.h"
#include "esif_ccb_memtrace.h"

#ifdef ESIF_ATTR_MEMTRACE
struct memtrace_s g_memtrace = { 0 };

// Native memory allocation functions for use by memtrace functions only
#define native_malloc(siz)          malloc(siz)
#define native_realloc(ptr, siz)    realloc(ptr, siz)
#define native_free(ptr)            free(ptr)

// Dump Memory Leaks to Console by Default
#define	MEMTRACE_DEBUG(fmt, ...)	fprintf(stderr, fmt, ##__VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
// Function Definitions
///////////////////////////////////////////////////////////////////////////////

void *esif_memtrace_alloc(
	void *old_ptr,
	size_t size,
	const char *func,
	const char *file,
	int line
)
{
	void *mem_ptr = NULL;

	// Call Native functions if Memtrace not enabled
	if (!g_memtrace.enabled) {
		if (old_ptr) {
			mem_ptr = native_realloc(old_ptr, size);
		}
		else {
			mem_ptr = native_malloc(size);
			if (mem_ptr) {
				esif_ccb_memset(mem_ptr, 0, size);
			}
		}
		return mem_ptr;
	}

	esif_ccb_write_lock(&g_memtrace.lock);

	struct memalloc_s *mem = g_memtrace.allocated;
	struct memalloc_s **last = &g_memtrace.allocated;

	if (file) {
		const char *slash = strrchr(file, *ESIF_PATH_SEP);
		if (slash) {
			file = slash + 1;
		}
	}

	if (old_ptr) {
		mem_ptr = native_realloc(old_ptr, size);

		// realloc(ptr, size) leaves ptr unaffected if realloc fails and size is nonzero
		if (!mem_ptr && size > 0) {
			goto exit;
		}
		while (mem) {
			if (old_ptr == mem->mem_ptr) {
				// realloc(ptr, 0) behaves like free(ptr)
				if (size == 0) {
					*last = mem->next;
					native_free(mem);
				}
				else {
					mem->mem_ptr = mem_ptr;
					mem->size = size;
					mem->func = func;
					mem->file = file;
					mem->line = line;
				}
				goto exit;
			}
			last = &mem->next;
			mem = mem->next;
		}
	}
	else {
		mem_ptr = native_malloc(size);
		if (mem_ptr) {
			esif_ccb_memset(mem_ptr, 0, size);
			atomic_inc(&g_memtrace.allocs);
		}
	}

	mem = (struct memalloc_s *)native_malloc(sizeof(*mem));
	if (mem) {
		esif_ccb_memset(mem, 0, sizeof(*mem));
		mem->mem_ptr = mem_ptr;
		mem->size = size;
		mem->func = func;
		mem->file = file;
		mem->line = line;
		mem->next = g_memtrace.allocated;
		g_memtrace.allocated = mem;
	}

exit:
	esif_ccb_write_unlock(&g_memtrace.lock);
	return mem_ptr;
}


char *esif_memtrace_strdup(
	const char *str,
	const char *func,
	const char *file,
	int line
)
{
	size_t len = esif_ccb_strlen(str, 0x7fffffff) + 1;
	char *mem_ptr = (char *)esif_memtrace_alloc(0, len, func, file, line);
	if (NULL != mem_ptr) {
		esif_ccb_strcpy(mem_ptr, str, len);
	}
	return mem_ptr;
}


void esif_memtrace_free(void *mem_ptr)
{
	if (g_memtrace.enabled) {
		esif_ccb_write_lock(&g_memtrace.lock);

		struct memalloc_s *mem = g_memtrace.allocated;
		struct memalloc_s **last = &g_memtrace.allocated;

		while (mem) {
			if (mem_ptr == mem->mem_ptr) {
				*last = mem->next;
				native_free(mem);
				atomic_inc(&g_memtrace.frees);
				break;
			}
			last = &mem->next;
			mem = mem->next;
		}
		esif_ccb_write_unlock(&g_memtrace.lock);
	}

	if (mem_ptr) {
		native_free(mem_ptr);
	}
}


esif_error_t esif_memtrace_init(void)
{
	esif_error_t rc = ESIF_OK;
	struct memalloc_s *mem = NULL;

	if (!g_memtrace.enabled) {
		esif_ccb_lock_init(&g_memtrace.lock);
		esif_ccb_write_lock(&g_memtrace.lock);

		g_memtrace.enabled = ESIF_TRUE;
		mem = g_memtrace.allocated;

		// Ignore any allocations made before this function was called
		while (mem) {
			struct memalloc_s *node = mem;
			mem = mem->next;
			native_free(node);
		}
		g_memtrace.allocated = NULL;
		esif_ccb_write_unlock(&g_memtrace.lock);
	}
	return rc;
}


void esif_memtrace_exit()
{
	struct memalloc_s *mem = NULL;
	struct memalloc_s *node = NULL;

	if (g_memtrace.enabled) {
		esif_ccb_write_lock(&g_memtrace.lock);
		g_memtrace.enabled = ESIF_FALSE;
		mem = g_memtrace.allocated;
		g_memtrace.allocated = NULL;
		esif_ccb_write_unlock(&g_memtrace.lock);

		if (mem) {
			MEMTRACE_DEBUG("\n*** MEMORY LEAKS DETECTED: Allocs=" ATOMIC_FMT " Frees=" ATOMIC_FMT " ***\n", atomic_read(&g_memtrace.allocs), atomic_read(&g_memtrace.frees));

			while (mem) {
				node = mem;
				MEMTRACE_DEBUG("[%s@%s:%d]: (%lld bytes) %p\n", mem->func, mem->file, mem->line, (long long)mem->size, mem->mem_ptr);
				mem = mem->next;
				if (g_memtrace.free_leaks) {
					native_free(node->mem_ptr);
				}
				native_free(node);
			}
		}
		esif_ccb_lock_uninit(&g_memtrace.lock);
	}
}

#endif /* ESIF_ATTR_MEMTRACE */
