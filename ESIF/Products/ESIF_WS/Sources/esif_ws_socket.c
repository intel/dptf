/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#include "esif_ccb_string.h"
#include "esif_ccb_file.h"

#include "esif_ws_server.h"
#include "esif_ws_socket.h"

#ifdef ESIF_ATTR_OS_WINDOWS
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

// WebSocket Frame Header consists of:
// 1. UInt16 header bits
// 2. Optional UInt16 or UInt64 Extended Payload, if hdr.payloadSize=126 or 127
// 3. Optional UInt32 Mask Key, if hdr.maskFlag=1
//
#pragma warning(disable:4214)
#pragma pack(push, 1)
typedef union WsFrameHeader_u {

	struct {
		UInt16 frameType : 4;	// 3:0  - Websocket Frame Type
		UInt16 reserved : 3;	// 7:4  - Reserved
		UInt16 fin : 1;			// 8    - Final Fragment Flag
		UInt16 payloadSize : 7;	// 14:9 - Payload Size: 0-125=hdr.payloadSize, 126=T2.payloadSize, 127=Use T3.payloadSize
		UInt16 maskFlag : 1;	// 15   - Mask Flag: Mandatory for Client to Server, Optional for Server to Client
	} hdr;

	struct {				// If hdr.payloadSize <= 125
		UInt16 header;		// Header Bits
	} T1;

	struct {				// If hdr.payloadSize <= 125
		UInt16 header;		// Header Bits
		UInt32 maskKey;		// If hdr.maskFlag=1
	} T1_WithMask;

	struct {
		UInt16 header;		// Header Bits
		UInt16 payloadSize;	// If hdr.payloadSize=126 [Big-Endian]
	} T2;

	struct {
		UInt16 header;		// Header Bits
		UInt16 payloadSize;	// If hdr.payloadSize=126 [Big-Endian]
		UInt32 maskKey;		// If hdr.maskFlag=1
	} T2_WithMask;

	struct {
		UInt16 header;		// Header Bits
		UInt64 payloadSize;	// If hdr.payloadSize=127 [Big-Endian]
	} T3;

	struct {
		UInt16 header;		// Header Bits
		UInt64 payloadSize;	// If hdr.payloadSize=127 [Big-Endian]
		UInt32 maskKey;		// If hdr.maskFlag=1
	} T3_WithMask;

} WsFrameHeader, *WsFrameHeaderPtr;
#pragma pack(pop)

// Normalized WebSocket Frame
typedef struct WsFrame_s {
	FrameType		frameType;		// Frame Type (or First Frame Type if Multi-Fragment message)
	WsFrameHeader	header;			// Binary Websocket Header
	size_t			headerSize;		// Binary Websocket Header Size
	u8 *			payload;		// Binary Payload
	size_t			payloadSize;	// Binary Payload Size
	size_t			frameSize;		// Total Frame Size (headerSize + payloadSize)
} WsFrame, *WsFramePtr;

#define WSFRAME_HEADER_TYPE1		125	// hdr.payloadSize <= 125
#define WSFRAME_HEADER_TYPE2		126	// hdr.payloadSize = 126, T2.payloadSize = UInt16 [Big-Endian]
#define WSFRAME_HEADER_TYPE3		127	// hdr.payloadSize = 127, T3.payloadSize = UInt64 [Big-Endian]

#define WSFRAME_HEADER_TYPE1_MAX	125			// header Type1 max length
#define WSFRAME_HEADER_TYPE2_MAX	0xFFFF		// header Type2 max length
#define WSFRAME_HEADER_TYPE3_MAX	0x7FFFFFFE	// header Type3 max length

