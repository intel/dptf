/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
#define ESIF_TRACE_ID ESIF_TRACEMODULE_WEBSERVER

#include <ctype.h>
#include "esif_ws_socket.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif


/*
 *******************************************************************************
 ** PRIVATE
 *******************************************************************************
 */


static void esif_ws_socket_convert_to_lower_case(char *string);

static void esif_ws_socket_copy_line (
	const char *source,
	char *destination
	);

static char *esif_ws_socket_get_field_value(const char *source);

static size_t esif_ws_socket_get_payload_size (
	const WsSocketFramePtr framePtr,
	size_t incomingFrameLen,
	FrameTypePtr frameTypePtr 
	);

static size_t esif_ws_socket_get_header_len(
	const WsSocketFramePtr framePtr,
	size_t frameLen
	);

static UInt8 *esif_ws_socket_get_mask(
	const WsSocketFramePtr framePtr,
	size_t frameLen
	);

static UInt8 esif_ws_socket_is_mask_enabled(const WsSocketFramePtr framePtr);


#ifdef ESIF_ATTR_OS_LINUX
static ESIF_INLINE UInt64 htonll(UInt64 value)
{
	UInt32 hi = htonl((UInt32)(value >> 32));
	UInt32 lo = htonl((UInt32)value);
	return (((UInt64) lo) << 32) | hi;
}
#endif

/*
 *******************************************************************************
 ** PUBLIC
 *******************************************************************************
 */
