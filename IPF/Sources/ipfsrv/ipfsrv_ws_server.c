/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "esif_ccb.h"
#include "esif_ccb_atomic.h"
#include "esif_ccb_lock.h"
#include "esif_ccb_file.h"
#include "esif_ccb_string.h"
#include "esif_link_list.h"

#include "ipf_ibinary.h"

#include "ipfsrv_appmgr.h"
#include "ipfsrv_ws_server.h"
#include "ipfsrv_ws_http.h"
#include "ipfsrv_ws_socket.h"
#include "ipf_ipc_codec.h"
#include "ipf_ipc_trxmgr.h"
#include "ipf_ipc_clisrv.h"
#include "ipf_core_api.h"


#define IPC_NAMEDPIPE_LOCKFILE	"ipfsrv.lock"	// Open Lock File used to prevent Socket File Folder from being Delete/Renamed by other processes

//////////////////////////////////////////////

// Get Number of Logical CPUs
#include <sys/sysinfo.h>
static ESIF_INLINE int esif_ccb_nprocs()
{
	return get_nprocs_conf();
}

static void *ESIF_CALLCONV Irpc_WorkerThread(void *ctx);

// Use Non-Blocking Socket I/O
#ifdef MSG_NOSIGNAL
#define WS_NONBLOCKING_FLAGS (MSG_NOSIGNAL|MSG_DONTWAIT)
#else
#define WS_NONBLOCKING_FLAGS 0
#endif
#define WS_SEND_FLAGS	0
#define WS_RECV_FLAGS	0

#define WS_NETWORK_BUFFER_LEN	(1*1024*1024)	// Network Buffer Size for HTTP/Websocket Send and Receive Buffer
#define WS_SOCKET_TIMEOUT		30				// Socket activity timeout waiting on blocking select() [2 or greater]

#define WS_MAX_CLIENT_SENDBUF	(8*1024*1024)	// Max size of client send buffer (multiple messages)
#define WS_MAX_CLIENT_RECVBUF	(8*1024*1024)	// Max size of client receive buffer (multiple messages)

#define WS_MAX_RPCQUEUE_MINVAL	12				// Max RPC Message Queue Lower Limit ~ Max Supported Client SDK Threads
#define WS_MAX_RPCQUEUE_MAXVAL	(64*1024)		// Max RPC Message Queue Upper Limit
#define WS_MAX_RPCQUEUE_NPROCSX	2				// Max RPC Message Queue Logical Processor Multiplier: i.e., Queue = max(MINVAL, NPROCSX * nprocs())

// Default Server Listener Address
#ifdef ESIF_ATTR_DEBUG
#define DEFAULT_LISTENADDR		"ws://0.0.0.0:18086"		// Default Server IP Address (All Interfaces)
#else
#define DEFAULT_LISTENADDR		"ws://127.0.0.1:18086"	// Default Server IP Address (Loopback Interface)
#endif

WebServerPtr g_WebServer = NULL;	// Global Web Server Singleton Intance

// Doorbell Opcodes
#define WS_OPCODE_NOOP			0x00	// No-Operation
#define WS_OPCODE_MESSAGE		0x01	// Message to Deliver
#define WS_OPCODE_CLOSE			0x02	// Close Orphan Connections
#define WS_OPCODE_QUIT			0xFF	// Quit Web Server

//// Message Queue Object Methods ////

// Outgoing Message Queue Node Object
typedef struct ClientMsg_s {
	esif_handle_t	ipfHandle;	// Unique IPF Client Handle
	size_t			buf_len;	// size of data buffer
	char			data[1];	// dynamic buffer of size buf_len
} ClientMsg, *ClientMsgPtr;

// Default Data Destructor
void MessageQueue_Destructor(void *object)
{
	esif_ccb_free(object);
}

// ClientMsg Destructor
void ClientMsg_Destructor(void *object)
{
	esif_ccb_free(object);
}

// ClientRequest Destructor
void ClientRequest_Destructor(void *object)
{
	if (object) {
		ClientRequestPtr self = object;
		IrpcTransaction_PutRef(self->trx);
		esif_ccb_memset(self, 0, sizeof(*self));
		esif_ccb_free(self);
	}
}

// Create a new Queue
MessageQueuePtr MessageQueue_Create(link_list_data_destroy_func destructor)
{
	MessageQueuePtr self = esif_ccb_malloc(sizeof(*self));
	if (self) {
		self->queue = esif_link_list_create();
		if (self->queue == NULL) {
			esif_ccb_free(self);
			self = NULL;
		}
		else {
			esif_ccb_lock_init(&self->lock);
			atomic_set(&self->refcount, 1);
			self->destructor = (destructor ? destructor : MessageQueue_Destructor);
		}
	}
	return self;
}

// Destroy a Queue, freeing its contents
void MessageQueue_Destroy(MessageQueuePtr self)
{
	if (self) {
		esif_ccb_write_lock(&self->lock);
		esif_link_list_free_data_and_destroy(self->queue, self->destructor);
		self->queue = NULL;
		self->destructor = NULL;
		esif_ccb_write_unlock(&self->lock);
		esif_ccb_lock_uninit(&self->lock);
		esif_ccb_free(self);
	}
}

// Take a Reference
void MessageQueue_GetRef(MessageQueuePtr self)
{
	if (self) {
		atomic_inc(&self->refcount);
	}
}

// Release a Reference
void MessageQueue_PutRef(MessageQueuePtr self)
{
	if (self) {
		if (atomic_dec(&self->refcount) == 0) {
			MessageQueue_Destroy(self);
		}
	}
}

// Add item to the Queue. object is a dynamic object now owned by queue
esif_error_t MessageQueue_EnQueue(MessageQueuePtr self, void *object)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && object) {
		esif_ccb_write_lock(&self->lock);
		rc = esif_link_list_add_at_back(self->queue, object);
		esif_ccb_write_unlock(&self->lock);
	}
	return rc;
}

// Remove the next Message from the Queue or NULL. Caller is responsible for freeing result
void *MessageQueue_DeQueue(MessageQueuePtr self)
{
	void *object = NULL;
	if (self) {
		esif_ccb_write_lock(&self->lock);
		struct esif_link_list_node *node_ptr = self->queue->head_ptr;
		if (node_ptr) {
			object = node_ptr->data_ptr;
			esif_link_list_node_remove(self->queue, node_ptr);
		}
		esif_ccb_write_unlock(&self->lock);
	}
	return object;
}

// Add a message to the Message Queue. object is a dynamic object buffer now owned by queue
// Synchronize with non-WebServer threads
esif_error_t WebServer_EnQueueMsg(WebServerPtr self, void *object)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && object) {
		esif_ccb_read_lock(&self->lock);
		MessageQueuePtr msgQueue = self->msgQueue;
		MessageQueue_GetRef(msgQueue);
		esif_ccb_read_unlock(&self->lock);
		rc = MessageQueue_EnQueue(msgQueue, object);
		MessageQueue_PutRef(msgQueue);
	}
	return rc;
}


// Remove the next IO Message from the Queue or NULL. Caller is responsible for freeing result
// Synchronize with non-WebServer threads
void *WebServer_DeQueueMsg(WebServerPtr self)
{
	void *object = NULL;
	if (self) {
		esif_ccb_read_lock(&self->lock);
		MessageQueuePtr msgQueue = self->msgQueue;
		MessageQueue_GetRef(msgQueue);
		esif_ccb_read_unlock(&self->lock);
		object = MessageQueue_DeQueue(msgQueue);
		MessageQueue_PutRef(msgQueue);
	}
	return object;
}

