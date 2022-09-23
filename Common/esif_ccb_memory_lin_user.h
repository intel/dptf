/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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


#include <memory.h>
#include <malloc.h>

/* Memory Copy */
static ESIF_INLINE void esif_ccb_memcpy(
	void *dest_ptr,
	const void *src_ptr,
	size_t size
	)
{
	memcpy(dest_ptr, src_ptr, size);
}


/* Memory Move */
static ESIF_INLINE void esif_ccb_memmove(
	void *dest_ptr,
	const void *src_ptr,
	size_t size
	)
{
	memmove(dest_ptr, src_ptr, size);
}


/* Memory Set */
static ESIF_INLINE void *esif_ccb_memset(
	void *s_ptr,
	int c,
	size_t count
	)
{
	return memset(s_ptr, c, count);
}

/* Use Memtrace Overrides or Standard Functions? */
#ifdef ESIF_ATTR_MEMTRACE
#include "esif_ccb_memtrace.h"
#else

/* Memory Allocate */
static ESIF_INLINE void *esif_ccb_malloc(size_t size)
{
	void *mem_ptr = NULL;
	mem_ptr = malloc(size);
	if (NULL != mem_ptr)
		esif_ccb_memset(mem_ptr, 0, size);

	return mem_ptr;
}


/* Memory Free */
static ESIF_INLINE
void esif_ccb_free(void *mem_ptr)
{
	if (NULL != mem_ptr)
		free(mem_ptr);
}


/* Memory Reallocate */
static ESIF_INLINE void *esif_ccb_realloc(
	void *mem_ptr,
	size_t new_size
	)
{
	void *old_ptr = mem_ptr;

	mem_ptr = realloc(mem_ptr, new_size);
	if (NULL == old_ptr && NULL != mem_ptr)
		esif_ccb_memset(mem_ptr, 0, new_size);

	return mem_ptr;
}

#endif /* ESIF_ATTR_MEMTRACE */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