FrameType esif_ws_socket_get_initial_frame_type(
	const char *framePtr,
	size_t incomingFrameLen,
	ProtocolPtr protPtr
	)
{
	unsigned char is_upgraded       = 0;
	unsigned char has_subprotocol   = 0;
	char *beg_resource     = NULL;
	char *end_resource     = NULL;
	char version[2]={0};
	char *connection_field = NULL;
	char *web_socket_field = NULL;
	static const char uri_scheme[] = "http://";
	eEsifError rc = ESIF_OK;

	#define MAX_SIZE 1000

	const char *beg_input_frame = (const char*)framePtr;
	const char *end_input_frame = (const char*)framePtr + incomingFrameLen;

	if (!strstr((const char*)framePtr, "\r\n\r\n")) {
		return INCOMPLETE_FRAME;
	}

	if (memcmp(framePtr, ("GET "), 4) != 0) {
		return ERROR_FRAME;
	}

	beg_resource = strchr((const char*)framePtr, ' ');
	if (!beg_resource) {
		return ERROR_FRAME;
	}

	beg_resource++;

	end_resource = strchr(beg_resource, ' ');
	if (!end_resource) {
		return ERROR_FRAME;
	}

	if (protPtr->webpage) {
		esif_ccb_free(protPtr->webpage);
		protPtr->webpage = NULL;
	}

	protPtr->webpage = (char*)esif_ccb_malloc(end_resource - beg_resource + 1);
	if (NULL == protPtr->webpage) {
		return ERROR_FRAME;
	}

	if (esif_ccb_sscanf(beg_input_frame, ("GET %s HTTP/1.1\r\n"), SCANFBUF(protPtr->webpage, (UInt32)(end_resource - beg_resource + 1)) ) != 1) {
		return ERROR_FRAME;
	}

	beg_input_frame = strstr(beg_input_frame, "\r\n") + 2;
	if (!beg_input_frame) {
		return ERROR_FRAME;
	}

	memset(version, 0, sizeof(version));

	while (beg_input_frame < end_input_frame && beg_input_frame[0] != '\r' && beg_input_frame[1] != '\n') {
		if (memcmp(beg_input_frame, HOST_FIELD, esif_ccb_strlen(HOST_FIELD, MAX_SIZE)) == 0) {
			beg_input_frame += esif_ccb_strlen(HOST_FIELD, MAX_SIZE);
			esif_ccb_free(protPtr->hostField);
			protPtr->hostField  = esif_ws_socket_get_field_value(beg_input_frame);
			if (NULL == protPtr->hostField)
				rc = ESIF_E_NO_MEMORY;
		} else if (memcmp(beg_input_frame, ORIGIN_FIELD, esif_ccb_strlen(ORIGIN_FIELD, MAX_SIZE)) == 0) {
			beg_input_frame  += esif_ccb_strlen(ORIGIN_FIELD, MAX_SIZE);
			esif_ccb_free(protPtr->originField);
			protPtr->originField = esif_ws_socket_get_field_value(beg_input_frame);
			if (NULL == protPtr->originField)
				rc = ESIF_E_NO_MEMORY;
		} else if (memcmp(beg_input_frame, WEB_SOCK_PROT_FIELD, esif_ccb_strlen(WEB_SOCK_PROT_FIELD, MAX_SIZE)) == 0) {
			beg_input_frame += esif_ccb_strlen(WEB_SOCK_PROT_FIELD, MAX_SIZE);
			esif_ccb_free(protPtr->web_socket_field);
			protPtr->web_socket_field = esif_ws_socket_get_field_value(beg_input_frame);
			has_subprotocol = 1;
			if (NULL == protPtr->web_socket_field)
				rc = ESIF_E_NO_MEMORY;
		} else if (memcmp(beg_input_frame, WEB_SOCK_KEY_FIELD, esif_ccb_strlen(WEB_SOCK_KEY_FIELD, MAX_SIZE)) == 0) {
			beg_input_frame += esif_ccb_strlen(WEB_SOCK_KEY_FIELD, MAX_SIZE);
			esif_ccb_free(protPtr->keyField);
			protPtr->keyField   = esif_ws_socket_get_field_value(beg_input_frame);
			if (NULL == protPtr->keyField)
				rc = ESIF_E_NO_MEMORY;
		} else if (memcmp(beg_input_frame, WEB_SOCK_VERSION_FIELD, esif_ccb_strlen(WEB_SOCK_VERSION_FIELD, MAX_SIZE)) == 0) {
			beg_input_frame += esif_ccb_strlen(WEB_SOCK_VERSION_FIELD, MAX_SIZE);
			esif_ws_socket_copy_line(beg_input_frame, version);
		} else if (memcmp(beg_input_frame, CONNECTION_FIELD, esif_ccb_strlen(CONNECTION_FIELD, MAX_SIZE)) == 0) {
			beg_input_frame += esif_ccb_strlen(CONNECTION_FIELD, MAX_SIZE);
			connection_field = NULL;
			connection_field = esif_ws_socket_get_field_value(beg_input_frame);
			if (NULL == connection_field) {
				rc = ESIF_E_NO_MEMORY;
			}
			else {
				esif_ws_socket_convert_to_lower_case(connection_field);
				if (strstr(connection_field, UPGRADE_FIELD) != NULL) {
					;	
				}
				esif_ccb_free(connection_field);
				connection_field = NULL;
			}
		} else if (memcmp(beg_input_frame, ALT_UPGRADE_FIELD, esif_ccb_strlen(ALT_UPGRADE_FIELD, MAX_SIZE)) == 0) {
			beg_input_frame += esif_ccb_strlen(ALT_UPGRADE_FIELD, MAX_SIZE);
			web_socket_field = NULL;
			web_socket_field = esif_ws_socket_get_field_value(beg_input_frame);
			if (NULL == web_socket_field) {
				rc = ESIF_E_NO_MEMORY;
			}
			else {
				esif_ws_socket_convert_to_lower_case(web_socket_field);
				if (memcmp(web_socket_field, WEB_SOCK_FIELD, esif_ccb_strlen(WEB_SOCK_FIELD, MAX_SIZE)) == 0) {
					is_upgraded = 1;
				}
				esif_ccb_free(web_socket_field);
				web_socket_field = NULL;
			}
		}

		beg_input_frame = strstr(beg_input_frame, "\r\n") + 2;

		if (!beg_input_frame) {
			return ERROR_FRAME;
		}
	}	/* End of while loop */

	/* Verify WebSocket Origin (No current HTTPS support) */
	if (protPtr->hostField && protPtr->originField && (esif_ccb_strncmp(protPtr->originField, uri_scheme, sizeof(uri_scheme)-1) == 0)) {
		char *origin = protPtr->originField + esif_ccb_strlen(uri_scheme, sizeof(uri_scheme));
		if (esif_ccb_stricmp(protPtr->hostField, origin) != 0) {
			rc = ESIF_E_NOT_SUPPORTED;
		}
	}

	if (rc != ESIF_OK || !protPtr->hostField) {
		protPtr->frameType = ERROR_FRAME;
	} else if (protPtr->hostField && !protPtr->keyField) {
		protPtr->frameType = HTTP_FRAME;
	} else if (protPtr->keyField || !is_upgraded || has_subprotocol || memcmp(version, VERSION_FIELD, esif_ccb_strlen(VERSION_FIELD, 3)) == 0) {
		protPtr->frameType = OPENING_FRAME;
	}

	return protPtr->frameType;
}