#define MAX_WEBSOCKET_BUFFER		(8*1024*1024)	// Max size of any single inbound websocket message (all combined fragments)
#define MAX_WEBSOCKET_SENDBUF		(1*1024*1024)	// Max size of outbound websocket send buffer (multiple messages)
#define MAX_WEBSOCKET_FRAGMENT_SIZE	(64*1024)		// Max WebSocket Fragment Size for Multi-Fragment TEXT Frames REST API Responses
#define MAX_WEBSOCKET_MSGID_LEN		15				// Max WebSocket MessageId Length (space for "%u:" output)
#define MAX_WEBSOCKET_RESPONSE_LEN	0x7FFFFFFE		// Max WebSocket Response Length
#define DEF_WEBSOCKET_RESPONSE_LEN	256				// Default WebSocket REST API Response Buffer

// Compute Websocket Header Size based on Payload Size or 0 if invalid payload_size
static size_t WebSocket_HeaderSize(size_t payload_size, Bool mask_flag)
{
	size_t header_size = 0;
	WsFrameHeader header = { 0 };

	if (payload_size < WSFRAME_HEADER_TYPE1_MAX) {
		header_size = sizeof(header.T1);
	}
	else if (payload_size < WSFRAME_HEADER_TYPE2_MAX) {
		header_size = sizeof(header.T2);
	}
	else if (payload_size < WSFRAME_HEADER_TYPE3_MAX) {
		header_size = sizeof(header.T3);
	}

	if (mask_flag) {
		header_size += sizeof(header.T1_WithMask.maskKey);
	}
	return header_size;
}

// Whitelisted ESIF Shell Commands UI clients can execute through REST API
// NOTE: This list must be sorted alphabetically so we can do a binary search
static char *g_RestApiWhitelist[] = {
	"about",
	"actionstart",
	"apps",
	"arbitrator",
	"capture",
	"config",
	"devices",
	"dptf",
	"echo",
	"format",
	"getp",
	"getp_part",
	"idsp",
	"log",
	"participant",
	"participantlog",
	"participants",
	"rstp",
	"rstp_part",
	"setp",
	"setp_part",
	"status",
	"tableobject",
	"trace",
	"ui",
	NULL
}; 

// Get Frame Data from Websocket Request
static esif_error_t Websocket_GetFrame(
	WsFramePtr frame,
	u8 *buffer,
	size_t buf_len)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	WsFrameHeaderPtr header = (WsFrameHeaderPtr)buffer;

	if (frame && buffer) {
		size_t header_size = sizeof(header->T1);
		size_t payload_size = 0;
		UInt32 mask_key = 0;
		size_t j = 0;

		esif_ccb_memset(frame, 0, sizeof(*frame));

		// Incomplete Header
		if (buf_len < sizeof(header->hdr)) {
			buf_len = 0;
		}
		// PayloadSize 0..125 in hdr.payloadSize
		else if (header->hdr.payloadSize <= WSFRAME_HEADER_TYPE1) {
			if (header->hdr.maskFlag) {
				header_size = sizeof(header->T1_WithMask);
			}

			if (buf_len >= header_size) {
				payload_size = (size_t)header->hdr.payloadSize;
				if (header->hdr.maskFlag) {
					mask_key = header->T1_WithMask.maskKey;
				}
			}
		}
		// PayloadSize is UInt16 in T2.payloadSize
		else if (header->hdr.payloadSize == WSFRAME_HEADER_TYPE2) {
			if (header->hdr.maskFlag) {
				header_size = sizeof(header->T2_WithMask);
			}
			else {
				header_size = sizeof(header->T2);
			}

			if (buf_len >= header_size) {
				payload_size = (size_t)esif_ccb_htons(header->T2.payloadSize);
				if (header->hdr.maskFlag) {
					mask_key = header->T2_WithMask.maskKey;
				}
			}
		}
		// PayloadSize is UInt64 in T3.payloadSize
		else  if (header->hdr.payloadSize == WSFRAME_HEADER_TYPE3) {
			if (header->hdr.maskFlag) {
				header_size = sizeof(header->T3_WithMask);
			}
			else {
				header_size = sizeof(header->T3);
			}

			if (buf_len >= header_size) {
				payload_size = (size_t)esif_ccb_htonll(header->T3.payloadSize);
				if (header->hdr.maskFlag) {
					mask_key = header->T3_WithMask.maskKey;
				}
			}
		}

		// Verify Full Header and Payload
		if (buf_len < header_size + payload_size) {
			rc = ESIF_E_WS_INCOMPLETE;
		}
		else if (header_size > sizeof(frame->header)) {
			rc = ESIF_E_REQ_SIZE_TYPE_MISTMATCH;
		}
		else {
			frame->frameType = header->hdr.frameType;
			esif_ccb_memcpy(&frame->header, header, header_size);
			frame->headerSize = header_size;
			frame->payload = buffer + header_size;
			frame->payloadSize = payload_size;
			frame->frameSize = header_size + payload_size;

			// Unmask Data
			if (frame->header.hdr.maskFlag) {
				for (j = 0; j < frame->payloadSize; j++) {
					frame->payload[j] = frame->payload[j] ^ ((UInt8 *)&mask_key)[j % 4];
				}
			}
			rc = ESIF_OK;
		}
	}
	return rc;
}

