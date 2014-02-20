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

#ifndef _ESIF_DATA_H_
#define _ESIF_DATA_H_

/* include enum esif_data_type, and lookup */
/* #ifdef ESIF_ATTR_USER */
    #include "esif.h"
/* #endif */

#include "autogen.h"

/*
** User Decleration
*/

/*
** Agnostic Decleration
*/
/* Have ESIF Allocate Buffer */
#define ESIF_DATA_ALLOCATE 0xFFFFFFFF

/* All ESIF Data Buffers Are Of This Form */
struct esif_data {
	enum esif_data_type  type;	/* Buffer Type */
	void  *buf_ptr;		/* Buffer */
	u32   buf_len;		/* Sizeof Allocated Buffer By Memory Allocator
				 **/
	u32   data_len;		/* Sizeof Actual Data Contents */
};

/* Cooling Mode Policy */
struct esif_data_complex_scp {
	u32  cooling_mode;
	u32  acoustic_limit;
	u32  power_limit;
};

/* Thermal Shutdown Event */
struct esif_data_complex_shutdown {
	u32 temperature;
	u32 tripPointTemperature;
};

/* Operating System Capabilities */
struct esif_data_complex_osc {
	esif_guid_t  guid;
	u32          revision;
	u32          count;
	u32          status;		/* Required */
	u32          capabilities;	/* Always Used By DPTF */
};

/* GUID Pair */
struct esif_data_complex_guid_pair {
	esif_guid_t  guid1; /* Example SUB_GROUP */
	esif_guid_t  guid2; /* Example Power Setting */
};

#define ESIF_TABLE_NO_REVISION  0xffff
struct esif_table_hdr {
	u8   revision;
	u16  rows;
	u16  cols;
};


/* Variant */

/* Binary data shall be identical to what upper framework sees */
/* Pack so we end up with exactly 12 bytes always */

#pragma pack(push, 1)
union esif_data_variant {
	enum esif_data_type  type;

	/* Integer */
	struct {
		enum esif_data_type  type;	/* 4 Bytes For Most Compilers */
		u64 value;			/* 8 Byte Integer */
	}

	integer;

	/* ASCII String NULL Included In Length */
	struct {
		enum esif_data_type  type;	/* 4 Bytes For Most Compilers
						 * */
		u32  length;			/* 4 Bytes */
		u32  reserved;			/* Align To 12 Bytes */
	}

	string;

	/* String Data Here */
};

#pragma pack(pop)

/* Allocate */
struct esif_data *esif_data_alloc(enum esif_data_type type, u32 data_len);
void esif_data_free(struct esif_data *data_ptr);

/* Init */
enum esif_rc esif_data_init(void);
void esif_data_exit(void);

#ifdef ESIF_ATTR_USER

typedef struct esif_data EsifData, *EsifDataPtr, **EsifDataPtrLocation;

/*
 *  ESIF Data is used to marshall most data between ESIF and the
 *  application it contains.  ESIF data uses the populare TLV
 *  format for describing the data.  It can be used to request
 *  specific data from an object or to identify the data type that
 *  is returned.  While a simple structure it is very helpful.  Here
 *  are a set of convinience initializers to make working with this
 *  structure easier.
 */
#define ESIF_DATA(var, type, buf_ptr, buf_len) \
	EsifData var = { \
		ESIF_ELEMENT(.type) type, \
		ESIF_ELEMENT(.buf_ptr) buf_ptr, \
		ESIF_ELEMENT(.buf_len) buf_len \
	}


#define ESIF_DATA_VOID(var) \
	EsifData var = { \
		ESIF_ELEMENT(.type)    ESIF_DATA_VOID, \
		ESIF_ELEMENT(.buf_ptr) NULL, \
		ESIF_ELEMENT(.buf_len) 0 \
	}


#define ESIF_DATA_AUTO(var) \
	EsifData var = { \
		ESIF_ELEMENT(.type)    ESIF_DATA_AUTO, \
		ESIF_ELEMENT(.buf_ptr) ESIF_DATA_ALLOCATE, \
		ESIF_ELEMENT(.buf_len) 0 \
	}


#endif /* ESIF_ATTR_USER */
#endif /* _ESIF_DATA_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
