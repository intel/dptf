/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#define ESIF_DEBUG_MODULE ESIF_DEBUG_MOD_HASH

#ifdef ESIF_ATTR_KERNEL

#define EVENT_ENTRY	0
#define EVENT_VERBOSE	1
#define EVENT_RC	2

#define ESIF_TRACE_DYN_ENTRY(fmt, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_HASH, EVENT_ENTRY, fmt, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_VERB(fmt, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_HASH, EVENT_VERBOSE, fmt, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_RC(fmt, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_HASH, EVENT_RC, fmt, ##__VA_ARGS__)

#endif /* ESIF_ATTR_KERNEL */

#ifdef ESIF_ATTR_USER

#define ESIF_TRACE_DYN_ENTRY NO_ESIF_DEBUG
#define ESIF_TRACE_DYN_VERB NO_ESIF_DEBUG
#define ESIF_TRACE_DYN_RC NO_ESIF_DEBUG

#endif /* ESIF_ATTR_USER */

/*
 * follows the FNV-1a method of hash creation
 * http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-1a
 */
static const u32 FNV_PRIME = 16777619;

/* Function Declarations */

static u32 esif_compute_hash(
	u8 *data_ptr,
	u32 data_length
	);

static struct esif_link_list *esif_ht_get_ll(
	struct esif_ht *self,
	u8 *key_ptr,
	u32 key_length
	);

static struct esif_ht_node *esif_alloc_ht_node(
	u8 *key_ptr,
	u32 key_length,
	void *item_ptr
	);

static struct esif_link_list_node *esif_find_node_in_ht_ll(
	struct esif_link_list *ll_ptr,
	u8 *key_ptr,
	u32 key_length
	);

static u8 esif_cmp_keys(
	u8 *key1_ptr,
	u32 key1_length,
	u8 *key2_ptr,
	u32 key2_length
	);

static void esif_destroy_ht_node(struct esif_ht_node *ht_node);

static void esif_destroy_ht_nodes(
	struct esif_link_list *ll_ptr,
	item_destroy_func item_destroy_fptr
	);

static struct esif_ht_node *esif_ht_get_ht_node(
	struct esif_ht *self,
	u8 *key_ptr,
	u32 key_length
	);

/* Function Definitions */

/* Hash */
static u32 esif_compute_hash(
	u8 *data_ptr,
	u32 data_length
	)
{
	u32 hash_value = 0;
	u32 offset     = 0;

	ESIF_ASSERT(data_ptr != NULL);

	for (offset = 0; offset < data_length; ++offset) {
		hash_value = hash_value ^ data_ptr[offset];
		hash_value = hash_value * FNV_PRIME;
	}

	ESIF_TRACE_DYN_VERB("Hash %08x\n", hash_value);
	return hash_value;
}

u8 esif_cmp_keys(
	u8 *key1_ptr,
	u32 key1_length,
	u8 *key2_ptr,
	u32 key2_length
	)
{
	u8 keys_equal = ESIF_FALSE;

	if ((key1_ptr == NULL) || (key2_ptr == NULL)) {
		ESIF_TRACE_DYN_VERB("Passed in key ptr NULL\n)");
		goto exit;
	}
	if (key1_length != key2_length) {
		ESIF_TRACE_DYN_VERB("Passed in key lengths differ\n)");
		goto exit;
	}

	if (memcmp((void *)key1_ptr, (void *)key2_ptr, key1_length) == 0) {
		keys_equal = ESIF_TRUE;
	}
exit:
	return keys_equal;
}