eEsifError esif_ws_socket_build_protocol_change_response(
	const ProtocolPtr protPtr,
	char *outgoingFrame,
	size_t outgoingFrameBufferSize,
	size_t *outgoingFrameSizePtr
	)
{
	int num_bytes=0;
	char shaHash[20]={0};
	char *response_key = NULL;
	eEsifError rc = ESIF_OK;

	UInt32 length = (UInt32)esif_ccb_strlen(protPtr->keyField, WS_PROT_KEY_SIZE_MAX) + (UInt32)esif_ccb_strlen(KEY, WS_PROT_KEY_SIZE_MAX);
	response_key = (char*)esif_ccb_malloc(length + 1);

	if ( NULL == response_key) {
		return ESIF_E_NO_MEMORY;
	}

	esif_ccb_memcpy(response_key, protPtr->keyField, esif_ccb_strlen(protPtr->keyField, WS_PROT_KEY_SIZE_MAX));
	esif_ccb_memcpy(&(response_key[esif_ccb_strlen(protPtr->keyField, WS_PROT_KEY_SIZE_MAX)]), KEY, esif_ccb_strlen(KEY, WS_PROT_KEY_SIZE_MAX));

	memset(shaHash, 0, sizeof(shaHash));
	esif_ws_algo_hash_sha_algo(shaHash, response_key, length * 8);
	esif_ws_algo_encode_base64_value(response_key, shaHash, 20);

	num_bytes = esif_ccb_sprintf(outgoingFrameBufferSize, (char*)outgoingFrame,
		"HTTP/1.1 101 Switching Protocols\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Accept: %s\r\n\r\n",
		response_key);

	*outgoingFrameSizePtr = num_bytes;

	if (response_key) {
		esif_ccb_free(response_key);
		response_key = NULL;
	}

	return rc;
}