// Add a message to the RPC Queue. object is a dynamic object buffer now owned by queue
// Synchronize with non-WebServer threads
esif_error_t WebServer_EnQueueRpc(WebServerPtr self, WebWorkerPtr rpcWorker, void *object)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && rpcWorker && object) {
		esif_ccb_read_lock(&self->lock);
		MessageQueuePtr rpcQueue = rpcWorker->rpcQueue;
		MessageQueue_GetRef(rpcQueue);
		esif_ccb_read_unlock(&self->lock);

		// Close Connection if RPC Queue Size Limit Exceeded
		atomic_t qsize = atomic_inc(&rpcWorker->queueSize);
		if (qsize > WebServer_GetRpcQueueMax(self)) {
			atomic_set(&rpcWorker->exitFlag, 1);
			rc = ESIF_E_WS_DISC;
		}
		else {
			rc = MessageQueue_EnQueue(rpcQueue, object);
		}
		MessageQueue_PutRef(rpcQueue);
	}
	return rc;
}

// Remove the next RPC Message from the Queue or NULL. Caller is responsible for freeing result
// Synchronize with non-WebServer threads
void *WebServer_DeQueueRpc(WebServerPtr self, WebWorkerPtr rpcWorker)
{
	void *object = NULL;
	if (self && rpcWorker) {
		esif_ccb_read_lock(&self->lock);
		MessageQueuePtr rpcQueue = rpcWorker->rpcQueue;
		MessageQueue_GetRef(rpcQueue);
		esif_ccb_read_unlock(&self->lock);
		object = MessageQueue_DeQueue(rpcQueue);
		if (object) {
			atomic_dec(&rpcWorker->queueSize);
		}
		MessageQueue_PutRef(rpcQueue);
	}
	return object;
}

//// TCP Doorbell Object Methods ////

// Initialize Doorbell Object
void TcpDoorbell_Init(TcpDoorbellPtr self)
{
	if (self) {
		int j = 0;
		self->isActive = ESIF_FALSE;
		for (j = 0; j < DOORBELL_SOCKETS; j++) {
			self->sockets[j] = INVALID_SOCKET;
		}
	}
}

// Create a Doorbell object with Paired Sockets for sending signals between threads
esif_error_t TcpDoorbell_Open(TcpDoorbellPtr self)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		if (esif_ccb_socketpair(ESIF_PF_LOCAL, SOCK_STREAM, IPPROTO_IP, self->sockets) == 0) {
			self->isActive = ESIF_TRUE;
			rc = ESIF_OK;
		}
		else {
			rc = ESIF_E_WS_INIT_FAILED;
		}
	}
	return rc;
}

// Close Doorbell object
void TcpDoorbell_Close(TcpDoorbellPtr self)
{
	if (self) {
		int j = 0;
		self->isActive = ESIF_FALSE;
		for (j = 0; j < DOORBELL_SOCKETS; j++) {
			if (self->sockets[j] != INVALID_SOCKET) {
				esif_ccb_socket_close(self->sockets[j]);
				self->sockets[j] = INVALID_SOCKET;
			}
		}
	}
}

// Stop Doorbell object and signal blocking select() to exit
void TcpDoorbell_Stop(TcpDoorbellPtr self)
{
	if (self && self->sockets[DOORBELL_BUTTON] != INVALID_SOCKET) {
		esif_ccb_socket_shutdown(self->sockets[DOORBELL_BUTTON], ESIF_SHUT_RDWR);
		self->isActive = ESIF_FALSE;
	}
}

// Send a signal using Doorbell object and signal blocking select() to exit
esif_error_t TcpDoorbell_Ring(TcpDoorbellPtr self, u8 opcode)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && self->sockets[DOORBELL_BUTTON] != INVALID_SOCKET) {
		if (self->isActive && send(self->sockets[DOORBELL_BUTTON], (const char *)&opcode, sizeof(opcode), 0) == sizeof(opcode)) {
			rc = ESIF_OK;
		}
		else {
			rc = ESIF_E_WS_SOCKET_ERROR;
		}
	}
	return rc;
}

// Receive a Doorbell signal from a Listener thread
esif_error_t TcpDoorbell_Receive(TcpDoorbellPtr self, u8 *opcodePtr)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && opcodePtr && self->isActive && self->sockets[DOORBELL_RINGER] != INVALID_SOCKET) {
		ssize_t messageLength = recv(self->sockets[DOORBELL_RINGER], (char *)opcodePtr, sizeof(*opcodePtr), WS_RECV_FLAGS);
		if (messageLength == 0 || messageLength == SOCKET_ERROR || messageLength < sizeof(*opcodePtr)) {
			*opcodePtr = WS_OPCODE_NOOP;
			rc = ESIF_E_WS_SOCKET_ERROR;
		}
		else {
			rc = ESIF_OK;
		}
	}
	return rc;
}

//// Listener Object Methods ////

// Initialize Listener Object
void WebListener_Init(WebListenerPtr self)
{
	if (self) {
		esif_ccb_memset(self, 0, sizeof(*self));
		self->socket = INVALID_SOCKET;
		self->authHandle = ESIF_INVALID_HANDLE;
		self->accessControl = NULL;
	}
}

// Configure a Web Sever Listener
esif_error_t WebListener_Config(WebListenerPtr self, const AccessDef *listener)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && listener) {
		const char *serverAddr = listener->serverAddr;
		if (serverAddr == NULL || serverAddr[0] == 0) {
			serverAddr = DEFAULT_LISTENADDR;
		}
		esif_ccb_strcpy(self->serverAddr, serverAddr, sizeof(self->serverAddr));
		self->authHandle = listener->authHandle;
		self->accessControl = listener->accessControl;

		// Disable Pipe if App Disabled in DCFG
		if (listener->dcfgOpt.asU32 & IpfSrv_GetDcfg().asU32) {
			self->accessControl = IPFAUTH_ACL_NOBODY;
		}
		// Disable Pipe if Required App is not Installed
		else if (!IpfSrv_IsAppInstalled(listener->appKey)) {
			self->accessControl = IPFAUTH_ACL_NOBODY;
		}
		rc = ESIF_OK;
	}
	return rc;
}

/*
** Linux-Specific Security ACL Functions
** Linux Security is based on setting desired owner/group and necessary read/write permissions
** on a managed object (Unix Domain Socket file)
** ACL Format: "permissions[:owner][:group]" where permissions=<octal|'rwxrwxrwx'>
*/
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#define ERROR_SUCCESS	0

esif_error_t enable_chown_capability();
esif_error_t disable_chown_capability();

// Set Security ACLs for Folder containing Unix Domain Socket files
static int WebListener_SetSocketFolderSecurity(WebListenerPtr self)
{
	int rc = EINVAL;
	if (self && self->sockaddr.type == AF_UNIX) {

		rc = enable_chown_capability();
		if (rc != ERROR_SUCCESS) {
			WS_TRACE_WARNING("Error Enabling Capability\n");
		}

		// Change folder permissions and owner/group to: rwxr-xr-x root root
		rc = chmod(IPC_NAMEDPIPE_PATH, 0755);
		if (rc == ERROR_SUCCESS) {
			rc = chown(IPC_NAMEDPIPE_PATH, 0, 0);
		}

		rc = disable_chown_capability();
		if (rc != ERROR_SUCCESS) {
			WS_TRACE_WARNING("Error Disabling Capability\n");
		}
	}
	esif_ccb_socket_seterror(rc);
	return rc;
}

