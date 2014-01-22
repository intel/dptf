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

#ifndef _ESIF_LINK_LIST_H_
#define _ESIF_LINK_LIST_H_

#include "esif.h"

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

/* Linked List Node */
struct esif_link_list_node *esif_link_list_create_node(void *data_ptr);
void esif_link_list_destroy_node(struct esif_link_list_node *node_ptr);

/* Linked List */
struct esif_link_list *esif_link_list_create(void);
void esif_link_list_destroy(struct esif_link_list *list_ptr);

void esif_link_list_node_add(struct esif_link_list *list_ptr,
			     struct esif_link_list_node *new_node_ptr);

/* Init */
enum esif_rc esif_link_list_init(void);
void esif_link_list_exit(void);

#ifdef ESIF_ATTR_USER
typedef struct esif_link_list EsifLinkList, *EsifLinkListPtr,
	**EsifLinkListPtrLocation;
#endif

#endif /* _ESIF_LINK_LIST_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