void esif_ws_socket_build_payload(
	const char *data,
	size_t dataLength,
	WsSocketFramePtr framePtr,
	size_t bufferSize,
	size_t *outgoingFrameSizePtr,
	FrameType frameType
	)
{
	size_t hdrLen = 0;
	char *frameDataPtr = NULL;

	if (dataLength > 0) {
		ESIF_ASSERT(data);
	}

	ESIF_ASSERT(framePtr != NULL);
	ESIF_ASSERT(outgoingFrameSizePtr != NULL);

	*outgoingFrameSizePtr  = 0;
	framePtr->header.asU16 = 0;

	/* Set the FIN flag */
	framePtr->header.s.fin = 1;

	/* Set the frame type */
	framePtr->header.s.opcode = frameType;

	if (dataLength <= WS_FRAME_SIZE_TYPE_1_MAX_PAYLOAD) {
		framePtr->header.s.payLen = (UInt8)dataLength;
		frameDataPtr = framePtr->u.T1_NoMask.data;

	} else if (dataLength <= WS_FRAME_SIZE_TYPE_2_MAX_PAYLOAD) {
		UInt16 payLoadLength = htons((UInt16)dataLength);
		framePtr->header.s.payLen = WS_FRAME_SIZE_TYPE_2;
		framePtr->u.T2_NoMask.extPayLen = payLoadLength;
		frameDataPtr = framePtr->u.T2_NoMask.data;

	} else {
		UInt64 payLoadLength = htonll((UInt64)dataLength);
		framePtr->header.s.payLen = WS_FRAME_SIZE_TYPE_3;
		framePtr->u.T3_NoMask.extPayLen = payLoadLength;
		frameDataPtr = framePtr->u.T3_NoMask.data;
	}

	hdrLen = esif_ws_socket_get_header_len(framePtr, bufferSize);
	if ((dataLength + hdrLen) <= bufferSize) {
		esif_ccb_memcpy(frameDataPtr, data, dataLength);
		*outgoingFrameSizePtr = hdrLen + dataLength;
	}
}


FrameType esif_ws_socket_get_subsequent_frame_type(
	WsSocketFramePtr framePtr,
	size_t incomingFrameLen,
	char **dataPtr,
	size_t *dataLength,
	size_t *bytesRemaining
	)
{
	size_t hdrLen = 0;
	size_t payloadLength = 0;
	UInt8 mode = 0;
	FrameType frameType = INCOMPLETE_FRAME;
	UInt8 *maskingKey = NULL;
	size_t i=0;

	*bytesRemaining = 0;

	hdrLen = esif_ws_socket_get_header_len(framePtr, incomingFrameLen);
	if (0 == hdrLen) {
		return INCOMPLETE_FRAME;
	}

	if (framePtr->header.s.rsvd1 != 0) {
		return ERROR_FRAME;
	}

	if (framePtr->header.s.fin != 1) {
		return ERROR_FRAME;
	}

	if (!esif_ws_socket_is_mask_enabled(framePtr)) {
		return ERROR_FRAME;
	}

	mode = (UInt8)framePtr->header.s.opcode;
	if (mode == TEXT_FRAME || mode == BINARY_FRAME || mode == CLOSING_FRAME || mode == PING_FRAME || mode == PONG_FRAME) {
		frameType = (FrameType)mode;

		payloadLength = esif_ws_socket_get_payload_size(framePtr, incomingFrameLen, &frameType);
		if (payloadLength <= 0) {
			return frameType;
		}

		if (payloadLength > (incomingFrameLen - hdrLen)) {
			return INCOMPLETE_FRAME;
		}

		if (payloadLength < (incomingFrameLen - hdrLen)) {
			*bytesRemaining = incomingFrameLen - hdrLen - payloadLength;
		}

		maskingKey  = esif_ws_socket_get_mask(framePtr, incomingFrameLen);
		if (NULL == maskingKey) {
			ESIF_ASSERT(ESIF_FALSE); /* Should never happen, all should have been checked by here */
			return ERROR_FRAME;
		}

		*dataPtr = (char *)(maskingKey + sizeof(framePtr->u.T1.maskKey));
		*dataLength = payloadLength;

		for (i = 0; i < *dataLength; i++) {
			(*dataPtr)[i] = (*dataPtr)[i] ^ maskingKey[i % 4];
		}
		return frameType;
	}
	return ERROR_FRAME;
}


static void esif_ws_socket_convert_to_lower_case(char *string)
{
	int i=0;
	for (i = 0; string[i]; i++)
		string[i] = (char)tolower(string[i]);
}


static void esif_ws_socket_copy_line (
	const char *source,
	char *destination
	)
{
	u32 newLength = (u32)((char*)strstr(source, "\r\n") - source);

	esif_ccb_memcpy(destination, source, newLength);
}