static int WebListener_SetSocketSecurity(WebListenerPtr self)
{
	int rc = EINVAL;
	if (self && self->sockaddr.type == AF_UNIX && self->sockaddr.un_addr.sun_path[0] && self->accessControl) {
		const char *accessControl = self->accessControl;
		// Do not allow any user access if Pipe is Disabled
		if (esif_ccb_stricmp(accessControl, IPFAUTH_ACL_NOBODY) == 0) {
			accessControl = "r--------:root:root";
		}
		char *acl = esif_ccb_strdup(accessControl);
		if (acl == NULL) {
			return ENOMEM;
		}
		rc = EACCES;

		// Set permissions to:  rw-rw---- root root	by default
		mode_t mode = 0660;
		uid_t owner = 0;
		gid_t group = 0;

		// Extract and convert optional Permissions, Owner, and Group from ACL
		char *ctx = NULL;
		char *mode_name = esif_ccb_strtok(acl, ":", &ctx);
		char *owner_name = esif_ccb_strtok(NULL, ":", &ctx);
		char *group_name = esif_ccb_strtok(NULL, ":", &ctx);

		if (owner_name) {
			struct passwd *pwd = getpwnam(owner_name);
			if (pwd) {
				owner = pwd->pw_uid;
			}
		}
		if (group_name) {
			struct group *grp = getgrnam(group_name);
			if (grp) {
				group = grp->gr_gid;
			}
		}
		if (mode_name) {
			mode_t mod = 0;
			// Octal mode, i.e. 660 or 0660
			if (isdigit(*mode_name)) {
				while (*mode_name >= '0' && *mode_name <= '7') {
					mod = (mod << 3) | (*mode_name++ - '0');
				}
			}
			// Bitmask mode, i.e. "rw-rw----"
			else {
				for (int j = 0; j < 3 && esif_ccb_strlen(mode_name, 3) == 3; j++, mode_name += 3) {
					mod = (mod << 3);
					if (mode_name[0] == 'r') {
						mod |= 0x4;
					}
					if (mode_name[1] == 'w') {
						mod |= 0x2;
					}
					if (mode_name[2] == 'x') {
						mod |= 0x1;
					}
				}
			}
			mode = mod;
		}

		rc = enable_chown_capability();
		if (rc != ERROR_SUCCESS) {
			WS_TRACE_WARNING("Error Enabling Capability\n");
		}

		// Set File Owner/Group and Access Mode
		rc = chmod(self->sockaddr.un_addr.sun_path, mode);
		if (rc == 0) {
			rc = chown(self->sockaddr.un_addr.sun_path, owner, group);
		}

		rc = disable_chown_capability();
		if (rc != ERROR_SUCCESS) {
			WS_TRACE_WARNING("Error Enabling Capability\n");
		}

		esif_ccb_free(acl);
	}
	esif_ccb_socket_seterror(rc);
	return rc;
}


// Close a Listener Object
void WebListener_Close(WebListenerPtr self)
{
	// Close connection and delete Unix Domain Socket file
	if (self->socket != INVALID_SOCKET) {
		esif_ccb_socket_close(self->socket);
		self->socket = INVALID_SOCKET;
		self->authHandle = ESIF_INVALID_HANDLE;
		self->accessControl = NULL;
		if (self->sockaddr.type == AF_UNIX) {
			esif_ccb_unlink(self->sockaddr.un_addr.sun_path);
		}
	}
	// Delete the Socket File folder after last file deleted
	if (self->sockaddr.type == AF_UNIX) {
		(void)esif_ccb_rmdir(IPC_NAMEDPIPE_PATH);
		esif_ccb_memset(&self->sockaddr, 0, sizeof(self->sockaddr));
	}
}

// Open a Web Listener Object and Listen on the configured Server Address
esif_error_t WebListener_Open(WebListenerPtr self)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		int sockopt = 1; // Exclusive port use (Fail if already in use by another listener) [AF_INET only]

		if (self->serverAddr[0] == 0) {
			return ESIF_OK;
		}

		// Parse Server Address 
		rc = IpfIpc_ParseServerAddress(self->serverAddr, &self->sockaddr);
		if (rc != ESIF_OK) {
			WS_TRACE_DEBUG("Invalid Server Address: %s\n", self->serverAddr);
			return rc;
		}

		// Restart Listener if Socket File Deleted
		if (self->sockaddr.type == AF_UNIX && self->socket != INVALID_SOCKET && esif_socket_file_stat(self->sockaddr.un_addr.sun_path) == ESIF_SOCKERR_ENOENT) {
			esif_ccb_socket_close(self->socket);
			self->socket = INVALID_SOCKET;
		}
		else if (self->socket != INVALID_SOCKET) {
			return ESIF_E_WS_ALREADY_STARTED;
		}

		// Always Delete Unix Domain Socket File since Server owns it
		if (self->sockaddr.type == AF_UNIX) {
			esif_ccb_unlink(self->sockaddr.un_addr.sun_path);
		}

		// Create Listener socket on specified IP/Port, Failing if port is already in use
		if (((self->socket = socket(self->sockaddr.type, SOCK_STREAM, IPPROTO_IP)) != INVALID_SOCKET) &&
			(self->sockaddr.type != AF_INET || setsockopt(self->socket, SOL_SOCKET, ESIF_SO_EXCLUSIVEADDRUSE, (const char *)&sockopt, sizeof(sockopt)) != SOCKET_ERROR) &&
			(bind(self->socket, (struct sockaddr *)&self->sockaddr, IpfIpc_SockaddrLen(self->sockaddr.type)) != SOCKET_ERROR) &&
			(self->sockaddr.type != AF_UNIX || WebListener_SetSocketSecurity(self) == ERROR_SUCCESS) &&
			(listen(self->socket, IPF_WS_MAX_CLIENTS) != SOCKET_ERROR)
			) {
			rc = ESIF_OK;
		}
		else {
			WS_TRACE_ERROR("Cannot Listen on %s (Error %d)\n", self->serverAddr, esif_ccb_socket_error());
			WebListener_Close(self);
			rc = ESIF_E_WS_INIT_FAILED;
		}
	}
	return rc;
}

// Accept a Client Connection from a Listener Socket into a Socket Handle
esif_error_t WebListener_AcceptClient(WebListenerPtr self, esif_ccb_socket_t *socketPtr, esif_ccb_sockaddr_t *sockaddrPtr)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && socketPtr) {
		esif_ccb_sockaddr_t client = { 0 };
		socklen_t clientLen = IpfIpc_SockaddrLen(self->sockaddr.type);

		*socketPtr = accept(self->socket, (struct sockaddr *)&client, &clientLen);

		if (*socketPtr == INVALID_SOCKET) {
			rc = ESIF_E_WS_SOCKET_ERROR;
		}
		else {
			if (self->sockaddr.type == AF_UNIX) {
				esif_ccb_strcpy(client.un_addr.sun_path, self->sockaddr.un_addr.sun_path, sizeof(client.un_addr.sun_path));
			}
			*sockaddrPtr = client;
			rc = ESIF_OK;
		}
	}
	return rc;
}

//// WebWorker Object Methods ////