// Build a WebSocket Frame using the given output buffer
static esif_error_t WebSocket_BuildFrame(
	WsFramePtr frame,		// Normalized WebSocket Frame
	void *out_buf,			// Frame Output Buffer (Header+Payload)
	size_t out_buf_len,		// Frame Output Buffer Length
	void *data_buf,			// Frame Payload Data
	size_t data_len,		// Frame Payload Data Length
	FrameType frameType,	// Frame Type
	FinType finType			// FIN Type
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (frame && (out_buf || out_buf_len == 0) && (data_buf || data_len == 0)) {
		size_t header_size = 0;
		esif_ccb_memset(frame, 0, sizeof(*frame));

		rc = ESIF_OK;
		frame->frameType = frameType;
		frame->header.hdr.frameType = frameType;
		frame->header.hdr.fin = finType;

		if (data_len <= WSFRAME_HEADER_TYPE1_MAX) {
			header_size = sizeof(frame->header.T1);
			frame->header.hdr.payloadSize = (UInt8)data_len;
		}
		else if (data_len <= WSFRAME_HEADER_TYPE2_MAX) {
			header_size = sizeof(frame->header.T2);
			frame->header.hdr.payloadSize = WSFRAME_HEADER_TYPE2;
			frame->header.T2.payloadSize = esif_ccb_htons((UInt16)data_len);
		}
		else if (data_len <= WSFRAME_HEADER_TYPE3_MAX) {
			header_size = sizeof(frame->header.T3);
			frame->header.hdr.payloadSize = WSFRAME_HEADER_TYPE3;
			frame->header.T3.payloadSize = esif_ccb_htonll((UInt64)data_len);
		}
		else {
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		}

		if (rc == ESIF_OK) {
			if (out_buf && header_size + data_len > out_buf_len) {
				rc = ESIF_E_NEED_LARGER_BUFFER;
			}
			else if (out_buf) {
				esif_ccb_memcpy(out_buf, &frame->header, header_size);
				frame->headerSize = header_size;
				frame->payloadSize = data_len;
				frame->frameSize = header_size + data_len;
				if (data_buf && data_len > 0) {
					esif_ccb_memcpy((u8 *)out_buf + header_size, data_buf, data_len);
					frame->payload = (u8 *)out_buf + header_size;
				}
			}
		}
	}
	return rc;
}

