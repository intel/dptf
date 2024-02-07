/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "esif_sdk.h"
#include "esif_ccb_rc.h"

#pragma pack(push, 1)

// ESIF Message Header Version 1.0.0
typedef struct EsifMsgHdrV1_s {
	UInt16	signature;		// Header Signature [E5 D8]
	UInt16	headersize;		// Header Size, including signature and headersize
	UInt32	version;		// Header Version [0xAABBCCCC: AA=major BB=minor CCCC=revision]
	UInt32	msgclass;		// Payload Message Class
	UInt32	msglen;			// Payload Message Length (0 - ESIFMSG_MAX_PAYLOAD)
} EsifMsgHdrV1;

// ESIF Message Header Union (all versions)
typedef union EsifMsgHdr_s {
	EsifMsgHdrV1	v1;		// Version 1 Message Header; Future Versions will always contain v1 members
} EsifMsgHdr, *EsifMsgHdrPtr;
/*
** Message Frame consists of Variable-length Header and Variable-length Payload (Header + Payload = Frame)
**
** ESIF Messages consist of a Message Frame (Header + Payload) and can be transmitted over an arbitrary
** protocol, such as WebSocket, HTTP, Named Pipes, or files. The Header is owned by the Framework and is
** unlikely to change very often. The Payload is owned by the application that implements the msgclass
** and may change frequently, and it may be any format (XML, Binary, struct/union, etc). Payloads must be
** self-contained so that they can cross process and network boundaries and should never contain pointers.
**
** For maximum compatibility, Payloads should always start with a revision of some type (UInt32, UInt16, etc)
** and Clients should verify the Revision before processing, and may support multiple revisions if they choose.
**
** Optionally, Clients may also support multiple Header versions by reading 'headersize' bytes before reading
** the variable-length Payload. The rule-of-thumb is that if the Major version of the header matches what
** the client expects, the client should be able to process the Payload by skipping the unknown portion
** of headersize before reading the Payload. If the Major versions do not match, the client should skip
** both the remaining header and the entire Payload (so the next message can be read). For example, a
** Client written to read v1.0 messages should be able to read a v1.1 header if it chooses but not v2.0
**
** Clients are not required to support multiple Header versions or multiple Payload revisions, but if they
** don't, they will only work withthe most recent version of the Server messages, and therefore Clients and 
** Server (ESIF) must always be updated at the same time. As a result, if maxiumum compatibility is desired,
** Header and Payload should be read in separate Reads rather than in one combined read using a Frame struct. 
** However, it is perfectly appropriate for a Server to define a Frame struct for a message it is sending since
** it only needs to support the latest Header version and Payload revision.
**
** Clients must always verify Header Signature, Version, MsgClass, and Payload Revision before processing the Payload.
*/

#pragma pack(pop)

// Constants
#define	ESIFMSG_SIGNATURE		0xD8E5					// [E5 D8] - ESIF Message Header Signature
#define ESIFMSG_MAX_HEADER		0x2000					// Maximum Header Size (8KB)
#define ESIFMSG_MAX_PAYLOAD		0x7FFFFFFF				// Maximum Payload Size
#define ESIFMSG_VERSION			ESIFHDR_VERSION(1,0,0)	// Current EsifMsgHdr Version

// Known Message Classes (Hex or Multichar); May be extended without rebuilding Framework
#define ESIFMSG_CLASS_UNDEFINED	0x00000000	// Undefined Message Class
#define ESIFMSG_CLASS_UFSP		0x50534655	// "UFSP" = ESIF Upper Framework Service Protocol
#define ESIFMSG_CLASS_KEYS		0x5359454B	// "KEYS" = DataVault Key/Value Pair List
#define ESIFMSG_CLASS_REPO		0x4F504552	// "REPO" = Data Repository
#define ESIFMSG_CLASS_IRPC		0x43505249	// "IRPC" = IPF RPC Call

/* 
 * Inline Helper Functions; May be moved to Standard functions later
 */

#ifdef __cplusplus
extern "C" {
#endif

// Initialize a Message Header
static ESIF_INLINE void EsifMsgHdr_Init(
	EsifMsgHdrPtr header,
	UInt32 msgclass,
	size_t msglen)
{
	if (header) {
		header->v1.signature = ESIFMSG_SIGNATURE;
		header->v1.headersize = sizeof(*header);
		header->v1.version = ESIFMSG_VERSION;
		header->v1.msgclass = msgclass;
		header->v1.msglen = (UInt32) msglen;
	}
}

// Verify that a Message Header is compatible
static ESIF_INLINE esif_error_t EsifMsgHdr_Verify(EsifMsgHdrPtr header)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	// Allow Variable-Length Header if Major Version Matches and Signature matches
	if (header != NULL) {
		if ((header->v1.signature == ESIFMSG_SIGNATURE) &&
			(ESIFHDR_GET_MAJOR(header->v1.version) == ESIFHDR_GET_MAJOR(ESIFMSG_VERSION)) &&
			(header->v1.headersize >= sizeof(EsifMsgHdr))) {
			rc = ESIF_OK;
		}
		else {
			rc = ESIF_E_NOT_SUPPORTED;
		}
	}
	return rc;
}

// Get Payload data from a Message Frame buffer (Header + Payload); Payload is optional if payload_ptr is NULL
static ESIF_INLINE esif_error_t EsifMsgFrame_GetPayload(
	EsifMsgHdrPtr header,	// Header or Frame (Header + Payload) Pointer
	size_t frame_size,		// Total Frame Size, including variable-length Header; Payload data optional
	UInt32 *msgclass_ptr,	// [Out] Optional Parameter to return Message Class
	void **payload_ptr,		// [Out] Optional Parameter to return Payload Data, if it fits in the Frame
	size_t *payload_len)	// [Out] Optional Parameter to return Payload Size, if it fits in the Frame
{
	esif_error_t rc = ESIF_E_NOT_SUPPORTED;

	// Verify Variable-length Header
	if (frame_size >= sizeof(EsifMsgHdr)) {
		rc = EsifMsgHdr_Verify(header);
	}
	if (rc == ESIF_OK) {
		if (msgclass_ptr) {
			*msgclass_ptr = header->v1.msgclass;
		}
		// Return Payload only if Out parameters are supplied and the Frame size = Header size + Payload size
		if (payload_ptr && payload_len) {
			if (frame_size < (size_t)header->v1.headersize + header->v1.msglen) {
				rc = ESIF_E_REQUEST_DATA_OUT_OF_BOUNDS;
			}
			else {
				*payload_ptr = (void *) (((UInt8 *)header) + header->v1.headersize);
				*payload_len = (size_t) header->v1.msglen;
			}
		}
	}
	return rc;
}

#ifdef __cplusplus
}
#endif