// Create an RPC Worker Object
WebWorkerPtr WebWorker_Create()
{
	WebWorkerPtr self = esif_ccb_malloc(sizeof(*self));
	if (self) {
		self->rpcQueue = MessageQueue_Create(ClientRequest_Destructor);
		if (self->rpcQueue == NULL) {
			esif_ccb_free(self);
			self = NULL;
		}
		else {
			atomic_set(&self->exitFlag, 0);
			atomic_set(&self->refCount, 0);
			self->rpcThread = ESIF_THREAD_NULL;
			signal_init(&self->rpcSignal);
			WebWorker_GetRef(self);
		}
	}
	return self;
}

// Destroy an RPC Worker Object
void WebWorker_Destroy(WebWorkerPtr self)
{
	if (self) {
		MessageQueuePtr rpcQueue = self->rpcQueue;
		self->rpcQueue = NULL;
		MessageQueue_PutRef(rpcQueue);
		WebWorker_Stop(self);
		signal_uninit(&self->rpcSignal);
		esif_ccb_free(self);
	}
}

// Take a Reference
void WebWorker_GetRef(WebWorkerPtr self)
{
	if (self) {
		atomic_inc(&self->refCount);
	}
}

// Release a Reference
void WebWorker_PutRef(WebWorkerPtr self)
{
	if (self) {
		if (atomic_dec(&self->refCount) == 0) {
			WebWorker_Destroy(self);
		}
	}
}

// Start RPC Thread
esif_error_t WebWorker_Start(WebWorkerPtr self)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		if (self->rpcThread != ESIF_THREAD_NULL) {
			rc = ESIF_E_WS_ALREADY_STARTED;
		}
		else {
			WebWorker_GetRef(self);
			rc = esif_ccb_thread_create(&self->rpcThread, Irpc_WorkerThread, self);
			if (rc != ESIF_OK) {
				WebWorker_PutRef(self);
			}
		}
	}
	return rc;
}

// Stop RPC Thread
void WebWorker_Stop(WebWorkerPtr self)
{
	if (self) {
		if (self->rpcThread != ESIF_THREAD_NULL) {
			atomic_set(&self->exitFlag, 1);
			signal_post(&self->rpcSignal);
			esif_ccb_thread_join(&self->rpcThread);
			self->rpcThread = ESIF_THREAD_NULL;
		}
	}
}

//// WebClient Object Methods ////

// Initialize a WebClient Object
void WebClient_Init(WebClientPtr self)
{
	if (self) {
		esif_ccb_memset(self, 0, sizeof(*self));
		self->type = ClientClosed;
		self->socket = INVALID_SOCKET;
		self->authHandle = ESIF_INVALID_HANDLE;
		self->msgType = FRAME_NULL;
		self->ipfHandle = ESIF_INVALID_HANDLE;
	}
}

// Close Web Client
void WebClient_Close(WebClientPtr self)
{
	if (self) {
		esif_handle_t ipfHandle = self->ipfHandle;
		esif_ccb_free(self->sendBuf);
		esif_ccb_free(self->recvBuf);
		esif_ccb_free(self->httpBuf);
		esif_ccb_free(self->httpRequest);
		esif_ccb_free(self->fragBuf);
		if (self->socket != INVALID_SOCKET) {
			esif_ccb_socket_close(self->socket);
		}
		WebWorkerPtr rpcWorker = self->rpcWorker;
		self->rpcWorker = NULL;
		WebWorker_Stop(rpcWorker);
		WebWorker_PutRef(rpcWorker);
		WebClient_Init(self);
		IpfClient_Stop(ipfHandle);
	}
}

// Write a Buffer to a WebClient
esif_error_t WebClient_Write(WebClientPtr self, void *buffer, size_t buf_len)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (self && self->socket != INVALID_SOCKET) {
		int send_flags = (self->type == ClientWebsocket ? WS_NONBLOCKING_FLAGS : WS_SEND_FLAGS);
		ssize_t ret = 0;
		rc = ESIF_OK;

		// Debug HTTP Response
		if (buffer && esif_ccb_strncmp(buffer, "HTTP/", 5) == 0) {
			WS_TRACE_DEBUG("%.*s", (int)buf_len, (char *)buffer);
		}

		// Do Non-Blocking Send of any data already in send buffer first
		if (self->sendBuf != NULL && self->sendBufLen > 0) {
			ret = send(self->socket, (const char *)self->sendBuf, (int)self->sendBufLen, WS_NONBLOCKING_FLAGS);

			// Destroy send buffer if Complete send, otherwise remove sent data before appending new data
			if (ret == (int)self->sendBufLen) {
				esif_ccb_free(self->sendBuf);
				self->sendBuf = NULL;
				self->sendBufLen = 0;
				ret = 0;
			}
			else if (ret > 0) {
				esif_ccb_memmove(self->sendBuf, self->sendBuf + ret, self->sendBufLen - ret);
				self->sendBufLen -= ret;
				ret = 0;
			}
		}

		// Do Blocking or Non-Blocking send if send buffer is clear
		if (self->sendBuf == NULL && buffer != NULL && buf_len > 0) {
			ret = send(self->socket, (char*)buffer, (int)buf_len, send_flags);
		}

		// Close Socket on Failure; EWOULDBLOCK is not a failure it is expected if the operation would block
		if (ret == SOCKET_ERROR) {
			int errnum = 0;
			if ((errnum = esif_ccb_socket_error()) == ESIF_SOCKERR_EWOULDBLOCK) {
				ret = 0;
			}
			else {
				rc = ESIF_E_WS_SOCKET_ERROR;
			}
		}

		// Append any unsent data to send buffer
		if (rc == ESIF_OK && buffer != NULL && ret != (ssize_t)buf_len) {
			size_t newBufLen = self->sendBufLen + buf_len - ret;
			u8 *newBuffer = NULL;
			if (newBufLen <= WS_MAX_CLIENT_SENDBUF) {
				newBuffer = esif_ccb_realloc(self->sendBuf, newBufLen);
			}
			if (newBuffer == NULL) {
				rc = ESIF_E_NO_MEMORY;
			}
			else {
				esif_ccb_memmove(newBuffer + self->sendBufLen, (u8*)buffer + ret, buf_len - ret);
				self->sendBuf = newBuffer;
				self->sendBufLen = newBufLen;
			}
			WS_TRACE_DEBUG("WS SEND Buffering (%d): buffer=%zd sent=%d error=%d send_buf=%zd\n", (int)self->socket, buf_len, ret, esif_ccb_socket_error(), self->sendBufLen);
		}

		// Close Socket on Error
		if (rc != ESIF_OK) {
			WS_TRACE_DEBUG("WS SEND Failure (%d): buffer=%zd sent=%d error=%d send_buf=%zd [%zd total]\n", (int)self->socket, buf_len, ret, esif_ccb_socket_error(), self->sendBufLen, self->sendBufLen + buf_len);
			WebClient_Close(self);
		}
	}
	return rc;
}

//// WebServer Object Methods ////

