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

#include "esif_hash_table.h"

#ifdef ESIF_ATTR_OS_WINDOWS
/*
 *
 * The Windows banned-API check header must be included after all other headers,
 * or issues can be identified
 * against Windows SDK/DDK included headers which we have no control over.
 *
 */
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#ifdef ESIF_ATTR_KERNEL

/* Debug Logging Defintions */
#define INIT_DEBUG         0
#define CREATE_DEBUG       1
#define DESTROY_DEBUG      2
#define PUT_DEBUG          3
#define GET_DEBUG          4
#define HASH_DEBUG         5

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_HASH, INIT_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_CREATE(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_HASH, CREATE_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_DESTROY(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_HASH, DESTROY_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_PUT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_HASH, PUT_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_GET(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_HASH, GET_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_HASH(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_HASH, HASH_DEBUG, format, ##__VA_ARGS__)

#endif /* ESIF_ATTR_KERNEL */

#ifdef ESIF_ATTR_USER

#define ESIF_TRACE_DYN_HASH NO_ESIF_DEBUG
#define ESIF_TRACE_DYN_GET NO_ESIF_DEBUG
#define ESIF_TRACE_DYN_PUT NO_ESIF_DEBUG
#define ESIF_TRACE_DYN_CREATE NO_ESIF_DEBUG
#define ESIF_TRACE_DYN_DESTROY NO_ESIF_DEBUG
#define ESIF_TRACE_DYN_INIT NO_ESIF_DEBUG


#endif /* ESIF_ATTR_USER */

/*
 * follows the FNV-1a method of hash creation
 * http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-1a
 */
static const u32 FNV_PRIME = 16777619;

/* Hash */
static u32 esif_hash_table_hash(
	u8 *data_ptr,
	u32 data_length
	)
{
	u32 hash_value = 0;
	u32 offset     = 0;

	for (offset = 0; offset < data_length; ++offset) {
		hash_value = hash_value ^ data_ptr[offset];
		hash_value = hash_value * FNV_PRIME;
	}
	ESIF_TRACE_DYN_HASH("%s: hash %08x\n", ESIF_FUNC, hash_value);
	return hash_value;
}


/* Get Item From Hash Table */
struct esif_link_list *esif_hash_table_get_item(
	u8 *key_ptr,
	u32 key_length,
	struct esif_hash_table *hash_table_ptr
	)
{
	struct esif_link_list *ll_ptr = NULL;
	u32 hash_index =
		esif_hash_table_hash(key_ptr,
				     key_length) % hash_table_ptr->size;

	ll_ptr = hash_table_ptr->table[hash_index];
	ESIF_TRACE_DYN_GET(
		"%s: key %p, key size %d, table %p index %d llr %p\n",
		ESIF_FUNC,
		key_ptr,
		key_length,
		hash_table_ptr,
		hash_index,
		ll_ptr);
	return ll_ptr;
}


/* Put Item In Hash Table */
enum esif_rc esif_hash_table_put_item(
	u8 *key_ptr,
	u32 key_length,
	void *item_ptr,
	struct esif_hash_table *hash_table_ptr
	)
{
	u32 hash_index =
		esif_hash_table_hash(key_ptr,
				     key_length) % hash_table_ptr->size;

	esif_link_list_node_add(hash_table_ptr->table[hash_index],
				esif_link_list_create_node(item_ptr));

	ESIF_TRACE_DYN_PUT(
		"%s: key %p, key size %d, item %p, table %p index %d\n",
		ESIF_FUNC,
		key_ptr,
		key_length,
		item_ptr,
		hash_table_ptr,
		hash_index);
	return ESIF_OK;
}


/* Create Hash Table */
struct esif_hash_table *esif_hash_table_create(u32 size)
{
	u32 table_index = 0;

	struct esif_hash_table *new_hash_table_ptr = NULL;

	new_hash_table_ptr = (struct esif_hash_table *)
				esif_ccb_mempool_zalloc(ESIF_MEMPOOL_TYPE_HASH);

	if (NULL == new_hash_table_ptr)
		return NULL;

	new_hash_table_ptr->size  = size;
	new_hash_table_ptr->table = (struct esif_link_list **)
			esif_ccb_malloc(sizeof(struct esif_link_list *) * size);
	if (new_hash_table_ptr->table == NULL) {
		esif_ccb_mempool_free(ESIF_MEMPOOL_TYPE_HASH,
				      new_hash_table_ptr);
		return NULL;
	}

	for (table_index = 0; table_index < size; ++table_index) {
		ESIF_TRACE_DYN_CREATE("%s: create linked list %d\n",
				      ESIF_FUNC,
				      table_index);
		new_hash_table_ptr->table[table_index] =
			esif_link_list_create();
	}
	ESIF_TRACE_DYN_CREATE("%s: have hash table %p\n",
			      ESIF_FUNC,
			      new_hash_table_ptr);
	return new_hash_table_ptr;
}


/* Destroy Hash Table */
void esif_hash_table_destroy(struct esif_hash_table *hash_table_ptr)
{
	u32 table_index;
	ESIF_TRACE_DYN_DESTROY("%s: destorying hash table %p\n",
			       ESIF_FUNC,
			       hash_table_ptr);
	for (table_index = 0; table_index < hash_table_ptr->size;
	     ++table_index) {
		ESIF_TRACE_DYN_DESTROY("%s: destroy linked list %d\n",
				       ESIF_FUNC,
				       table_index);
		esif_link_list_destroy(hash_table_ptr->table[table_index]);
	}
	esif_ccb_free(hash_table_ptr->table);

	esif_ccb_mempool_free(ESIF_MEMPOOL_TYPE_HASH, hash_table_ptr);
}


/* Init */
enum esif_rc esif_hash_table_init(void)
{
	struct esif_ccb_mempool *mempool_ptr = NULL;
	ESIF_TRACE_DYN_INIT("%s: Initialize Primitive Hash Table\n", ESIF_FUNC);

	mempool_ptr =
		esif_ccb_mempool_create(ESIF_MEMPOOL_TYPE_HASH,
					ESIF_MEMPOOL_FW_HASH,
					sizeof(struct esif_hash_table));
	if (NULL == mempool_ptr)
		return ESIF_E_NO_MEMORY;

	return ESIF_OK;
}


/* Exit */
void esif_hash_table_exit(void)
{
	ESIF_TRACE_DYN_INIT("%s: Exit Primitive Hash Table\n", ESIF_FUNC);
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