static struct esif_ht_node *esif_ht_get_ht_node(
	struct esif_ht *self,
	u8 *key_ptr,
	u32 key_length
	)
{
	struct esif_link_list *ll_ptr = NULL;
	struct esif_link_list_node *ll_node = NULL;
	struct esif_ht_node *ht_node = NULL;

	ESIF_ASSERT(key_ptr != NULL);
	ESIF_ASSERT(self != NULL);

	ll_ptr = esif_ht_get_ll(self, key_ptr, key_length);
	if (ll_ptr == NULL) {
		ESIF_ASSERT(ESIF_FALSE); /* Should never happen */
		ESIF_TRACE_DYN_VERB("LL NULL for passed in key\n");
		goto exit;
	}

	ll_node = esif_find_node_in_ht_ll(ll_ptr, key_ptr, key_length);
	if (ll_node == NULL) {
		ESIF_TRACE_DYN_VERB("LL node not found for passed in key\n");
		goto exit;
	}

	ht_node = (struct esif_ht_node *)ll_node->data_ptr;
exit:
	return ht_node;
}

/* Get link list corresponding to the hash index */
struct esif_link_list * esif_ht_get_ll(
	struct esif_ht *self,
	u8 *key_ptr,
	u32 key_length
	)
{
	struct esif_link_list *ll_ptr = NULL;
	u32 hash_index;
	
	ESIF_ASSERT(key_ptr != NULL);
	ESIF_ASSERT(self != NULL);

	hash_index = esif_compute_hash(key_ptr, key_length) % self->size;

	ll_ptr = self->table[hash_index];
	ESIF_TRACE_DYN_VERB(
		"Key %p, key size %d, table %p index %d ll %p\n",
		key_ptr,
		key_length,
		self,
		hash_index,
		ll_ptr);

	return ll_ptr;
}

static struct esif_link_list_node * esif_find_node_in_ht_ll(
	struct esif_link_list *ll_ptr,
	u8 *key_ptr,
	u32 key_length
	)
{
	struct esif_link_list_node *cur_ptr = NULL;
	struct esif_ht_node *ht_node = NULL;

	ESIF_ASSERT(ll_ptr != NULL);
	ESIF_ASSERT(key_ptr != NULL);

	cur_ptr = ll_ptr->head_ptr;
	while (cur_ptr) {
		ht_node = (struct esif_ht_node *)cur_ptr->data_ptr;
		if (ht_node != NULL) {
			if (esif_cmp_keys(key_ptr, key_length,
				ht_node->key_ptr, ht_node->key_length)) {
				break;
			}
		}
		cur_ptr = cur_ptr->next_ptr;
	}
	return cur_ptr;
}