// Initialize WebServer Object
void WebServer_Init(WebServerPtr self)
{
	int j = 0;
	if (self) {
		esif_ccb_lock_init(&self->lock);
		TcpDoorbell_Init(&self->doorbell);
		for (j = 0; j < IPF_WS_MAX_LISTENERS; j++) {
			WebListener_Init(&self->listeners[j]);
		}
		for (j = 0; j < IPF_WS_MAX_CLIENTS; j++) {
			WebClient_Init(&self->clients[j]);
		}
		self->fileDoorbell = NULL;
		atomic_set(&self->isActive, 0);
		atomic_set(&self->isPaused, 0);
		atomic_set(&self->isDiagnostic, 0);
		atomic_set(&self->activeThreads, 0);
		atomic_set(&self->listenerMask, 0);
		self->netBuf = NULL;
		self->netBufLen = 0;
		self->msgQueue = MessageQueue_Create(ClientMsg_Destructor);
		atomic_t nprocs = esif_ccb_nprocs();
		atomic_set(&self->rpcQueueMax, esif_ccb_max(esif_ccb_min(nprocs * WS_MAX_RPCQUEUE_NPROCSX, WS_MAX_RPCQUEUE_MAXVAL), WS_MAX_RPCQUEUE_MINVAL));
		IpfSrv_Init();
	}
}

// Get Active Listener Bitmask
atomic_t WebServer_GetListenerMask(WebServerPtr self)
{
	atomic_t mask = ATOMIC_INIT(0);
	if (self) {
		mask = atomic_read(&self->listenerMask);
	}
	return mask;
}

// Set Active Listener Bitmask for specified Bits in mask
void WebServer_SetListenerMask(WebServerPtr self, atomic_t mask)
{
	if (self) {
		atomic_set(&self->listenerMask, atomic_read(&self->listenerMask) | atomic_read(&mask));
	}
}

// Clear Active Listener Bitmask for specified Bits in mask
void WebServer_ClearListenerMask(WebServerPtr self, atomic_t mask)
{
	if (self) {
		atomic_set(&self->listenerMask, atomic_read(&self->listenerMask) & ~atomic_read(&mask));
	}
}

// Get Active Pipes Bitmask
atomic_t WebServer_GetPipeMask(WebServerPtr self)
{
	atomic_t mask = ATOMIC_INIT(0);
	if (self) {
		esif_ccb_read_lock(&self->lock);
		mask = atomic_read(&self->listenerMask);
		atomic_t bit = ATOMIC_INIT(0x1);
		for (size_t j = 0; j < IPF_WS_MAX_LISTENERS; j++) {
			if ((mask & bit) && (self->listeners[j].accessControl == NULL || esif_ccb_stricmp(self->listeners[j].accessControl, IPFAUTH_ACL_NOBODY) == 0)) {
				mask &= ~bit;
			}
			bit <<= 1;
		}
		esif_ccb_read_unlock(&self->lock);
	}
	return mask;
}

// Get RPC Queue Limit
atomic_t WebServer_GetRpcQueueMax(WebServerPtr self)
{
	return (self ? atomic_read(&self->rpcQueueMax): 0);
}

// Set RPC Queue Limit
void WebServer_SetRpcQueueMax(WebServerPtr self, size_t maxQueue)
{
	if (self) {
		atomic_set(&self->rpcQueueMax, esif_ccb_min(maxQueue, WS_MAX_RPCQUEUE_MAXVAL));
	}
}

// Close Web Server Objects
void WebServer_Close(WebServerPtr self)
{
	if (self) {
		int j = 0;
		TcpDoorbell_Close(&self->doorbell);
		for (j = 0; j < IPF_WS_MAX_LISTENERS; j++) {
			WebListener_Close(&self->listeners[j]);
			WebServer_ClearListenerMask(self, ((size_t)1 << j));
		}
		for (j = 0; j < IPF_WS_MAX_CLIENTS; j++) {
			WebClient_Close(&self->clients[j]);
		}
		esif_ccb_free(self->netBuf);
		self->netBuf = NULL;
		self->netBufLen = 0;
		atomic_set(&self->isActive, 0);
		atomic_set(&self->isPaused, 0);
		atomic_set(&self->isDiagnostic, 0);
		MessageQueuePtr msgQueue = self->msgQueue;
		self->msgQueue = NULL;
		self->rpcQueueMax = 0;
		MessageQueue_PutRef(msgQueue);
	}
}

// Unitialize Web Server
void WebServer_Exit(WebServerPtr self)
{
	if (self) {
		WebServer_Stop(self);
		WebServer_Close(self);
		IpfSrv_Uninit();
		esif_ccb_lock_uninit(&self->lock);
	}
}

// Configure Web Server
esif_error_t WebServer_Config(WebServerPtr self, u8 instance, const AccessDef *listener)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && listener && instance < IPF_WS_MAX_LISTENERS) {
		rc = WebListener_Config(&self->listeners[instance], listener);
	}
	return rc;
}

// Deliver a Message to Client by adding it to Message Queue
// Synchronize with non-WebServer threads
esif_error_t WebServer_SendMsg(WebServerPtr self, const esif_handle_t ipfHandle, const void *buffer, size_t buf_len)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	// Queue Outgoing Message
	if (self && buffer && buf_len > 0) {
		size_t msg_len = sizeof(ClientMsg) + buf_len;
		ClientMsgPtr msg_ptr = esif_ccb_malloc(msg_len);
		if (msg_ptr == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}
		else {
			msg_ptr->ipfHandle = ipfHandle;
			msg_ptr->buf_len = buf_len;
			esif_ccb_memcpy((void *)&msg_ptr->data, buffer, buf_len);

			rc = WebServer_EnQueueMsg(self, msg_ptr);

			if (rc != ESIF_OK) {
				esif_ccb_free(msg_ptr);
			}
			// Signal Web Server that there is work to do by ringing the Doorbell
			TcpDoorbell_Ring(&self->doorbell, WS_OPCODE_MESSAGE);
		}
	}
	return rc;
}

// Receive and Process a Client Request, Buffering message if necessary
esif_error_t WebServer_ProcessRequest(WebServerPtr self, WebClientPtr client)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (self && client && self->netBuf && self->netBufLen > 0) {
		u8 *buffer = self->netBuf;
		size_t buf_len = self->netBufLen;
		ssize_t messageLength = 0;
		u8 *messageBuffer = NULL;

		esif_ccb_memset(self->netBuf, 0, self->netBufLen);
		rc = ESIF_OK;

		// Read the next partial or complete message fragment from the client socket.
		messageLength = recv(client->socket, (char*)buffer, (int)buf_len, WS_RECV_FLAGS);
		if (messageLength == 0 || messageLength == SOCKET_ERROR) {
			int errnum = 0;
			if ((errnum = esif_ccb_socket_error()) != ESIF_SOCKERR_EWOULDBLOCK) {
				rc = ESIF_E_WS_DISC;
			}
		}
		if (rc == ESIF_OK) {
			WS_TRACE_DEBUG("Socket[%d] Received %d bytes\n", (int)client->socket, (int)messageLength);

			// Combine this partial frame with the current connection's RECV Buffer, if any
			if (client->recvBuf != NULL && client->recvBufLen > 0) {
				size_t total_buffer_len = client->recvBufLen + messageLength;
				if (total_buffer_len <= WS_MAX_CLIENT_RECVBUF) {
					messageBuffer = esif_ccb_realloc(client->recvBuf, total_buffer_len);
				}
				if (messageBuffer == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				else {
					WS_TRACE_DEBUG("WS Frame Unbuffering: buflen=%zd msglen=%zd total=%zd net=%zd\n", client->recvBufLen, messageLength, total_buffer_len, self->netBufLen);
					esif_ccb_memcpy(messageBuffer + client->recvBufLen, self->netBuf, messageLength);
					buffer = messageBuffer;
					buf_len = total_buffer_len;
					messageLength = total_buffer_len;
					client->recvBuf = NULL;
					client->recvBufLen = 0;
				}
			}

			// Process Request for Connection current Protocol Type
			if (rc == ESIF_OK) {
				switch (client->type) {
				case ClientHttp:
					rc = WebServer_HttpRequest(self, client, buffer, messageLength);
					break;
				case ClientWebsocket:
					rc = WebServer_WebsocketRequest(self, client, buffer, messageLength);
					break;
				default:
					rc = ESIF_E_WS_DISC;
					break;
				}
			}

			// Process Incomplete Requests after more data is received
			if (rc == ESIF_E_WS_INCOMPLETE) {
				rc = ESIF_OK;
			}
		}
		esif_ccb_free(messageBuffer);
	}
	return rc;
}

