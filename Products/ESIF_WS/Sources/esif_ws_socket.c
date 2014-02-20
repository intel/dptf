/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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


static void esif_ws_socket_convert_to_lower_case (char*);
static void esif_ws_socket_copy_line (const char*, char*);
static char*esif_ws_socket_get_field_value (const char*);
static size_t esif_ws_socket_get_payload_size (const UInt8*, size_t, UInt8*, enum frameType*);


/*
 *******************************************************************************
 ** PUBLIC
 *******************************************************************************
 */
void esif_ws_socket_initialize_frame (protocol *prot)
{
	esif_ccb_free(prot->hostField);
	esif_ccb_free(prot->originField);
	esif_ccb_free(prot->webpage);
	esif_ccb_free(prot->keyField);
	esif_ccb_free(prot->web_socket_field);
	prot->hostField   = NULL;
	prot->originField = NULL;
	prot->webpage     = NULL;
	prot->keyField    = NULL;
	prot->web_socket_field = NULL;
	prot->frame = EMPTY_FRAME;
}


void esif_ws_socket_initialize_message_buffer (msgBuffer *buf)
{
	esif_ccb_free(buf->msgReceive);
	buf->msgReceive = NULL;
	buf->rcvSize    = 0;
	esif_ccb_free(buf->msgSend);
	buf->msgSend    = NULL;
	buf->rcvSize    = 0;
}


