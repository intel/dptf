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

#ifdef ESIF_ATTR_USER
# define ESIF_TRACE_DEBUG_DISABLED
#endif
#include "esif.h"
#include "esif_ipc.h"


#ifdef ESIF_ATTR_KERNEL

#include <linux/miscdevice.h>	/* Linux /dev/esif MISC Device      */
#include <linux/poll.h>		/* Linux Polling Support            */

/* Debug Logging Defintions */
#define ESIF_DEBUG_MODULE ESIF_DEBUG_MOD_LINUX

#define INIT_DEBUG      0	/* this */
#define IPC_DEBUG       1	/* this */
#define IPC_POLL        2	/* this */
#define RESERVED4       3	/* this */
#define RESERVED8       4	/* esif_ipc.c */
#define RESERVED10      5	/* esif_ipc.c */
#define RESERVED20      6	/* esif_ipc.c */
#define RESERVED40      7	/* esif_ipc.c */

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_IPC, INIT_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_IPC(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_IPC, IPC_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_IPC_POLL(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_IPC, IPC_POLL, format, ##__VA_ARGS__)

/*
 ******************************************************************************
 * PRIVATE
 ******************************************************************************
 */

/* Translate User Space IPC to Kernel */
static struct esif_ipc *esif_ipc_user_to_kernel(struct esif_ipc *ipc_user_ptr)
{
	int ipc_len = 0;
	struct esif_ipc ipc_hdr  = {0};
	struct esif_ipc *ipc_ptr = NULL;

	if (NULL == ipc_user_ptr)
		return NULL;

	/* Copy from user an error indicates how many bytes copied. */
	if (copy_from_user(&ipc_hdr, ipc_user_ptr,
			   sizeof(struct esif_ipc)) > 0) {
		return NULL;
	}

	/* Sanity Check */
	if (ESIF_IPC_VERSION != ipc_hdr.version)
		return NULL;

	ESIF_TRACE_DYN_IPC(
		"linux_%s: ipc version=%d, type=%d, data_len=%d, rc=%d\n",
		__func__,
		ipc_hdr.version,
		ipc_hdr.type,
		(int)ipc_hdr.data_len,
		(int)ipc_hdr.return_code);

	ipc_len = ipc_hdr.data_len + sizeof(struct esif_ipc);
	ipc_ptr = esif_ccb_malloc(ipc_len);
	if (NULL == ipc_ptr)
		return NULL;

	/* Copy from user an error indicates how many bytes copied. */
	if (copy_from_user(ipc_ptr, ipc_user_ptr, ipc_len) > 0) {
		esif_ccb_free(ipc_ptr);
		return NULL;
	}
	return ipc_ptr;
}


/* Translate Kernel IPC To User Space */
static int esif_ipc_kernel_to_user(
	struct esif_ipc *ipc_user_ptr,
	struct esif_ipc *ipc_kernel_ptr
	)
{
	u32 len = 0;
	if (NULL == ipc_user_ptr || NULL == ipc_kernel_ptr)
		return -EINVAL;

	len = ipc_kernel_ptr->data_len + sizeof(struct esif_ipc);

	ESIF_TRACE_DYN_IPC(
		"linux_%s: ipc version=%d, type=%d, len = %d, "
		"data_len=%d, rc=%d\n",
		__func__,
		ipc_kernel_ptr->version,
		ipc_kernel_ptr->type,
		(int)len,
		(int)ipc_kernel_ptr->data_len,
		ipc_kernel_ptr->return_code);

	/* TODO Check For Fit When Possible */
	if (copy_to_user(ipc_user_ptr, ipc_kernel_ptr, len) > 0)
		return -ENOMEM;

	return len;
}


/* IOCTL */
static int esif_ipc_ioctl(struct esif_ipc *ipc_user_ptr)
{
	int rc = 0;
	struct esif_ipc *ipc_req_ptr = esif_ipc_user_to_kernel(ipc_user_ptr);
	struct esif_ipc *ipc_rsp_ptr = NULL;

	if (NULL == ipc_req_ptr)
		return -ENOMEM;

	ESIF_TRACE_DYN_IPC("linux_%s: user %p kernel %p\n",
			   __func__,
			   ipc_user_ptr,
			   ipc_req_ptr);
	ipc_rsp_ptr = esif_ipc_process(ipc_req_ptr);

	if (NULL != ipc_rsp_ptr) {
		esif_ipc_kernel_to_user((struct esif_ipc *)ipc_user_ptr,
					ipc_rsp_ptr);
		ESIF_TRACE_DYN_IPC("linux_%s: user %p kernel %p\n",
				   __func__,
				   ipc_user_ptr,
				   ipc_rsp_ptr);
		if (ipc_req_ptr != ipc_rsp_ptr)
			esif_ipc_free(ipc_rsp_ptr);
	} else {
		rc = -EINVAL;
	}
	esif_ccb_free(ipc_req_ptr);
	return rc;
}


/* IOCTL Operation */
static long esif_ioctl(
	struct file *filp_ptr,
	u_int cmd,
	u_long arg
	)
{
	int rc = -EINVAL;

	ESIF_TRACE_DYN_IPC("linux_%s: ioctl cmd %x\n", __func__, cmd);
	switch (cmd) {
	case ESIF_IOCTL_IPC_NOOP:
		ESIF_TRACE_DYN_IPC("linux_%s: NOOP cmd=%x\n", __func__, cmd);
		break;

	case ESIF_IOCTL_IPC:
		ESIF_TRACE_DYN_IPC("linux_%s: ESIF_IOCTL_IPC\n", __func__);
		rc = esif_ipc_ioctl((struct esif_ipc *)arg);
		break;

	default:
		ESIF_TRACE_DYN_IPC("linux_%s: NOT IMPLEMENTED %x\n",
				   __func__,
				   cmd);
		break;
	}
	return rc;
}


/*
** File Operations
*/
static DECLARE_WAIT_QUEUE_HEAD(esif_waitqueue);

/* IOCTL */
static ssize_t esif_file_ipc(
	struct file *fp,
	char __user *buf,
	size_t count,
	loff_t *ppos
	)
{
	u32 len = 0;
	struct esif_ipc *ipc_req_ptr = esif_ipc_user_to_kernel(
			(struct esif_ipc *)buf);
	struct esif_ipc *ipc_rsp_ptr = NULL;

	if (count < sizeof(struct esif_ipc))
		return -EINVAL;

	if (NULL == ipc_req_ptr)
		return -ENOMEM;

	ESIF_TRACE_DYN_IPC("linux_%s: user %p kernel %p count %d\n",
			   __func__, buf, ipc_req_ptr, (int)count);

	ipc_rsp_ptr = esif_ipc_process(ipc_req_ptr);

	if (NULL != ipc_rsp_ptr) {
		len = esif_ipc_kernel_to_user((struct esif_ipc *)buf,
						ipc_rsp_ptr);
		ESIF_TRACE_DYN_IPC("linux_%s: user %p kernel %p count = %d\n",
				   __func__, buf, ipc_rsp_ptr, (int)count);

		if (ipc_req_ptr != ipc_rsp_ptr)
			esif_ipc_free(ipc_rsp_ptr);
	}
	esif_ccb_free(ipc_req_ptr);
	return len;
}


/* Read */
static ssize_t esif_read_file_ipc(
	struct file *fp,
	char __user *buf,
	size_t count,
	loff_t *ppos
	)
{
	return esif_file_ipc(fp, buf, count, ppos);
}


/* Write */
static ssize_t esif_write_file_ipc(
	struct file *fp,
	const char __user *buf,
	size_t count,
	loff_t *ppos
	)
{
	return esif_file_ipc(fp, (char __user *)buf, count, ppos);
}


/* Poll */
static unsigned int esif_poll(
	struct file *fp,
	poll_table * wait
	)
{
	ESIF_TRACE_DYN_IPC_POLL("linux_%s: POLL Event Queue\n", __func__);

	poll_wait(fp, &esif_waitqueue, wait);
	return esif_event_queue_size() == 0 ? 0 : POLLIN | POLLRDNORM;
}


/* Open */
static int esif_open(struct inode *inode, struct file *filp)
{
	ESIF_TRACE_DYN_IPC("linux_%s: %s Device Opened\n", __func__,
			   IPC_DEVICE);
	return 0;
}


/* Release */
static int esif_release(struct inode *inode, struct file *filp)
{
	ESIF_TRACE_DYN_IPC("linux_%s: %s Device Closed\n", __func__,
			   IPC_DEVICE);
	return 0;
}


/* Seek Not Used */
static loff_t esif_llseek(struct file *filp, loff_t a, int b)
{
	return -EINVAL;
}


/* File Operations */
static const struct file_operations esif_fops = {
	.owner          = THIS_MODULE,
	.read           = esif_read_file_ipc,
	.write          = esif_write_file_ipc,
	.poll           = esif_poll,
	.unlocked_ioctl = esif_ioctl,
	.open           = esif_open,
	.release        = esif_release,
	.llseek         = esif_llseek	/* noop_llseek */
};

/* Define ESIF MISC Device */
static struct miscdevice esif_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = IPC_DEVICE,
	.fops  = &esif_fops
};

/*
 ******************************************************************************
 * PUBLIC
 ******************************************************************************
 */

/* Init */
enum esif_rc esif_os_ipc_init(esif_device_t device)
{
	int rc = 0;
	ESIF_TRACE_DYN_INIT("linux_%s: Initialize IPC\n", __func__);

	/* Create misc_device here */
	rc = misc_register(&esif_device);
	if (rc) {
		dev_dbg(esif_device.this_device,
			"failed to register misc dev %d\n",
			rc);
		return ESIF_E_UNSPECIFIED;
	}

	ESIF_TRACE_DYN_INIT("linux_%s: IPC Registration:\n"
			    "  Method: IOCTL\n"
			    "  Device: /dev/%s\n"
			    "  Type:   char\n"
			    "  Mode:   READ/WRITE\n",
			    __func__, IPC_DEVICE);

	return ESIF_OK;
}


/* Exit */
void esif_os_ipc_exit(esif_device_t device)
{
	int rc = 0;

	ESIF_TRACE_DYN_INIT("linux_%s: Exit IPC\n", __func__);

	rc = misc_deregister(&esif_device);
	if (rc) {
		dev_dbg(esif_device.this_device,
			"failed to unregister misc dev %d\n",
			rc);
	}

	ESIF_TRACE_DYN_INIT("linux_%s: IOCTL Device %s UnRegistered\n",
			    __func__,
			    IPC_DEVICE);
}


#endif /* ESIF_ATTR_KERNEL */

#ifdef ESIF_ATTR_USER

/*
 ******************************************************************************
 * User
 ******************************************************************************
 */

/* IPC OS Connect */
esif_handle_t esif_os_ipc_connect(char *session_id)
{
	int fd = 0;
	char device[64];/* Jacob Is There a Standard Path Length? */

	sprintf(device, "/dev/%s", IPC_DEVICE);
	fd = open(device, O_RDWR);

	ESIF_TRACE_DEBUG("linux_%s: session_id=%s device=%s handle=%d\n",
			 __func__, session_id, device, fd);
	return fd;
}


/* IPC OS Disconnect */
void esif_os_ipc_disconnect(esif_handle_t handle)
{
	ESIF_TRACE_DEBUG("linux_%s: IPC handle = %d\n", __func__, handle);
	close(handle);
}


/* IPC OS Execution */
enum esif_rc esif_os_ipc_execute(
	esif_handle_t handle,
	struct esif_ipc *ipc_ptr
	)
{
	int rc = 0;

	ESIF_TRACE_DEBUG("linux_%s: handle = %d, IPC = %p\n",
			 __func__,
			 handle,
			 ipc_ptr);

	/* use IOCTL or read here */
#ifdef ESIF_ATTR_OS_ANDROID
	rc = read(handle, ipc_ptr, ipc_ptr->data_len + sizeof(struct esif_ipc));
	ESIF_TRACE_DEBUG("linux_%s: READ handle = %d, IPC = %p rc = %d\n",
			 __func__, handle, ipc_ptr, rc);
#else
	rc = ioctl(handle, ESIF_IOCTL_IPC, ipc_ptr);
	ESIF_TRACE_DEBUG("linux_%s: IOCTL handle = %d, IPC = %p rc = %d\n",
			 __func__, handle, ipc_ptr, rc);
	if (rc)
		return ESIF_E_UNSPECIFIED;
#endif

	return ESIF_OK;
}


#endif /* ESIF_ATTR_USER */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