// Close Orphaned Connections
void WebServer_CloseOrphans(WebServerPtr self)
{
	if (self) {
		TcpDoorbell_Ring(&self->doorbell, WS_OPCODE_CLOSE);
	}
}

// Web Server Main Module (One per Thread)
static esif_error_t WebServer_Main(WebServerPtr self)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		struct timeval tv = { 0 };	// Timeout
		fd_set readFDs = { 0 };		// Readable Sockets List
		fd_set writeFDs = { 0 };	// Writable Sockets List
		fd_set exceptFDs = { 0 };	// Exception Sockets List

		int selectResult = 0;		// select() result
		int maxfd = 0;				// Max file descriptor ID + 1
		int setsize = 0;			// Number of items in FD List
		int j = 0;

		atomic_inc(&self->activeThreads);
		rc = ESIF_OK;
	
		// Allocate Network Buffer
		self->netBufLen = WS_NETWORK_BUFFER_LEN;
		self->netBuf = esif_ccb_malloc(self->netBufLen);
		if (self->netBuf == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}

		// Create Doorbell Socket Pair
		if (rc == ESIF_OK) {
			rc = TcpDoorbell_Open(&self->doorbell);
		}

		// Create Listener Socket(s)
		if (ESIF_OK == rc) {
			int running = 0;
			esif_error_t last_error = ESIF_OK;
			for (j = 0; j < IPF_WS_MAX_LISTENERS; j++) {
				if (self->listeners[j].serverAddr[0]) {
					rc = WebListener_Open(&self->listeners[j]);
				
					if (rc == ESIF_OK) {
						running++;
						WebServer_SetListenerMask(self, ((size_t)1 << j));
						WS_TRACE_INFO("\nIPF Server Listening [%s]\n", self->listeners[j].serverAddr);
					}
					else {
						last_error = rc;
						WS_TRACE_ERROR("\nUnable to start IPF Server Listener [%s]\n", self->listeners[j].serverAddr);
					}
				}
			}
			if (running) {
				rc = ESIF_OK;
			}
			else {
				rc = last_error;
			}
		}

		//// MAIN LOOP ////
		
		// Process all active sockets and accept new connections until Quit signaled
		while (rc == ESIF_OK && atomic_read(&self->isActive)) {

			// Reset File Descriptor Lists after each iteration
			FD_ZERO(&readFDs);
			FD_ZERO(&writeFDs);
			FD_ZERO(&exceptFDs);
			maxfd = 0;
			setsize = 0;

			// Add Doorbell Ringer
			FD_SET(self->doorbell.sockets[DOORBELL_RINGER], &readFDs);
			FD_SET(self->doorbell.sockets[DOORBELL_RINGER], &exceptFDs);
			maxfd = (int)self->doorbell.sockets[DOORBELL_RINGER] + 1;
			setsize++;

			// Add Listener Socket(s)
			for (j = 0; j < IPF_WS_MAX_LISTENERS && setsize < WS_MAX_SOCKETS; j++) {

				// Recreate Unix Domain Socket Listener if file was deleted
				if ((self->listeners[j].sockaddr.type == AF_UNIX) && 
					(WebServer_GetListenerMask(self) & ((size_t)1 << j)) &&
					(esif_socket_file_stat(self->listeners[j].sockaddr.un_addr.sun_path) == ESIF_SOCKERR_ENOENT)) {

					if (self->listeners[j].socket != INVALID_SOCKET) {
						esif_ccb_socket_close(self->listeners[j].socket);
						self->listeners[j].socket = INVALID_SOCKET;
					}
					WebServer_ClearListenerMask(self, ((size_t)1 << j));

					rc = WebListener_Open(&self->listeners[j]);

					if (rc == ESIF_OK) {
						WebServer_SetListenerMask(self, ((size_t)1 << j));
						WS_TRACE_INFO("Recreated Listener %s (%s)\n", self->listeners[j].serverAddr, esif_rc_str(rc));
					}
					else {
						WS_TRACE_ERROR("Error Recreating Listener %s (%s)\n", self->listeners[j].serverAddr, esif_rc_str(rc));
						break;
					}
				}
				if (self->listeners[j].socket != INVALID_SOCKET) {
					FD_SET(self->listeners[j].socket, &readFDs);
					FD_SET(self->listeners[j].socket, &exceptFDs);
					maxfd = esif_ccb_max(maxfd, (int)self->listeners[j].socket + 1);
					setsize++;
				}
			}

			// Add Client Socket(s)
			for (j = 0; j < IPF_WS_MAX_CLIENTS && setsize < WS_MAX_SOCKETS; j++) {
				if (self->clients[j].socket != INVALID_SOCKET) {
					FD_SET(self->clients[j].socket, &readFDs);
					FD_SET(self->clients[j].socket, &exceptFDs);
					maxfd = esif_ccb_max(maxfd, (int)self->clients[j].socket + 1);
					setsize++;

					// Wait for socket to become writable if pending send buffer
					if (self->clients[j].sendBuf) {
						FD_SET(self->clients[j].socket, &writeFDs);
					}
				}
			}

			// Use Lowest Remaining Transaction Timeout for all Active Connections, if any
			double timeout = IpfTrxMgr_GetMinTimeout(IpfTrxMgr_GetInstance());
			double sessionTimeout = AppSessionMgr_GetMinTimeout();
			timeout = esif_ccb_min(timeout, sessionTimeout);
			tv.tv_sec = (long)timeout;
			tv.tv_usec = (long)((timeout - (Int64)timeout) * 1000000);
			if (tv.tv_sec == 0 && tv.tv_usec == 0) {
				tv.tv_sec = WS_SOCKET_TIMEOUT;
			}

			//// WAIT FOR SOCKET ACTIVITY ////

			selectResult = select(maxfd, &readFDs, &writeFDs, &exceptFDs, &tv);

			// Exit loop if select error or server stopping; continue loop if inactivity timeout
			if (selectResult == SOCKET_ERROR) {
				WS_TRACE_ERROR("SELECT Error (%d)\n", selectResult);
				rc = ESIF_E_WS_SOCKET_ERROR;
				break;
			}
			else if (!atomic_read(&self->isActive)) { // Exit if Server not Active
				break;
			}
			else if (selectResult == 0) { // Timeout
				IpfTrxMgr_ExpireInactive(IpfTrxMgr_GetInstance());
				AppSessionMgr_SessionCleanup();
				continue;
			}

			//// PROCESS ALL ACTIVE SOCKETS ////

			// 1. Respond to Incoming Doorbell Socket signals
			if (FD_ISSET(self->doorbell.sockets[DOORBELL_RINGER], &readFDs)) {
				u8 opcode = WS_OPCODE_NOOP;

				// Exit if Doorbell socket(s) shutdown or a QUIT opcode received
				if (TcpDoorbell_Receive(&self->doorbell, &opcode) != ESIF_OK) {
					WS_TRACE_DEBUG("Doorbell Closed; Exiting");
					break;
				}
				WS_TRACE_DEBUG("Doorbell Received: 0x%02X", ((int)opcode & 0xFF));
				if (opcode == WS_OPCODE_QUIT) {
					break;
				}
				else if (opcode == WS_OPCODE_CLOSE) {
					for (j = 0; j < IPF_WS_MAX_CLIENTS && atomic_read(&self->isActive); j++) {
						WebClientPtr client = &self->clients[j];
						if (client->socket != INVALID_SOCKET && client->ipfHandle != ESIF_INVALID_HANDLE && !IpfClient_Exists(client->ipfHandle)) {
							WebClient_Close(client);
							WS_TRACE_DEBUG("Closing Client[%d]: (Stopped) Socket[%d]\n", j, (int)client->socket);
						}
					}
				}
			}

			// 2. Close open Sockets with Exceptions
			for (j = 0; j < IPF_WS_MAX_CLIENTS && atomic_read(&self->isActive); j++) {
				WebClientPtr client = &self->clients[j];
				if (client->socket != INVALID_SOCKET && FD_ISSET(client->socket, &exceptFDs)) {
					WebClient_Close(client);
					WS_TRACE_DEBUG("Closing Client[%d]: (Exception) Socket[%d]\n", j, (int)client->socket);
				}
			}

			// 3. Accept any new connections on the Listener Socket(s)
			int sockets = setsize;
			for (j = 0; j < IPF_WS_MAX_LISTENERS && atomic_read(&self->isActive); j++) {
				WebListenerPtr listener = &self->listeners[j];
				if (listener->socket != INVALID_SOCKET && FD_ISSET(listener->socket, &readFDs)) {
					esif_ccb_socket_t clientSocket = INVALID_SOCKET;
					WebClientPtr client = NULL;
					int k = 0;

					// Find first Unused Client
					for (k = 0; k < IPF_WS_MAX_CLIENTS && sockets < WS_MAX_SOCKETS; k++) {
						if (self->clients[k].type == ClientClosed) {
							client = &self->clients[k];
							break;
						}
					}

					// Accept Incoming Connection
					esif_ccb_sockaddr_t clientSockaddr = { 0 };
					if ((rc = WebListener_AcceptClient(listener, &clientSocket, &clientSockaddr)) != ESIF_OK) {
						WS_TRACE_DEBUG("Listener[%d] Socket[%d]: Accept Error: %s (%d)\n", j, listener->socket, esif_rc_str(rc), rc);
					}
					else {
						// Close Client if Max Connections exceeded or Server Paused
						if (client == NULL || atomic_read(&self->isPaused)) {
							if (client == NULL) {
								WS_TRACE_WARNING("Connection Limit Exceeded (%d)\n", IPF_WS_MAX_CLIENTS);
							}
							else {
								WS_TRACE_INFO("Server Paused: Connection Closed[%d]", clientSocket);
							}
							esif_ccb_socket_close(clientSocket);
							clientSocket = INVALID_SOCKET;
							continue;
						}

						// Client is now an HTTP Connection
						client->type = ClientHttp;
						client->socket = clientSocket;
						client->sockaddr = clientSockaddr;
						client->authHandle = listener->authHandle;
						clientSocket = INVALID_SOCKET;
						sockets++;	
						WS_TRACE_DEBUG("Accepted Client[%d]: Socket[%d]\n", k, (int)client->socket);
					}
				}
			}

			// 4. Process Active Client Requests
			for (j = 0; j < IPF_WS_MAX_CLIENTS && atomic_read(&self->isActive); j++) {
				WebClientPtr client = &self->clients[j];

				// Receive and Process Requests from Readable Clients
				esif_handle_t ipfHandle = client->ipfHandle;
				if (client->socket != INVALID_SOCKET && FD_ISSET(client->socket, &readFDs)) {
					if ((rc = WebServer_ProcessRequest(self, client)) != ESIF_OK) {
						WebClient_Close(client);
						WS_TRACE_DEBUG("Client[%d] Disconnected: %s (%d)\n", j, esif_rc_str(rc), (int)rc);
					}
				}

				// Start New IPF Client Session for each new Connection
				if (client->socket != INVALID_SOCKET && ipfHandle == ESIF_INVALID_HANDLE && client->ipfHandle != ESIF_INVALID_HANDLE) {
					IpfClient_Start(client->ipfHandle, client->sockaddr, client->authHandle);
				}

				// Flush pending Send Buffer when socket becomes writable
				if (client->socket != INVALID_SOCKET && FD_ISSET(client->socket, &writeFDs)) {
					size_t sendBufLen = client->sendBufLen;
					UNREFERENCED_PARAMETER(sendBufLen);
					rc = WebClient_Write(client, NULL, 0);
					WS_TRACE_DEBUG("WS SEND Unbuffering (%d): before=%zd after=%zd\n", (int)client->socket, sendBufLen, client->sendBufLen);
				}
			}

			// 5. Deliver Pending Messages to Active IPF Clients
			ClientMsgPtr msg_ptr = NULL;
			while (atomic_read(&self->isActive) && (msg_ptr = WebServer_DeQueueMsg(self)) != NULL) {
				rc = ESIF_E_INVALID_HANDLE;
				for (j = 0; j < IPF_WS_MAX_CLIENTS && atomic_read(&self->isActive); j++) {
					WebClientPtr client = &self->clients[j];
					if (client->type == ClientWebsocket && client->socket != INVALID_SOCKET && client->ipfHandle != ESIF_INVALID_HANDLE && client->ipfHandle == msg_ptr->ipfHandle) {
						rc = WebServer_WebsocketDeliver(self, client, (u8 *)msg_ptr->data, msg_ptr->buf_len);
						break;
					}
				}
				esif_ccb_free(msg_ptr);
			}

			// 6. Destroy Expired Transactions and Dead Sessions
			IpfTrxMgr_ExpireInactive(IpfTrxMgr_GetInstance());
			AppSessionMgr_SessionCleanup();
			rc = ESIF_OK;
		}

		// Cleanup
		WebServer_Close(self);
		atomic_dec(&self->activeThreads);
	}
	return rc;
}