static char *esif_ws_socket_get_field_value(const char *source)
{
	char *destination   = NULL;
	u32 adjusted_length = (u32)(strstr(source, "\r\n") - source);
	destination = (char*)esif_ccb_malloc(adjusted_length + 1);

	if (destination == NULL) {
		return destination;
	}

	esif_ccb_memcpy(destination, source, adjusted_length);
	destination[adjusted_length] = 0;

	return destination;
}


static size_t esif_ws_socket_get_payload_size (
	const WsSocketFramePtr framePtr,
	size_t incomingFrameLen,
	FrameTypePtr frameTypePtr 
	)
{
	size_t payLen = 0;
	size_t hdrLen = 0;

	hdrLen = esif_ws_socket_get_header_len(framePtr, incomingFrameLen);
	if (0 == hdrLen) {
		*frameTypePtr  = INCOMPLETE_FRAME;
		return 0;
	}

	/* Get the base payload type/length */
	payLen = framePtr->header.s.payLen;

	if (payLen == WS_FRAME_SIZE_TYPE_2) {
		payLen = (size_t) htons(framePtr->u.T2.extPayLen);

	} else if (payLen == WS_FRAME_SIZE_TYPE_3) {
		if (((u8)framePtr->u.T3.extPayLen) > MAXIMUM_SIZE) {
			*frameTypePtr  = ERROR_FRAME;
			return 0;
		}

		payLen = (size_t) htonll(framePtr->u.T3.extPayLen);
	}

	return payLen;
}


/* Returns 0 if an invalid header detected */
static size_t esif_ws_socket_get_header_len(
	const WsSocketFramePtr framePtr,
	size_t frameLen
	)
{
	size_t hdrLen = 0;

	ESIF_ASSERT(framePtr != NULL);

	if (frameLen < sizeof(framePtr->header)) {
		goto exit;
	}

	hdrLen += sizeof(framePtr->header);

	/* Check if we have mask bits present or not */
	if (esif_ws_socket_is_mask_enabled(framePtr)) {
		hdrLen += sizeof(framePtr->u.T1.maskKey);
	}

	if (WS_FRAME_SIZE_TYPE_2 == framePtr->header.s.payLen) {
		hdrLen += sizeof(framePtr->u.T2_NoMask.extPayLen);
	} else if (WS_FRAME_SIZE_TYPE_3 == framePtr->header.s.payLen) {
		hdrLen += sizeof(framePtr->u.T3_NoMask.extPayLen);
	}

	if (frameLen < hdrLen) {
		hdrLen = 0;
	}
exit:
	return hdrLen;
}


/* Returns NULL if header error detected or not enabled */
static UInt8 *esif_ws_socket_get_mask(
	const WsSocketFramePtr framePtr,
	size_t frameLen
	)
{
	UInt8 *maskPtr = NULL;

	ESIF_ASSERT(framePtr != NULL);

	if (frameLen < sizeof(framePtr->header)) {
		goto exit;
	}

	/* Check if we have mask bits present or not */
	if (!esif_ws_socket_is_mask_enabled(framePtr)) {
		goto exit;
	}

	if (WS_FRAME_SIZE_TYPE_2 == framePtr->header.s.payLen) {
		maskPtr = (UInt8 *)&framePtr->u.T2.maskKey;
	} else if (WS_FRAME_SIZE_TYPE_3 == framePtr->header.s.payLen) {
		maskPtr = (UInt8 *)&framePtr->u.T3.maskKey;
	} else {
		maskPtr = (UInt8 *)&framePtr->u.T1.maskKey;
	}

exit:
	return maskPtr;
}


static UInt8 esif_ws_socket_is_mask_enabled(
	const WsSocketFramePtr framePtr
	)
{
	return (UInt8)framePtr->header.s.maskFlag;
}