// Execute an Internal REST API Command within this Server
char *WebServer_ExecRestCmdInternal(
	WebServerPtr self,
	const char *rest_cmd,
	const char *prefix)
{
	char *response = NULL;

	// <appname> subcmd [parameters]
	if (self && rest_cmd) {
		prefix = (prefix ? prefix : "");

		// Extract <appname>
		char thisapp[ESIF_NAME_LEN] = { 0 };
		for (size_t k = 0; k < sizeof(thisapp) - 1 && *rest_cmd && !isspace(*rest_cmd); k++) {
			thisapp[k] = *rest_cmd++;
		}
		while (*rest_cmd && isspace(*rest_cmd)) {
			rest_cmd++;
		}

		// Verify <appname>==<libname> or <appname>==WS_APPNAME_LOCALSERVER
		const char *appname = EsifWsAppName();
		if (appname && esif_ccb_stricmp(thisapp, appname) != 0 && esif_ccb_stricmp(thisapp, WS_APPNAME_LOCALSERVER) != 0) {
			return response;
		}

		// Extract subcmd
		char subcmd[ESIF_NAME_LEN] = { 0 };
		for (size_t k = 0; k < sizeof(subcmd) - 1 && *rest_cmd && !isspace(*rest_cmd); k++) {
			subcmd[k] = *rest_cmd++;
		}
		while (*rest_cmd && isspace(*rest_cmd)) {
			rest_cmd++;
		}

		// <appname> fetch <filename>
		if (esif_ccb_stricmp(subcmd, "fetch") == 0 && *rest_cmd && *rest_cmd != '.') {
			const char *docRoot = EsifWsDocRoot();
			char pathname[MAX_PATH] = {0};
			esif_ccb_sprintf(sizeof(pathname), pathname, "%s%s%s", docRoot, ESIF_PATH_SEP, rest_cmd);
			struct stat st = {0};
			char *filebuf = NULL;
			size_t prefix_len = esif_ccb_strlen(prefix, MAX_WEBSOCKET_MSGID_LEN);
			if (esif_ccb_stat(pathname, &st) == 0 && (filebuf = esif_ccb_malloc(prefix_len + st.st_size + 1)) != NULL) {
				FILE *fp = esif_ccb_fopen(pathname, "rb", NULL);
				if (fp) {
					esif_ccb_strcpy(filebuf, prefix, prefix_len + 1);
					size_t bytes = esif_ccb_fread(filebuf + prefix_len, st.st_size, 1, st.st_size, fp);
					if (bytes == (size_t)st.st_size) {
						response = filebuf;
						filebuf = NULL;
					}
					esif_ccb_fclose(fp);
				}
				esif_ccb_free(filebuf);
			}
		}
	}
	return response;
}