// Web Server Main Worker Thread Wrapper
static void *ESIF_CALLCONV WebServer_WorkerThread(void *ctx)
{
	WebServerPtr self = (WebServerPtr)ctx;
	WS_TRACE_INFO("Web Server Worker Thread Starting");
	WebServer_Main(self);
	WS_TRACE_INFO("Web Server Worker Thread Exiting");
	return 0;
}

// Worker Thread to detect changes to the folder containing Unix Domain Socket files
static void *ESIF_CALLCONV WebServer_FileChangeThread(void *ctx)
{
	UNREFERENCED_PARAMETER(ctx);
	return NULL;
}

static esif_error_t WebServer_IrpcMain(WebServerPtr self, WebWorkerPtr rpcWorker)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && rpcWorker) {
		atomic_inc(&self->activeThreads);

		while (!atomic_read(&rpcWorker->exitFlag)) {
			signal_wait(&rpcWorker->rpcSignal);

			ClientRequestPtr object = WebServer_DeQueueRpc(self, rpcWorker);

			if (object && object->trx) {
				IrpcTransaction *trx = object->trx;

				rc = IrpcTransaction_EsifRequest(trx);

				if (rc == ESIF_OK) {
					EsifMsgHdr header = {
						.v1.signature = ESIFMSG_SIGNATURE,
						.v1.headersize = sizeof(header),
						.v1.version = ESIFMSG_VERSION,
						.v1.msgclass = ESIFMSG_CLASS_IRPC
					};
					header.v1.msglen = (UInt32)IBinary_GetLen(trx->response);

					if (IBinary_Insert(trx->response, &header, sizeof(header), 0) != NULL) {
						rc = WebServer_SendMsg(
							self,
							object->ipfHandle,
							IBinary_GetBuf(trx->response),
							IBinary_GetLen(trx->response)
						);
					}
					else {
						rc = ESIF_E_NO_MEMORY;
					}
						
				}
			}
			ClientRequest_Destructor(object);
		}
		atomic_dec(&self->activeThreads);
	}
	WebWorker_PutRef(rpcWorker);
	return rc;
}

