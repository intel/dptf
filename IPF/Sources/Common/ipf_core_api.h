/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "esif_ccb.h"
#include "esif_ccb_rc.h"
#include "esif_sdk_event_type.h"
#include "esif_sdk_data.h"
#include "ipf_version.h"

///////////////////////////////////////////////////////////////////////////////
// General Definitions
///////////////////////////////////////////////////////////////////////////////

#define IPF_INTRO_LEN			1024	// Max Length of Intro/About/Banner

// Supported Protocols
#define IPC_PROTO_LEN			10		// Max Length of Protocol Name
#define IPC_PROTO_SCHEME		"://"	// URL Scheme Separator
#define IPC_PROTO_WEBSOCKET		"ws"	// WebSocket over TCP/IP
#define IPC_PROTO_NAMEDPIPE		"pipe"	// WebSocket over Logical Named Pipe (Unix Domain Socket)

// Default Server Addresses
#define	WEBSOCKET_SERVERADDR	"ws://127.0.0.1:18086"		// Public Access (WebSocket over TCP/IP port 18086 on localhost only)
#define	PUBLIC_SERVERADDR		"pipe://ipfsrv.public"		// Public Access (WebSocket over Logical Named Pipe)
#define	ELEVATED_SERVERADDR		"pipe://ipfsrv.elevated"	// Elevated Access (WebSocket over Logical Named Pipe)
#define	DEFAULT_SERVERADDR		PUBLIC_SERVERADDR

///////////////////////////////////////////////////////////////////////////////
// Type Declarations (Used by interface function definitions)
///////////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)

// IpfSessionInfo contains option connection parameters and data for IPF Client sessions
typedef union IpfSessionInfo_u {
	UInt32		revision;						// Revision Number
	struct {
		UInt32	revision;						// Revision Number
		char	appName[ESIF_NAME_LEN];			// Optional Client-Supplied App Name [Default=IPFCORE]
		char	appVersion[ESIF_VERSION_LEN];	// Optional Client-Supplied App Version [Default=IPFCORE Version]
		char	appDescription[ESIF_DESC_LEN];	// Optional Client-Supplied App Description [Default=IPFCORE Description]
		char	appIntro[IPF_INTRO_LEN];		// Optional Client-Supplied App Intro Banner [Default=IPFCORE Banner]
		char	serverAddr[MAX_PATH];			// Optional Client-Supplied Server Address [Default=DEFAULT_SERVERADDR]
		char	sdkVersion[ESIF_VERSION_LEN];	// Server SDK Version [Filled in by IpfCore_SessionCreate()]
	} v1;
} IpfSessionInfo;

#pragma pack(pop)

#define IPF_SESSIONINFO_REVISION_V1		1	// First Revision
#define IPF_SESSIONINFO_REVISION		IPF_SESSIONINFO_REVISION_V1

// Opaque Session Object Type used by Public Interface
typedef esif_handle_t IpfSession_t;

#define IPF_INVALID_SESSION		ESIF_INVALID_HANDLE
#define IPF_SESSION_FMT			"0x%llx"
//
// Event callback function prototype
// Prototype for the callback function invoked when events registered using
// IpfCore_SessionRegisterEvent received.
//
typedef void (ESIF_CALLCONV *IPFAPI_EVENT_CALLBACK)(
	IpfSession_t session,
	esif_event_type_t eventType,
	char *participantName,
	EsifData *dataPtr,
	esif_context_t context
	);


///////////////////////////////////////////////////////////////////////////////
// function Prototypes
///////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

//
// IPF CORE API Public Interface Functions
//
// These functions are linked in directly to IPF client applications via a static
// library and provide a thin "wrapper" layer around the actual IPF Interface so
// that apps do not have to deal directly with locating and loading Dynamic Libraries,
// calling exported Interface functions, or making calls through function pointers.
//
// These functions are not linked into any CORE Dynamic Library or Server components.
//

//
// Initializes internal structures and synchronization objects necessary
// for the API to be fully functional.  
// Note(s):  Must be called before all other CORE functions.
//
esif_error_t ESIF_CALLCONV IpfCore_Init(void);

//
// Releases resources and destroys all current sessions.
// Note(s):
// 1) Should be the last IPF function called.
// 2) No other API functions should be called after IpfCore_Exit is called
// except IpfCore_Init.
//
void ESIF_CALLCONV IpfCore_Exit(void);

//
// Creates and initializes a session object for use when connecting to
// the server using IpfCore_SessionConnect.
// Note(s):
// 1) If sessionInfoPtr is NULL, the default setting are used and the public
// role is assumed
// 2) If sessionInfoPtr is non-NULL, the sdkVersion is filled in with the
// IPF SDK Version Number of the Installed Driver and returned to the caller
//
IpfSession_t ESIF_CALLCONV IpfCore_SessionCreate(
	IpfSessionInfo *sessionInfoPtr
	);

//
// Permanently stops a connection and destroys the session object
// Note(s):
// 1) Should be called by the client when the session object is no longer
// required
// 2) After IpfCore_SessionDestroy is called, the session object may not be used
// again for any other API calls; including IpfCore_SessionConnect
//
void ESIF_CALLCONV IpfCore_SessionDestroy(
	IpfSession_t session
	);

//
// Establishes a session with the server using the specified session object
// Note(s):
// 1) The input session object is created by calling IpfCore_SessionCreate
// 2) A session may not be immediately available.  The caller poll for a 
// session until established (or times out; dependent on implementation.)
// 3) If a session is terminated because the connection is lost,
// a new session must be established using IpfCore_SessionConnect.
//
esif_error_t ESIF_CALLCONV IpfCore_SessionConnect(
	IpfSession_t session
	);

//
// Closes the connection for specified session
// Note(s):
// 1) After IpfCore_SessionDisconnect is called, the session object may not be used
// for access until IpfCore_SessionConnect is called again with the object to open
// a new session
//
void ESIF_CALLCONV IpfCore_SessionDisconnect(
	IpfSession_t session
	);

//
// Waits for the connection to close for any reason and the IPC I/O thread to exit
// This function is mainly useful for applications that want to start an IPC session
// and allow it process RPC calls using ESIF/App Interface until the connection is closed.
//
void ESIF_CALLCONV IpfCore_SessionWaitForStop(
	IpfSession_t session
	);

//
// Registers the caller for the given event type
// Note(s):
// 1) See IPFAPI_EVENT_CALLBACK for the required callback function protoype.
// 2) If participantName is NULL, the primary participant (system) is assumed
// 
//
esif_error_t ESIF_CALLCONV IpfCore_SessionRegisterEvent(
	IpfSession_t session,
	esif_event_type_t eventType,
	char *participantName,
	IPFAPI_EVENT_CALLBACK callback,
	esif_context_t context
	);

//
// Unregisters the caller from the given event type
// Note(s):
// 1) See IPFAPI_EVENT_CALLBACK for the required callback function protoype.
// 2) If participantName is NULL, the primary participant (system) is assumed
//
esif_error_t ESIF_CALLCONV IpfCore_SessionUnregisterEvent(
	IpfSession_t session,
	esif_event_type_t eventType,
	char *participantName,
	IPFAPI_EVENT_CALLBACK callback,
	esif_context_t context
	);

//
// Executes a command
// Note(s):
//
esif_error_t ESIF_CALLCONV IpfCore_SessionExecute(
	IpfSession_t session,
	EsifData *cmdPtr,
	EsifData *reqPtr,
	EsifData *respPtr
	);

#ifdef __cplusplus
}
#endif