// Execute a request against the REST API
esif_error_t WebServer_WebSocketExecRestCmd(
	WebServerPtr self,
	WebClientPtr client,
	char *request,
	size_t request_len,
	char **response_ptr,
	size_t *response_len_ptr)
{
	int rc = ESIF_E_PARAMETER_IS_NULL;

	UNREFERENCED_PARAMETER(client);
	if (request && response_ptr && response_len_ptr) {
		UInt32 msg_id = (UInt32)atoi(request);
		char *shell_cmd = esif_ccb_strchr(request, ':');
		char response_buf[MAX_WEBSOCKET_MSGID_LEN + DEF_WEBSOCKET_RESPONSE_LEN] = { 0 };
		char *response = NULL;
		char *errmsg = "ERROR";
		
		*response_ptr = NULL;
		*response_len_ptr = 0;

		// REST Responses always begin with given message ID
		esif_ccb_sprintf(sizeof(response_buf), response_buf, "%u:", msg_id);
		
		if (shell_cmd > request) {

			// Copy shell command to beginning of request buffer to guarantee NULL-terminated shell_cmd
			size_t shell_cmd_len = request_len - (size_t)(shell_cmd - request);
			esif_ccb_memmove(request, shell_cmd + 1, shell_cmd_len - 1);
			shell_cmd = request;
			shell_cmd[shell_cmd_len - 1] = '\0';
			shell_cmd[shell_cmd_len] = '\0';
			errmsg = NULL;
			char *rest_cmd = shell_cmd;
			size_t rest_cmd_len = shell_cmd_len;

			// Ad-Hoc UI Shell commands begin with "0:" so verify ESIF Shell is enabled and command is valid
			if (msg_id == 0 && !EsifWsShellEnabled()) {
				errmsg = "Shell Disabled";
			}
			// Verify ESIF Shell is enabled and command is not in blacklist, or is in whitelist if defined
			else {
				static char *default_whitelist[] = { NULL }; // Allow All except blacklist
				static char *blacklist[] = { "exit", "quit", "shell", "web", NULL };
				const char *skip_cmds[] = { "format text && ", "format xml && ", NULL };
				char **whitelist = default_whitelist;
				int whitelist_len = ESIF_ARRAY_LEN(default_whitelist) - 1;
				Bool blocked = ESIF_FALSE;
				int j = 0;

				// Enforce REST API Whitelist except for Listeners configured with the whitelist disabled
				for (size_t k = 0; k < ESIF_ARRAY_LEN(self->listeners); k++) {
					if (!(self->listeners[k].flags & WS_FLAG_NOWHITELIST)) {
						whitelist = g_RestApiWhitelist;
						whitelist_len = ESIF_ARRAY_LEN(g_RestApiWhitelist) - 1;
						break;
					}
				}

				// Skip any commands in the skip_cmds list in before checking the shell command against the blacklist/whitelist
				while (skip_cmds[j]) {
					size_t cmd_len = esif_ccb_strlen(skip_cmds[j], MAX_PATH);
					if (cmd_len < rest_cmd_len && esif_ccb_strnicmp(rest_cmd, skip_cmds[j], cmd_len) == 0) {
						rest_cmd += cmd_len;
						rest_cmd_len -= cmd_len;
						j = 0;
						continue;
					}
					j++;
				}

				// Extract Command Name from REST command
				char cmd_name[ESIF_NAME_LEN] = { 0 };
				for (size_t k = 0; k < sizeof(cmd_name) - 1 &&  rest_cmd[k] && !isspace(rest_cmd[k]); k++) {
					cmd_name[k] = rest_cmd[k];
				}

				// Verify the shell command is in Whitelist if defined using a binary search
				if (whitelist[0] && cmd_name[0]) {
					int start = 0, end = whitelist_len - 1, node = whitelist_len / 2;
					while (start <= end && node >= 0 && node <= whitelist_len - 1) {
						int comp = esif_ccb_stricmp(cmd_name, whitelist[node]);
						if (comp == 0) {
							break;
						}
						else if (comp > 0) {
							start = node + 1;
						}
						else {
							end = node - 1;
						}
						node = (end - start) / 2 + start;
					}
					if (start > end) {
						blocked = ESIF_TRUE;
					}
				}

				// Execute Internal REST API Commands within this web server process rather than via ESIF Interface
				if (blocked || !whitelist[0]) {
					response = WebServer_ExecRestCmdInternal(self, rest_cmd, response_buf);
					if (response) {
						blocked = ESIF_FALSE;
						shell_cmd = NULL;
					}
				}

				// Verify the shell command is not in Blacklist if defined
				if (!blocked && !response && blacklist[0] && cmd_name[0]) {
					for (j = 0; blacklist[j] != NULL; j++) {
						if (esif_ccb_stricmp(cmd_name, blacklist[j]) == 0) {
							blocked = ESIF_TRUE;
							break;
						}
					}
				}
				if (blocked) {
					errmsg = "Unsupported Command";
				}
			}

			// Exit if shell or command unavailable
			if (errmsg) {
				esif_ccb_strcat(response_buf, errmsg, sizeof(response_buf));
				shell_cmd = NULL;
			}

			// Execute Shell Command against REST API
			if (shell_cmd) {
				if (atomic_read(&self->isActive)) {
					response = EsifWsShellExec(shell_cmd, shell_cmd_len + 1, response_buf, esif_ccb_strlen(response_buf, sizeof(response_buf)));
				}
			}

			// Strip Non-ASCII characters from REST API results
			if (response) {
				unsigned char *source = (unsigned char *)response;
				unsigned char *target = source;
				while (*source) {
					if (esif_ccb_strchr("\r\n\t", *(char *)source) != NULL || (*source >= ' ' && *source <= '~')) {
						*target++ = *source;
					}
					source++;
				}
				*target = '\0';
			}

			// Log Errors indicated by ESIF Error code or Error Message
			char *esif_error = (response ? esif_ccb_strstr(response, "ESIF_E_") : NULL);
			if ((errmsg || esif_error) && rest_cmd[0]) {
				char error_code[MAX_PATH] = { 0 };
				if (errmsg) {
					esif_ccb_strcpy(error_code, errmsg, sizeof(error_code));
				}
				else {
					for (size_t j = 0; j < sizeof(error_code) - 1 && esif_error[j] && (isalnum(esif_error[j]) || esif_error[j] == '_'); j++) {
						error_code[j] = esif_error[j];
					}
					// Ignore Specific Error Codes that create too much noise
					esif_error_t ignore_errors[] = {
						ESIF_E_NOT_FOUND,
						0
					};
					for (size_t j = 0; ignore_errors[j]; j++) {
						if (esif_ccb_strcmp(esif_rc_str(ignore_errors[j]), error_code) == 0) {
							esif_ccb_memset(error_code, 0, sizeof(error_code));
							break;
						}
					}
				}
				if (error_code[0]) {
					size_t maxcmd = 80;
					WS_TRACE_ERROR("REST API Error [IP=%s] (%s): [%.*s%s]\n",
						client->ipAddr,
						error_code,
						(int)maxcmd,
						rest_cmd,
						(rest_cmd_len > maxcmd + 1 ? "..." : "")
					);
				}
			}

			// Send REST API Response or Error message
			if ((response == NULL) && ((response = esif_ccb_strdup(response_buf)) == NULL)) {
				rc = ESIF_E_NO_MEMORY;
			}
			else {
				size_t response_len = esif_ccb_strlen(response, MAX_WEBSOCKET_RESPONSE_LEN);
				*response_ptr = response;
				*response_len_ptr = response_len;
				rc = ESIF_OK;
			}
		}
	}
	return rc;
}