// RPC Worker Thread Wrapper
static void *ESIF_CALLCONV Irpc_WorkerThread(void *ctx)
{
	WebServerPtr self = g_WebServer;
	WebWorkerPtr rpcWorker = (WebWorkerPtr)ctx;
	WS_TRACE_INFO("RPC Worker Thread Starting");
	WebServer_IrpcMain(self, rpcWorker);
	WS_TRACE_INFO("RPC Worker Thread Exiting");
	return 0;
}

// Start Web Server
esif_error_t WebServer_Start(WebServerPtr self)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;;
	if (self) {
		if (atomic_read(&self->isActive) || atomic_read(&self->activeThreads)) {
			rc = ESIF_E_WS_ALREADY_STARTED;
		}
		else {
			atomic_set(&self->isActive, 1);
			rc = ESIF_OK;
			WebListenerPtr listener = NULL;
			for (int j = 0; j < IPF_WS_MAX_LISTENERS; j++) {
				if (self->listeners[j].serverAddr[0]) {
					WS_CONSOLE_MSG("Starting IPF Server Listener [%s]\n", self->listeners[j].serverAddr);

					// Delete any orphan Socket Files
					if (IpfIpc_ParseServerAddress(self->listeners[j].serverAddr, &self->listeners[j].sockaddr) == ESIF_OK && self->listeners[j].sockaddr.type == AF_UNIX) {
						listener = (listener ? listener : &self->listeners[j]);
						esif_ccb_unlink(self->listeners[j].sockaddr.un_addr.sun_path);
					}
				}
			}

			// Attempt to Delete & Recreate Socket File Folder so it exists before FileWatcher Thread starts
			if (listener) {
				char lockpath[MAX_PATH] = {0};
				esif_ccb_sprintf(sizeof(lockpath), lockpath, "%s" ESIF_PATH_SEP "%s", IPC_NAMEDPIPE_PATH, IPC_NAMEDPIPE_LOCKFILE);
				esif_ccb_unlink(lockpath);
				esif_ccb_unlink(IPC_NAMEDPIPE_PATH);
				esif_ccb_makepath(IPC_NAMEDPIPE_PATH);
				if (WebListener_SetSocketFolderSecurity(listener) != ERROR_SUCCESS) {
					WS_TRACE_ERROR("Error[%d] Setting Socket Folder Security\n", esif_ccb_socket_error());
				}
			}

			// Create Doorbell signal so we can wake FileWatcher Thread on exit
			if (self->fileDoorbell == NULL) {
				self->fileDoorbell = esif_ccb_malloc(sizeof(signal_t));
				if (self->fileDoorbell) {
					signal_init(self->fileDoorbell);
				}
			}


			// Start I/O and FileWatcher Threads
			if (rc == ESIF_OK) {
				rc = esif_ccb_thread_create(&self->mainThread, WebServer_WorkerThread, self);
			}
			if (rc == ESIF_OK) {
				rc = esif_ccb_thread_create(&self->fileThread, WebServer_FileChangeThread, self);
			}
		}
	}
	return rc;
}

// Stop Web Server and wait for Active Thread(s) to exit
void WebServer_Stop(WebServerPtr self)
{
	if (self && (atomic_read(&self->isActive) || atomic_read(&self->activeThreads))) {
		atomic_set(&self->isActive, 0);

		// Ring Doorbells to signal blocking select() and FileWatcher threads to exit
		TcpDoorbell_Stop(&self->doorbell);
		if (self->fileDoorbell) {
			signal_post(self->fileDoorbell);
		}

		// Wait for IO and FileWatcher threads to exit
		esif_ccb_thread_join(&self->mainThread);
		esif_ccb_thread_join(&self->fileThread);
	}
	if (self) {
		// Delete Unix Domain Socket Files
		for (size_t j = 0; j < IPF_WS_MAX_LISTENERS; j++) {
			if (self->listeners[j].sockaddr.type == AF_UNIX) {
				esif_ccb_unlink(self->listeners[j].sockaddr.un_addr.sun_path);
			}
		}
		// Delete FileWatcher Doorbell Signal
		if (self->fileDoorbell) {
			signal_uninit(self->fileDoorbell);
			esif_ccb_free(self->fileDoorbell);
			self->fileDoorbell = NULL;
		}
	}
}

// Is WebServer Started or Starting or Stopping?
Bool WebServer_IsStarted(WebServerPtr self)
{
	Bool rc = ESIF_FALSE;
	if (self && (atomic_read(&self->isActive) || atomic_read(&self->activeThreads))) {
		rc = ESIF_TRUE;
	}
	return rc;
}

// Pause Web Server (Reject New Connections)
void WebServer_Pause(WebServerPtr self)
{
	if (self) {
		atomic_set(&self->isPaused, 1);
	}
}

// Resume Web Server (Accept New Connections)
void WebServer_Resume(WebServerPtr self)
{
	if (self) {
		atomic_set(&self->isPaused, 0);
	}
}

// Is Web Server Paused?
Bool WebServer_IsPaused(WebServerPtr self)
{
	Bool rc = ESIF_FALSE;
	if (self) {
		rc = (atomic_read(&self->isPaused) > 0);
	}
	return rc;
}

// Is Web Server in Diagnostic Mode?
Bool WebServer_IsDiagnostic(WebServerPtr self)
{
	Bool rc = ESIF_FALSE;
	if (self) {
		rc = (atomic_read(&self->isDiagnostic) > 0);
	}
	return rc;
}

// Initialize Plugin
esif_error_t WebPlugin_Init()
{
	esif_error_t rc = ESIF_OK;
	int ret = 0;

	if (((ret = esif_ccb_socket_init()) != 0) || ((ret = esif_link_list_init()) != 0)) {
		rc = ESIF_E_WS_INIT_FAILED;
	}

	if ((rc == ESIF_OK) && ((g_WebServer = esif_ccb_malloc(sizeof(*g_WebServer))) == NULL)) {
		rc = ESIF_E_NO_MEMORY;
	}

	if (rc == ESIF_OK) {
		WebServer_Init(g_WebServer);
	}
	else {
		WS_TRACE_ERROR("Socket Init Failure: %s (%d) [Error=%d]\n", esif_rc_str(rc), rc, ret);
	}
	return rc;
}

// Exit Plugin
void WebPlugin_Exit()
{
	if (g_WebServer) {
		WebServer_Exit(g_WebServer);
		esif_ccb_free(g_WebServer);
		g_WebServer = NULL;
	}
	esif_link_list_exit();
	esif_ccb_socket_exit();
}
