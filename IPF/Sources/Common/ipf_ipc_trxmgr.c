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

#include "ipf_trace.h"
#include "ipf_ipc_trxmgr.h"

/*
** IrpcTransaction Class
*/

// Create a Transaction and Take a Reference
IrpcTransaction *IrpcTransaction_Create(eIrpcMsgType msgType)
{
	IrpcTransaction *self = esif_ccb_malloc(sizeof(*self));
	if (self) {
		IrpcTransaction_GetRef(self);
		signal_init(&self->sync);
		self->timestamp = esif_ccb_realtime_current();
		self->result = ESIF_OK;

		if (((msgType == IrpcMsg_ProcRequest) && ((self->request = IBinary_Create()) == NULL)) ||
			((msgType == IrpcMsg_ProcResponse) && ((self->response = IBinary_Create()) == NULL))) {
			IrpcTransaction_Destroy(self);
			self = NULL;
		}
		else {
			// Generate Unique Transaction ID
			#define TRXID_MIN 0x8000000000000000	// Initial Transaction ID Seed
			#define TRXID_MAX 0xffffffffffff0000	// Max Transaction ID before Rollover
			static atomic64_t trxIdGenerator = ATOMIC_INIT(TRXID_MIN);
			self->trxId = (UInt64)atomic64_inc(&trxIdGenerator);
			if (self->trxId == TRXID_MAX) {
				atomic64_set(&trxIdGenerator, TRXID_MIN);
			}
		}
	}
	return self;
}

// Destroy a Transaction (Reference Count must be 0)
void IrpcTransaction_Destroy(IrpcTransaction *self)
{
	if (self) {
		IBinary_Destroy(self->request);
		IBinary_Destroy(self->response);
		self->request = NULL;
		self->response = NULL;
		signal_uninit(&self->sync);
		esif_ccb_memset(self, 0, sizeof(*self));
		esif_ccb_free(self);
	}
}

// Take a Reference
void IrpcTransaction_GetRef(IrpcTransaction *self)
{
	if (self) {
		atomic_inc(&self->refCount);
	}
}

// Release a Reference and Destroy Transaction when RefCount is 0
void IrpcTransaction_PutRef(IrpcTransaction *self)
{
	if (self) {
		if (atomic_dec(&self->refCount) == 0) {
			IrpcTransaction_Destroy(self);
		}
	}
}

// Signal Waiting Thread that Transaction Response has been received or timed out
void IrpcTransaction_Signal(IrpcTransaction *self)
{
	if (self) {
		signal_post(&self->sync);
	}
}

// Wait for Transaction Response to be received or time out
esif_error_t IrpcTransaction_Wait(IrpcTransaction *self)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		signal_wait(&self->sync);
		rc = self->result;
		if (rc == ESIF_OK && self->response == NULL) {
			rc = ESIF_E_SESSION_REQUEST_FAILED;
		}
	}
	return rc;
}

/*
** IpfTrxMgr Class
*/

#define	TRX_POOL_INITIAL			ESIF_MAX_CLIENTS		// Intial Size of Transaction Request Pool (Waiting Threads)
#define	TRX_POOL_GROWBY				2						// Size to Grow Transaction Request Pool when full
#define	TRX_POOL_MAXSIZE			(ESIF_MAX_CLIENTS * 64)	// Max Size of Transaction Request Pool (Max Waiting Threads)

#ifdef ESIF_ATTR_DEBUG
#define TRX_EXPIRE_TIMEOUT_SECONDS	(15*60)					// Expire Active Transactions after this many seconds (Default)
#else
#define TRX_EXPIRE_TIMEOUT_SECONDS	45						// Expire Active Transactions after this many seconds (Default)
#endif
#define TRX_EXPIRE_ALL				0						// Expire All Active Transactions Immediately
#define TRX_EXPIRE_TIMEOUT_NEVER	0x7fffffff				// Never Expire Transactions

// IPF Transaction Manager
typedef struct IpfTrxMgr_s {
	esif_ccb_lock_t	lock;		// Transaction Manager Lock
	IrpcTransaction	**trxPool;	// Active Transaction Pool
	size_t poolSize;			// Active Transaction Pool size
	atomic_t rpcTimeout;		// RPC Timeout in Seconds
} IpfTrxMgr;

static IpfTrxMgr g_trxMgr;	// Singleton Instance

static void IpfTrxMgr_ExpireTimeout(size_t seconds);

esif_error_t IpfTrxMgr_Init(void)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	IpfTrxMgr *self = &g_trxMgr;
	if (self) {
		self->trxPool = (IrpcTransaction **)esif_ccb_malloc(sizeof(IrpcTransaction *) * TRX_POOL_INITIAL);
		if (self->trxPool == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}
		else {
			esif_ccb_lock_init(&self->lock);
			self->poolSize = TRX_POOL_INITIAL;
			atomic_set(&self->rpcTimeout, TRX_EXPIRE_TIMEOUT_SECONDS);
			rc = ESIF_OK;
		}
	}
	return rc;
}

void IpfTrxMgr_Uninit(void)
{
	IpfTrxMgr *self = &g_trxMgr;
	if (self) {
		IpfTrxMgr_ExpireAll();
		esif_ccb_free(self->trxPool);
		self->trxPool = NULL;
		self->poolSize = 0;
		esif_ccb_lock_uninit(&self->lock);
	}
}

void IpfTrxMgr_ExpireInactive(void)
{
	IpfTrxMgr *self = &g_trxMgr;
	if (self) {
		IpfTrxMgr_ExpireTimeout(atomic_read(&self->rpcTimeout));
	}
}