// Process a Websocket Request and send a Response
esif_error_t WebServer_WebsocketResponse(
	WebServerPtr self,
	WebClientPtr client, 
	WsFramePtr request)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && client && request) {
		WsFrame outFrame = { 0 };

		switch (request->frameType) {

		case FRAME_CONTINUATION:
			// Ignore Continuation Frames; They will be processed when a FIN is received on the final Frame
			rc = ESIF_OK;
			break;

		case FRAME_TEXT:
		{	// Process Text Frames and send to REST API
			char *response = NULL;
			size_t response_len = 0;

			// Use a copy of the frame text to send to the rest API, including any prior fragments
			char *total_message = NULL;
			size_t prior_fragments_len = client->fragBufLen;
			size_t total_message_len = prior_fragments_len + request->payloadSize;
			if (total_message_len < MAX_WEBSOCKET_BUFFER) {
				total_message = (char*)esif_ccb_malloc(total_message_len + 1);
			}
			if (NULL == total_message) {
				rc = ESIF_E_NO_MEMORY;
			}
			else {
				// Combine the final fragment with the current connection's Fragment Buffer, if any
				if (prior_fragments_len > 0 && client->fragBuf != NULL) {
					WS_TRACE_DEBUG("WS Fragment Unbuffering: buflen=%zd msglen=%zd total=%zd\n", prior_fragments_len, request->payloadSize, total_message_len);
					esif_ccb_memcpy(total_message, client->fragBuf, prior_fragments_len);
					esif_ccb_free(client->fragBuf);
					client->fragBuf = NULL;
					client->fragBufLen = 0;
					client->msgType = FRAME_NULL;
				}
				esif_ccb_memcpy(total_message + prior_fragments_len, request->payload, request->payloadSize);
				total_message[total_message_len] = 0;
			}

			// Execute Command against REST API
			rc = WebServer_WebSocketExecRestCmd(
				self,
				client,
				total_message,
				total_message_len,
				&response,
				&response_len);

			// Send REST API Response
			if (rc == ESIF_OK) {
				size_t bytes_sent = 0;

				// Break up large Websocket Messages into Multiple Fragments
				while (rc == ESIF_OK && bytes_sent < response_len) {
					size_t header_size = WebSocket_HeaderSize(response_len - bytes_sent, ESIF_FALSE);
					size_t max_payload = (self->netBufLen <= header_size ? 0 : self->netBufLen - header_size);
					size_t fragment_size = esif_ccb_min(response_len - bytes_sent, max_payload);
					FrameType frame_type = (bytes_sent > 0 ? FRAME_CONTINUATION : FRAME_TEXT);
					FinType fin_type = (fragment_size < response_len - bytes_sent ? FIN_FRAGMENT : FIN_FINAL);

					if (fragment_size < 1) {
						rc = ESIF_E_NEED_LARGER_BUFFER;
					}
					else {
						rc = WebSocket_BuildFrame(
							&outFrame,
							self->netBuf, self->netBufLen,
							response + bytes_sent, fragment_size,
							frame_type,
							fin_type);
					}

					if (rc == ESIF_OK) {
						rc = WebClient_Write(client, self->netBuf, outFrame.frameSize);
					}
					bytes_sent += fragment_size;
				}
			}
			esif_ccb_free(total_message);
			esif_ccb_free(response);
			break;
		}
		case FRAME_BINARY:
			// Binary Frames currently unsupported; Discard Request and Ignored
			esif_ccb_free(client->fragBuf);
			client->fragBuf = NULL;
			client->fragBufLen = 0;
			client->msgType = FRAME_NULL;
			rc = ESIF_OK;
			break;

		case FRAME_CLOSING:
			// Send Closing Frame to Client and Disconnect
			rc = WebSocket_BuildFrame(
				&outFrame,
				self->netBuf, self->netBufLen,
				NULL, 0,
				FRAME_CLOSING,
				FIN_FINAL);

			if (rc == ESIF_OK) {
				rc = WebClient_Write(client, self->netBuf, outFrame.frameSize);
			}
			if (rc == ESIF_OK) {
				rc = ESIF_E_WS_DISC;
			}
			break;

		case FRAME_PING:
			// Respond to PING Frames with PONG Frame containing Ping's Data
			rc = WebSocket_BuildFrame(
				&outFrame,
				self->netBuf, self->netBufLen,
				request->payload, request->payloadSize,
				FRAME_PONG,
				FIN_FINAL);

			if (rc == ESIF_OK) {
				rc = WebClient_Write(client, self->netBuf, outFrame.frameSize);
			}
			break;

		case FRAME_PONG:
			// Handle unsolicited PONG (keepalive) messages from Internet Explorer 10 (Not required per RFC 6455 but allowed)
			rc = WebSocket_BuildFrame(
				&outFrame,
				self->netBuf, self->netBufLen,
				NULL, 0,
				FRAME_TEXT,
				FIN_FINAL);

			if (rc == ESIF_OK) {
				rc = WebClient_Write(client, self->netBuf, outFrame.frameSize);
			}
			break;
		default:
			break;
		}
	}
	return rc;
}