enum frameType esif_ws_get_initial_frame_type (
	const UInt8 *incoming_frame,
	size_t incoming_frame_length,
	protocol *prot
	)
{
	unsigned char is_upgraded       = 0;
	unsigned char has_subprotocol   = 0;
	char *beg_resource     = NULL;
	char *end_resource     = NULL;
	char version[2]={0};
	char *connection_field = NULL;
	char *web_socket_field = NULL;
    eEsifError rc = ESIF_OK;

	#define MAX_SIZE 1000

	const char *beg_input_frame = (const char*)incoming_frame;
	const char *end_input_frame = (const char*)incoming_frame + incoming_frame_length;

	if (!strstr((const char*)incoming_frame, "\r\n\r\n")) {
		return INCOMPLETE_FRAME;
	}

	if (memcmp(incoming_frame, ("GET "), 4) != 0) {
		return ERROR_FRAME;
	}

	beg_resource = strchr((const char*)incoming_frame, ' ');

	if (!beg_resource) {
		return ERROR_FRAME;
	}

	beg_resource++;

	end_resource = strchr(beg_resource, ' ');

	if (!end_resource) {
		return ERROR_FRAME;
	}

	if (prot->webpage) {
		esif_ccb_free(prot->webpage);
		prot->webpage = NULL;
	}


	prot->webpage = (char*)esif_ccb_malloc(end_resource - beg_resource + 1);

	if (NULL == prot->webpage) {
		return ERROR_FRAME;
	}

	if (esif_ccb_sscanf(beg_input_frame, ("GET %s HTTP/1.1\r\n"), prot->webpage, (UInt32)(end_resource - beg_resource + 1) ) != 1) {
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
			esif_ccb_free(prot->hostField);
			prot->hostField  = esif_ws_socket_get_field_value(beg_input_frame);
			if (NULL == prot->hostField)
				rc = ESIF_E_NO_MEMORY;
		} else if (memcmp(beg_input_frame, ORIGIN_FIELD, esif_ccb_strlen(ORIGIN_FIELD, MAX_SIZE)) == 0) {
			beg_input_frame  += esif_ccb_strlen(ORIGIN_FIELD, MAX_SIZE);
			esif_ccb_free(prot->originField);
			prot->originField = esif_ws_socket_get_field_value(beg_input_frame);
			if (NULL == prot->originField)
				rc = ESIF_E_NO_MEMORY;
		} else if (memcmp(beg_input_frame, WEB_SOCK_PROT_FIELD, esif_ccb_strlen(WEB_SOCK_PROT_FIELD, MAX_SIZE)) == 0) {
			beg_input_frame += esif_ccb_strlen(WEB_SOCK_PROT_FIELD, MAX_SIZE);
			esif_ccb_free(prot->web_socket_field);
			prot->web_socket_field = esif_ws_socket_get_field_value(beg_input_frame);
			has_subprotocol = 1;
			if (NULL == prot->web_socket_field)
				rc = ESIF_E_NO_MEMORY;
		} else if (memcmp(beg_input_frame, WEB_SOCK_KEY_FIELD, esif_ccb_strlen(WEB_SOCK_KEY_FIELD, MAX_SIZE)) == 0) {
			beg_input_frame += esif_ccb_strlen(WEB_SOCK_KEY_FIELD, MAX_SIZE);
			esif_ccb_free(prot->keyField);
			prot->keyField   = esif_ws_socket_get_field_value(beg_input_frame);
			if (NULL == prot->keyField)
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

	if (rc != ESIF_OK || !prot->hostField) {
		prot->frame = ERROR_FRAME;
	} else if (prot->hostField && !prot->keyField) {
		prot->frame = HTTP_FRAME;
	} else if (prot->keyField || !is_upgraded || has_subprotocol || memcmp(version, VERSION_FIELD, esif_ccb_strlen(VERSION_FIELD, 3)) == 0) {
		prot->frame = OPENING_FRAME;
	}


	return prot->frame;
}


eEsifError esif_ws_socket_build_response_header (
	const protocol *prot,
	UInt8 *outgoingFrame,
	size_t *outgoingFrameLength
	)
{
	#define MAX_SIZE 1000
	int num_bytes=0;
	char shaHash[20]={0};
	char *response_key = NULL;
	eEsifError rc = ESIF_OK;

	UInt32 length = (UInt32)esif_ccb_strlen(prot->keyField, MAX_SIZE) + (UInt32)esif_ccb_strlen(KEY, MAX_SIZE);
	response_key = (char*)esif_ccb_malloc(length + 1);

	if ( NULL == response_key) {
		return ESIF_E_NO_MEMORY;
	}

	esif_ccb_memcpy(response_key, prot->keyField, esif_ccb_strlen(prot->keyField, MAX_SIZE));
	esif_ccb_memcpy(&(response_key[esif_ccb_strlen(prot->keyField, MAX_SIZE)]), KEY, esif_ccb_strlen(KEY, MAX_SIZE));


	memset(shaHash, 0, sizeof(shaHash));
	esif_ws_algo_hash_sha_algo(shaHash, response_key, length * 8);
	esif_ws_algo_encode_base64_value(response_key, shaHash, 20);

	num_bytes = esif_ccb_sprintf(*outgoingFrameLength, (char*)outgoingFrame,
								 ("HTTP/1.1 101 Switching Protocols\r\n"
								  "%s%s\r\n"
								  "%s%s\r\n"
								  "Sec-WebSocket-Accept: %s\r\n\r\n"),
								 "Upgrade: ",
								 "websocket",
								 "Connection: ",
								 "Upgrade",
								 response_key);

	*outgoingFrameLength = num_bytes;

	if (response_key) {
		esif_ccb_free(response_key);
		response_key = NULL;
	}

	return rc;
}


void esif_ws_socket_build_payload (
	const UInt8 *data,
	size_t dataLength,
	UInt8 *outgoingFrame,
	size_t *outgoingFrameLength,
	enum frameType frameType
	)
{
	if (dataLength > 0) {
		ESIF_ASSERT(data);
	}

	outgoingFrame[0] = (UInt8)(0x80 | frameType);

	if (dataLength <= 125) {
		outgoingFrame[1]     = (UInt8)dataLength;
		*outgoingFrameLength = 2;
	} else if (dataLength <= 0xFFFF) {
		UInt16 payLoadLength = htons((UInt16)dataLength);
		outgoingFrame[1]     = 126;
		esif_ccb_memcpy(&outgoingFrame[2], &payLoadLength, sizeof(payLoadLength));
		*outgoingFrameLength = 2 + sizeof(payLoadLength);
	} else {
		UInt32 payLoadLength = htonl((UInt32)dataLength);
		outgoingFrame[1]     = 127;
		esif_ccb_memcpy(&outgoingFrame[2], &payLoadLength, sizeof(payLoadLength));
		*outgoingFrameLength = 2 + sizeof(payLoadLength);
	}

	esif_ccb_memcpy(&outgoingFrame[*outgoingFrameLength], data, dataLength);
	*outgoingFrameLength += dataLength;
}


enum frameType esif_ws_socket_get_subsequent_frame_type (
	UInt8 *incoming_frame,
	size_t incoming_frame_length,
	UInt8 * *dataPtr,
	size_t *dataLength
	)
{
	UInt8 mode=0;
	UInt8 *maskingKey=NULL;
	UInt8 extraBytes=0;
	size_t payloadLength=0;
	enum frameType frameType=(enum frameType)0;
	size_t i=0;

	if (incoming_frame_length < 2) {
		return INCOMPLETE_FRAME;
	}

	if ((incoming_frame[0] & 0x70) != 0x0) {
		return ERROR_FRAME;
	}

	if ((incoming_frame[0] & 0x80) != 0x80) {
		return ERROR_FRAME;
	}

	if ((incoming_frame[1] & 0x80) != 0x80) {
		return ERROR_FRAME;
	}

	mode = incoming_frame[0] & 0x0F;
	if (mode == TEXT_FRAME || mode == BINARY_FRAME || mode == CLOSING_FRAME || mode == PING_FRAME || mode == PONG_FRAME) {
		frameType     = (enum frameType)mode;

		extraBytes    = 0;
		payloadLength = esif_ws_socket_get_payload_size(incoming_frame, incoming_frame_length, &extraBytes, &frameType);
		if (payloadLength > 0) {
			if (payloadLength < incoming_frame_length - 6 - extraBytes) {	// 4-maskingKey, 2-header
				return INCOMPLETE_FRAME;
			
			}

			maskingKey  = &incoming_frame[2 + extraBytes];

			*dataPtr    = &incoming_frame[2 + extraBytes + 4];
			*dataLength = payloadLength;


			for (i = 0; i < *dataLength; i++)
				(*dataPtr)[i] = (*dataPtr)[i] ^ maskingKey[i % 4];

		}

		return frameType;
	}

	return ERROR_FRAME;
}


static void esif_ws_socket_convert_to_lower_case (char *string)
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


static char*esif_ws_socket_get_field_value (const char *source)
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
	const UInt8 *incoming_frame,
	size_t incoming_frame_length,
	UInt8 *extraBytes,
	enum frameType *frameType
	)
{
	UInt16 secondTypeOfPayLoadLength=0;
	size_t firstTypeOfPayLoadLength = incoming_frame[1] & 0x7F;
	*extraBytes = 0;

	if ((firstTypeOfPayLoadLength == 0x7E && incoming_frame_length < 4) || (firstTypeOfPayLoadLength == 0x7F && incoming_frame_length < 10)) {
		*frameType = INCOMPLETE_FRAME;
		return 0;
	}

	if (firstTypeOfPayLoadLength == 0x7F && (incoming_frame[3] & 0x80) != 0x0) {
		*frameType = ERROR_FRAME;
		return 0;
	}

	if (firstTypeOfPayLoadLength == 0x7E) {
		secondTypeOfPayLoadLength = 0;
		*extraBytes = 2;
		esif_ccb_memcpy(&secondTypeOfPayLoadLength, &incoming_frame[2], *extraBytes);
		firstTypeOfPayLoadLength = secondTypeOfPayLoadLength;
	} else if (firstTypeOfPayLoadLength == 0x7F) {
		UInt64 ThirdTypeOfPayLoadLength = 0;
		*extraBytes = 8;
		esif_ccb_memcpy(&ThirdTypeOfPayLoadLength, &incoming_frame[2], *extraBytes);

		if (ThirdTypeOfPayLoadLength > MAXIMUM_SIZE) {
			*frameType = ERROR_FRAME;
			return 0;
		}

		firstTypeOfPayLoadLength = (size_t)ThirdTypeOfPayLoadLength;
	}

	return firstTypeOfPayLoadLength;
}