static struct esif_ht_node *esif_alloc_ht_node(
	u8 *key_ptr,
	u32 key_length,
	void *item_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	struct esif_ht_node *ht_node_ptr = NULL;
	
	ESIF_ASSERT(key_ptr != NULL);
	
	ht_node_ptr =
		(struct esif_ht_node *)esif_ccb_malloc(sizeof(*ht_node_ptr));
	if (ht_node_ptr == NULL) {
		ESIF_TRACE_ERROR("Unable to allocate HT node\n");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	ht_node_ptr->key_ptr = esif_ccb_malloc(key_length);
	if (ht_node_ptr->key_ptr == NULL) {
		ESIF_TRACE_ERROR("Unable to allocate HT node key ptr\n");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	esif_ccb_memcpy(ht_node_ptr->key_ptr, key_ptr, key_length);
	ht_node_ptr->key_length = key_length;
	ht_node_ptr->item_ptr = item_ptr;
exit:
	if (rc != ESIF_OK) {
		if (ht_node_ptr) {
			if (ht_node_ptr->key_ptr)
				esif_ccb_free(ht_node_ptr->key_ptr);

			esif_ccb_free(ht_node_ptr);
			ht_node_ptr = NULL;
		}
	}

	return ht_node_ptr;
}

/* This does not delete the item_ptr */
static void esif_destroy_ht_node(
	struct esif_ht_node *ht_node
	)
{
	ESIF_ASSERT(ht_node != NULL);

	if (ht_node->key_ptr)
		esif_ccb_free(ht_node->key_ptr);

	/* TODO: If needed */
	/* We can call the callback destroy func here so that
	 * the caller does not have to destroy the item. Right 
	 * now this is done before coming into this function 
	 * for HT destroy only.
	 */
	esif_ccb_free(ht_node);

	return;
}

/* Destroys ht_node's in the Hash Table LL */
void esif_destroy_ht_nodes(
	struct esif_link_list *ll_ptr,
	item_destroy_func item_destroy_fptr
	)
{
	struct esif_link_list_node *cur_ptr = NULL;
	struct esif_ht_node *ht_node = NULL;
	void *item_ptr = NULL;

	if (ll_ptr == NULL) {
		goto exit;
	}

	cur_ptr = ll_ptr->head_ptr;
	while (cur_ptr) {
		ht_node = (struct esif_ht_node *)cur_ptr->data_ptr;
		if (ht_node != NULL) {
			if (item_destroy_fptr) {
				item_ptr = ht_node->item_ptr;
				ht_node->item_ptr = NULL;
				item_destroy_fptr(item_ptr);
			}

			esif_destroy_ht_node(ht_node);
			cur_ptr->data_ptr = NULL;
		}

		cur_ptr = cur_ptr->next_ptr;
	}
exit:
	return;
}

enum esif_rc esif_ht_add_item(
	struct esif_ht *self,
	u8 *key_ptr,
	u32 key_length,
	void *item_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	struct esif_link_list *ll_ptr = NULL;
	struct esif_ht_node *ht_node = NULL;

	if ((key_ptr == NULL) || (self == NULL)) {
		ESIF_TRACE_DYN_VERB("NULL ptr passed in \n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	ll_ptr = esif_ht_get_ll(self, key_ptr, key_length);
	if (ll_ptr == NULL) {
		ESIF_ASSERT(ESIF_FALSE); /* Should never happen */
		ESIF_TRACE_DYN_VERB("LL NULL for passed in key\n");
		rc = ESIF_E_NOT_FOUND;
		goto exit;
	}

	ht_node = esif_alloc_ht_node(key_ptr, key_length, item_ptr);
	if (ht_node == NULL) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	rc = esif_link_list_add_at_back(ll_ptr, (void *)ht_node);
	if (rc != ESIF_OK) {
		ESIF_TRACE_ERROR("Unable to add new HT link list node\n");
		goto exit;
	}

	ESIF_TRACE_DYN_VERB(
		"Key %p, key size %d, item %p, table %p ll %p\n",
		key_ptr,
		key_length,
		item_ptr,
		self,
		ll_ptr);
exit:
	if (rc != ESIF_OK) {
		if (ht_node != NULL)
			esif_destroy_ht_node(ht_node);
	}

	return rc;
}

enum esif_rc esif_ht_remove_item(
	struct esif_ht *self,
	u8 *key_ptr,
	u32 key_length
	)
{
	enum esif_rc rc = ESIF_OK;
	struct esif_link_list *ll_ptr = NULL;
	struct esif_link_list_node *ll_node = NULL;
	struct esif_ht_node *ht_node = NULL;

	if ((key_ptr == NULL) || (self == NULL)) {
		ESIF_TRACE_ERROR("NULL ptr passed in\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	ll_ptr = esif_ht_get_ll(self, key_ptr, key_length);
	if (ll_ptr == NULL) {
		ESIF_ASSERT(ESIF_FALSE); /* Should never happen */
		ESIF_TRACE_DYN_VERB("LL NULL for passed in key\n");
		rc = ESIF_E_NOT_FOUND;
		goto exit;
	}

	ll_node = esif_find_node_in_ht_ll(ll_ptr, key_ptr, key_length);
	if (ll_node == NULL) {
		ESIF_TRACE_DYN_VERB("LL node not found for passed in key\n");
		rc = ESIF_E_NOT_FOUND;
		goto exit;
	}

	ht_node = (struct esif_ht_node *)ll_node->data_ptr;
	if (ht_node == NULL) {
		ESIF_ASSERT(ESIF_FALSE);
		ESIF_TRACE_DYN_VERB("ht_node NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_destroy_ht_node(ht_node);
	esif_link_list_node_remove(ll_ptr, ll_node);
exit:
	return rc;
}

void * esif_ht_get_item(
	struct esif_ht *self,
	u8 *key_ptr,
	u32 key_length
	)
{
	struct esif_ht_node *ht_node = NULL;
	void *item_ptr = NULL;

	if ((key_ptr == NULL) || (self == NULL)) {
		ESIF_TRACE_ERROR("NULL ptr passed in\n");
		goto exit;
	}

	ht_node = esif_ht_get_ht_node(self,
		key_ptr,
		key_length);
	if (ht_node == NULL) {
		ESIF_TRACE_DYN_VERB("HT node data not found for passed in key\n");
		goto exit;
	}

	item_ptr = ht_node->item_ptr;
exit:
	return item_ptr;
}

/* Create Hash Table */
struct esif_ht * esif_ht_create(
	u32 size
	)
{
	u32 index = 0;
	struct esif_ht *new_ht_ptr = NULL;

	new_ht_ptr = (struct esif_ht *)
		esif_ccb_mempool_zalloc(ESIF_MEMPOOL_TYPE_HASH2);

	if (NULL == new_ht_ptr) {
		ESIF_TRACE_ERROR("Cannot allocate mem for hash table\n");
		ESIF_ASSERT(ESIF_FALSE);
		goto exit;
	}

	new_ht_ptr->size = size;
	new_ht_ptr->table = (struct esif_link_list **)
		esif_ccb_malloc(sizeof(*new_ht_ptr->table) * size);

	if (new_ht_ptr->table == NULL) {
		esif_ccb_mempool_free(ESIF_MEMPOOL_TYPE_HASH2, new_ht_ptr);
		new_ht_ptr = NULL;
		goto exit;
	}

	for (index = 0; index < size; ++index) {
		ESIF_TRACE_DYN_VERB("Create linked list %d\n", index);
		new_ht_ptr->table[index] = esif_link_list_create();

		if (new_ht_ptr->table[index] == NULL) {
			ESIF_TRACE_DYN_VERB("Creation failed\n");
			esif_ht_destroy(new_ht_ptr, NULL);
			new_ht_ptr = NULL;
			goto exit;
		}
	}

	ESIF_TRACE_DYN_VERB("Have hash table %p\n", new_ht_ptr);
exit:
	return new_ht_ptr;
}

/* Destroy Hash Table */
void esif_ht_destroy(
	struct esif_ht *self,
	item_destroy_func item_destroy_fptr
	)
{
	u32 index = 0;

	if ((self == NULL) || (self->table == NULL)) {
		ESIF_TRACE_ERROR("Hash table ptr NULL\n");
		goto exit;
	}

	ESIF_TRACE_DYN_VERB("Destroying hash table %p\n", self);

	for (index = 0; index < self->size; ++index) {
		if (self->table[index] == NULL)
			continue;

		ESIF_TRACE_DYN_VERB("Destroy linked list %d\n", index);

		esif_destroy_ht_nodes(self->table[index], item_destroy_fptr);
		esif_link_list_destroy(self->table[index]);
		self->table[index] = NULL;
	}

	esif_ccb_free(self->table);
	esif_ccb_mempool_free(ESIF_MEMPOOL_TYPE_HASH2, self);
exit:
	return;
}


/* Init */
enum esif_rc esif_ht_init(void)
{
	enum esif_rc rc = ESIF_OK;
	struct esif_ccb_mempool *mempool_ptr = NULL;

	ESIF_TRACE_DYN_VERB("Initialize Hash Table\n");

	mempool_ptr = esif_ccb_mempool_create(ESIF_MEMPOOL_TYPE_HASH2,
					ESIF_MEMPOOL_FW_HASH,
					sizeof(struct esif_ht));

	if (NULL == mempool_ptr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
exit:
	return rc;
}

/* Exit */
void esif_ht_exit(void)
{
	ESIF_TRACE_DYN_VERB("Exit Hash Table\n");
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