// Process a WebSocket Request, if a complete Message is available
esif_error_t WebServer_WebsocketRequest(
	WebServerPtr self,
	WebClientPtr client,
	u8 *buffer,
	size_t buf_len)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (self && client && buffer && buf_len > 0) {
		size_t bytesRemaining = 0;
		char *bufferRemaining = NULL;

		do {
			WsFrame request = { 0 };

			// If more frames remaining, copy them into buffer and reparse
			if (bufferRemaining != NULL && bytesRemaining > 0) {
				esif_ccb_memcpy(buffer, bufferRemaining, bytesRemaining);
				buf_len = bytesRemaining;
				bytesRemaining = 0;
			}
			
			// Parse WebSocket Header into a Request Frame
			rc = Websocket_GetFrame(&request, buffer, buf_len);

			if (rc == ESIF_OK && buf_len > request.frameSize) {
				bytesRemaining = buf_len - request.frameSize;
			}

			// Append this partial frame to the current connection's Receive Buffer, if any
			if (rc == ESIF_E_WS_INCOMPLETE) {
				size_t oldSize = client->recvBufLen;
				size_t newSize = oldSize + buf_len;
				u8 *newBuffer = NULL;
				if (newSize <= MAX_WEBSOCKET_BUFFER) {
					newBuffer = esif_ccb_realloc(client->recvBuf, newSize);
				}
				if (newBuffer == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				else {
					WS_TRACE_DEBUG("WS Frame Buffering: oldlen=%zd reqlen=%zd total=%zd\n", oldSize, buf_len, newSize);
					esif_ccb_memcpy(newBuffer + oldSize, buffer, buf_len);
					client->recvBuf = newBuffer;
					client->recvBufLen = newSize;
				}
				break;
			}

			// Append this message fragment to the current connection's Fragment Buffer, if any
			if (rc == ESIF_OK && request.header.hdr.fin == FIN_FRAGMENT) {
				size_t oldSize = client->fragBufLen;
				size_t newSize = oldSize + request.payloadSize;
				u8 *newBuffer = NULL;
				if (newSize <= MAX_WEBSOCKET_BUFFER) {
					newBuffer = esif_ccb_realloc(client->fragBuf, newSize);
				}
				if (newBuffer == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				else {
					WS_TRACE_DEBUG("WS Fragment Buffering: buflen=%zd msglen=%zd total=%zd\n", oldSize, request.payloadSize, newSize);
					esif_ccb_memcpy(newBuffer + oldSize, request.payload, request.payloadSize);
					client->fragBuf = newBuffer;
					client->fragBufLen = newSize;
				}
				// Multi-Fragment messages put real opcode in 1st Fragment only; All others are Continuation Frames, including FIN
				if (request.frameType != FRAME_CONTINUATION) {
					client->msgType = request.frameType;
				}
			}

			// Use First Fragment's Frame Type if FIN is set on (final) CONTINUATION frame
			if (request.frameType == FRAME_CONTINUATION && request.header.hdr.fin == FIN_FINAL) {
				request.frameType = client->msgType;
			}

			// Save remaining frames for reparsing if more than one frame received
			if (bytesRemaining > 0) {
				if (bufferRemaining == NULL) {
					bufferRemaining = (char *)esif_ccb_malloc(bytesRemaining);
					if (NULL == bufferRemaining) {
						rc = ESIF_E_NO_MEMORY;
					}
				}
				if (bufferRemaining) {
					esif_ccb_memcpy(bufferRemaining, request.payload + request.payloadSize, bytesRemaining);
				}
			}

			// Close Connection on Error
			if (rc != ESIF_OK) {
				WS_TRACE_DEBUG("Invalid Frame; Closing socket: rc=%s (%d) Type=%02hX FIN=%hd Len=%hd (%zd) Mask=%hd\n", esif_rc_str(rc), rc, request.header.hdr.frameType, request.header.hdr.fin, request.header.hdr.payloadSize, request.payloadSize, request.header.hdr.maskFlag);

				// Send Closing Frame to Client
				WsFrame outFrame = { 0 };
				rc = WebSocket_BuildFrame(
					&outFrame,
					self->netBuf, self->netBufLen,
					NULL, 0,
					FRAME_CLOSING,
					FIN_FINAL);

				if (rc == ESIF_OK) {
					rc = WebClient_Write(client, self->netBuf, outFrame.payloadSize);
				}
				if (rc == ESIF_OK) {
					rc = ESIF_E_WS_DISC;
				}
				break;
			}

			// Process a Complete Websocket Single or Multi-Fragment Message
			if (rc == ESIF_OK && request.header.hdr.fin == FIN_FINAL) {
				rc = WebServer_WebsocketResponse(self, client, &request);
			}

		} while (bytesRemaining > 0);

		esif_ccb_free(bufferRemaining);
	}
	return rc;
}