void IpfTrxMgr_ExpireAll(void)
{
	IpfTrxMgr_ExpireTimeout(TRX_EXPIRE_ALL);
}

static void IpfTrxMgr_ExpireTimeout(size_t seconds)
{
	IpfTrxMgr *self = &g_trxMgr;
	if (self && seconds < TRX_EXPIRE_TIMEOUT_NEVER) {
		esif_ccb_write_lock(&self->lock);
		esif_ccb_realtime_t now = esif_ccb_realtime_current();

		for (size_t j = 0; j < self->poolSize; j++) {
			IrpcTransaction *trx = self->trxPool[j];
			if (trx && (seconds == TRX_EXPIRE_ALL || esif_ccb_realtime_diff_sec(trx->timestamp, now) >= (Int64)seconds)) {
				IPF_TRACE_INFO("Expiring TrxID 0x%llx Session 0x%llx Timeout=%lf\n", trx->trxId, trx->ipfHandle, esif_ccb_realtime_diff_msec(trx->timestamp, now) / 1000);
				self->trxPool[j] = NULL;
				IBinary_Destroy(trx->request);
				IBinary_Destroy(trx->response);
				trx->request = NULL;
				trx->response = NULL;
				IrpcTransaction_Signal(trx);
				IrpcTransaction_PutRef(trx);
			}
		}
		esif_ccb_write_unlock(&self->lock);
	}
}

esif_error_t IpfTrxMgr_AddTransaction(IrpcTransaction *trx)
{
	IpfTrxMgr *self = &g_trxMgr;
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && trx) {
		rc = ESIF_E_MAXIMUM_CAPACITY_REACHED;

		// Find next free slot and add Transaction
		esif_ccb_write_lock(&self->lock);
		for (size_t j = 0; j < self->poolSize; j++) {
			if (self->trxPool[j] == NULL) {
				IrpcTransaction_GetRef(trx);
				self->trxPool[j] = trx;
				rc = ESIF_OK;
				break;
			}
		}

		// Grow Transaction Pool if full
		if (rc != ESIF_OK && self->poolSize + TRX_POOL_GROWBY <= TRX_POOL_MAXSIZE) {
			IrpcTransaction **newPool = (IrpcTransaction **)esif_ccb_realloc(self->trxPool, sizeof(IrpcTransaction *) * (self->poolSize + TRX_POOL_GROWBY));
			if (newPool) {
				esif_ccb_memset(&newPool[self->poolSize], 0, sizeof(IrpcTransaction *) * TRX_POOL_GROWBY);
				IrpcTransaction_GetRef(trx);
				newPool[self->poolSize] = trx;
				self->trxPool = newPool;
				self->poolSize += TRX_POOL_GROWBY;
				rc = ESIF_OK;
			}
		}
		esif_ccb_write_unlock(&self->lock);
	}
	return rc;
}

IrpcTransaction *IpfTrxMgr_GetTransaction(esif_handle_t ipfHandle, UInt64 trxId)
{
	IpfTrxMgr *self = &g_trxMgr;
	IrpcTransaction *trx = NULL;
	if (self) {
		esif_ccb_write_lock(&self->lock);
		for (size_t j = 0; j < self->poolSize; j++) {
			if (self->trxPool[j]
				&& (self->trxPool[j]->ipfHandle == ipfHandle || ipfHandle == ESIF_INVALID_HANDLE)
				&& (self->trxPool[j]->trxId == trxId || trxId == IPFTRX_MATCHANY)) {
				trx = self->trxPool[j];
				self->trxPool[j] = NULL;
				IrpcTransaction_PutRef(trx);
				break;
			}
		}
		esif_ccb_write_unlock(&self->lock);
	}
	return trx;
}

double IpfTrxMgr_GetMinTimeout()
{
	IpfTrxMgr *self = &g_trxMgr;
	double result = TRX_EXPIRE_TIMEOUT_NEVER;
	if (self) {
		esif_ccb_realtime_t now = esif_ccb_realtime_current();
		esif_ccb_read_lock(&self->lock);
		for (size_t j = 0; j < self->poolSize; j++) {
			if (self->trxPool[j]) {
				double timeout = (double)atomic_read(&self->rpcTimeout) - (esif_ccb_realtime_diff_msec(self->trxPool[j]->timestamp, now) / 1000.0);
				if (timeout > 0 && timeout < result) {
					result = timeout;
				}
			}
		}
		esif_ccb_read_unlock(&self->lock);
	}
	return result;
}

// Get Current RPC Transaction Timeout
size_t IpfTrxMgr_GetTimeout()
{
	IpfTrxMgr *self = &g_trxMgr;
	size_t timeout = 0;
	if (self) {
		timeout = atomic_read(&self->rpcTimeout);
	}
	return timeout;
}

// Set RPC Transaction Timeout (0 = Use Default)
void IpfTrxMgr_SetTimeout(size_t timeout)
{
	IpfTrxMgr *self = &g_trxMgr;
	if (self) {
		atomic_set(&self->rpcTimeout, (timeout ? timeout : TRX_EXPIRE_TIMEOUT_SECONDS));
	}
}

// Reset all Pending Transaction Timeouts after Resuming Timeouts
void IpfTrxMgr_ResetTimeouts()
{
	IpfTrxMgr *self = &g_trxMgr;
	if (self) {
		esif_ccb_realtime_t now = esif_ccb_realtime_current();
		esif_ccb_read_lock(&self->lock);
		for (size_t j = 0; j < self->poolSize; j++) {
			if (self->trxPool[j]) {
				self->trxPool[j]->timestamp = now;
			}
		}
		esif_ccb_read_unlock(&self->lock);
	}
}
