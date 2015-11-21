/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
#pragma once

#include "esif_ccb.h"
#include "esif_ccb_rc.h"
#include "esif_ccb_memory.h"

/*
 * INTERFACE - The interface consists of the following:
 *
 *    esif_link_list - Linked list type
 *
 *    esif_link_list_node - Linked list node type
 *
 *    esif_link_list_init - Should be called before any lists are created to
 *        allocated required resources and perform initialization
 *
 *    esif_link_list_exit - Should be called after all lists are destroyed and
 *        no further list will be created in order to release resource as needed
 *
 *    esif_link_list_create - Allocates a new esif_link_list.  Must be called
 *        before other functions that operate on the list
 *
 *    esif_link_list_free_data_and_destroy- Destroys a list and all data it
 *        contains.  It will call the provided callback function to free the
 *        data associated with each node during destruction, or will call
 *        esif_ccb_free on the data if no callback function is provided. No
 *        other functions should be called on a list after this is called.
 *        WARNING:  Care should be taken that an and locks held during the
 *        call of this function must be taken into account in the callback
 *        function to destroy the data.
 *
 *    esif_link_list_destroy- Destroys a list and all nodes it contains.  It
 *        is the responsibility of the user to free any resources that were
 *        passed in as data within before calling this function.  No other
 *        functions should be called on a list after this is called.
 *
 *    esif_link_list_add_at_front - Creates a new node with the data and adds it
 *        to the front of a list
 *
 *    esif_link_list_add_at_back - Creates a new node with the data and adds it
 *        to the back of a list
 *
 *    esif_link_list_node_remove - Removes a node from a list
 *
 *    All other code contained is implementation code and should not be used
 *    or referenced.
 *
 *    There is no interface function to find a node with specific data.  The
 *    user may use the list members to traverse a list as needed.
 *
 * IMPORTANT!!!!
 *
 *    In order to use this code, #define ESIF_CCB_LINK_LIST_MAIN must be
 *    declared before inclusion of this header file in ONE AND ONLY ONE file
 *    of an execution unit.  This brings the implementation code into the
 *    build.
 *
 *    NOTE:  This implementation provides no locking mechanisms for access.
 *        It is up tot he user to provide synchronization as needed.
 */

#pragma pack(push, 1)

/* Link List Node */
struct esif_link_list_node {
	void  *data_ptr;
	struct esif_link_list_node  *next_ptr;
	struct esif_link_list_node  *prev_ptr;
};

/* Link List */
struct esif_link_list {
	struct esif_link_list_node  *head_ptr;
	struct esif_link_list_node  *tail_ptr;
	u32 nodes;
};

#pragma pack(pop)

#ifdef ESIF_ATTR_USER
typedef struct esif_link_list EsifLinkList, *EsifLinkListPtr;
typedef struct esif_link_list_node EsifLinkListNode, *EsifLinkListNodePtr;
#endif

typedef void (*link_list_data_destroy_func)(void *data_ptr);

#ifdef __cplusplus
extern "C" {
#endif

enum esif_rc esif_link_list_init(void);
void esif_link_list_exit(void);

struct esif_link_list *esif_link_list_create(void);
void esif_link_list_destroy(struct esif_link_list *self);

void esif_link_list_free_data_and_destroy(
	struct esif_link_list *self,
	link_list_data_destroy_func destroy_func
	);

enum esif_rc esif_link_list_add_at_front(
	struct esif_link_list *self,
	void *data_ptr
	);

enum esif_rc esif_link_list_add_at_back(
	struct esif_link_list *self,
	void *data_ptr
	);

struct esif_link_list_node *esif_link_list_create_node(void *data_ptr);
void esif_link_list_destroy_node(struct esif_link_list_node *node_ptr);

void esif_link_list_add_node_at_front(
	struct esif_link_list *list_ptr,
	struct esif_link_list_node *new_node_ptr
	);

void esif_link_list_add_node_at_back(
	struct esif_link_list *list_ptr,
	struct esif_link_list_node *new_node_ptr
	);

void esif_link_list_node_remove(
	struct esif_link_list *self,
	struct esif_link_list_node *node_ptr
	);

#ifdef __cplusplus
}
#endif

#ifdef ESIF_CCB_LINK_LIST_MAIN

#include "esif_link_list.c"

#endif

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
