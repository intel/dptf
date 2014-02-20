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

#define ESIF_ATTR_OS_LINUX_DRIVER
#define ESIF_DEBUG_MODULE ESIF_DEBUG_MOD_LINUX
#define DRIVER_NAME "dptf_pch"

#include "esif_lf.h"

/* Event Receive */
static enum esif_rc pch_recv_event(
	enum esif_event_type type,
	u16 domain,
	struct esif_data *data_ptr
	)
{
	ESIF_TRACE_DEBUG("%s: event received %d\n", ESIF_FUNC, type);
	return ESIF_OK;
}


/* ESIF Particpant INTERFACE */
static struct esif_participant_iface pi = {
	.version     = ESIF_PARTICIPANT_VERSION,
	.class_guid  = ESIF_PARTICIPANT_PCH_CLASS_GUID,
#ifndef ESIF_ATTR_PLATFORM
	.enumerator  = ESIF_PARTICIPANT_ENUM_PCI,
#else /* ESIF_ATTR_PLATFORM */
	.enumerator  = ESIF_PARTICIPANT_ENUM_PLAT,
#endif
	.flags       =                             0x0,
	.name        = ESIF_PARTICIPANT_PCH_NAME,
	.desc        = ESIF_PARTICIPANT_PCH_DESC,
	.driver_name = "?",	/* Filled In Dynamically By Driver */
	.device_name = "?",	/* Filled In Dynamically By Driver */
	.device_path = "NA",	/* Filled In Dynamically By Driver */
	.device      = NULL,	/* Driver Assigned                 */
	.mem_base    = NULL,	/* Driver Assigned                 */
	.mem_size    = 0,	/* Driver Assigned                 */
	.acpi_handle = NULL,	/* Driver Assigned                 */

	/* EVENT */
	.recv_event = pch_recv_event,
	.send_event = NULL,	/* Filled In Dynamically By esif_lf */

	/* ACPI */
	.acpi_device = "NA",	/* NA For PCH Device               */
	.acpi_scope  = "?",	/* Filled In Dynamically By Driver */
	.acpi_uid    = ESIF_PARTICIPANT_INVALID_UID,  /* NA For PCH Device */
	.acpi_type   = ESIF_PARTICIPANT_INVALID_TYPE, /* NA For PCH Device */

	/* PCI */
	.pci_vendor     = 0,	/* Filled In Dynamically By Driver    */
	.pci_device     = 0,	/* Filled In Dynamically By Driver    */
	.pci_bus        = 0x0,	/* Hardware Bus                       */
	.pci_bus_device = 0x1f,	/* Hardware Device On Bus             */
	.pci_function   = 0x6,	/* Hardware Function Of Device On Bus */
	.pci_revision   = 0,	/* Filled In Dynamically By Driver    */
	.pci_class      = 0,	/* Filled In Dynamically By Driver    */
	.pci_sub_class  = 0,	/* Filled In Dynamically By Driver    */
	.pci_prog_if    = 0	/* Filled In Dynamically By Driver    */
};

/*
 ******************************************************************************
 * LINUX Driver Code Below Here.  Implemetned As Platform Driver For
 * Prototype to avoid the need for a CRB.
 *******************************************************************************
 * Two Versions Can Be Built
 * PCI - Requires Platform Hardware Support For Enum
 * Platform - Will Enum On Any Platform and Emulate HW
 */

/*
 ******************************************************************************
 * PRIVATE
 ******************************************************************************
 */

/* Event Send */
static enum esif_rc pch_send_event(
	enum esif_event_type type,
	u16 domain,
	void *data_ptr
	)
{
	if (NULL == pi.send_event)
		return ESIF_E_CALLBACK_IS_NULL;

	return pi.send_event(&pi, type, domain, data_ptr);
}


#ifndef ESIF_ATTR_PLATFORM

/*
 * PCI Bus Driver
 */

/* esif_pci_pch_ids pulled from autogen.h */
MODULE_DEVICE_TABLE(pci, esif_pci_pch_ids);

/* ACPI Send Event */
enum esif_rc acpi_send_event(
	struct esif_participant_iface *pi_ptr,
	enum esif_event_type type, 
	u16 domain, 
	struct esif_data *data_ptr
        )
{
        if (NULL == pi_ptr)
                return ESIF_E_PARAMETER_IS_NULL;

        if (NULL == pi_ptr->send_event)
                return ESIF_E_CALLBACK_IS_NULL;

        return pi_ptr->send_event(pi_ptr, type, domain, data_ptr);
}

/* ACPI Event Handler */
static void acpi_notify(acpi_handle handle, u32 event, void *data)
{
	struct esif_participant_iface *pi_ptr = data;
	struct esif_data event_data = { 
		ESIF_DATA_UINT32, &event, sizeof(event), sizeof(event)};

	ESIF_TRACE_DEBUG("%s: ACPI event on PCH %d\n", ESIF_FUNC, event);
	acpi_send_event(pi_ptr, ESIF_EVENT_ACPI, 'NA', &event_data);
}

/* Probe */
static int pci_pch_probe(
	struct pci_dev *dev_ptr,
	const struct pci_device_id *dev_id_ptr
	)
{
	enum esif_rc rc     = ESIF_OK;
	char *kobj_path_ptr = NULL;
	int err = 0;
	int enabled         = 0;
	u64 resource_start;	/* memory/io resource start */
	u64 resource_len;	/* memory/io resource length */

	ESIF_TRACE_DEBUG("%s: dev %p pci_id %p\n",
			 ESIF_FUNC,
			 dev_ptr,
			 dev_id_ptr);

	/* Grab Linux Device Driver Not Platform Driver */
	pi.device = &dev_ptr->dev;
	snprintf(pi.driver_name, ESIF_NAME_LEN, "%s.ko", DRIVER_NAME);
	snprintf(pi.device_name, ESIF_NAME_LEN, "%s", dev_name(&dev_ptr->dev));

	kobj_path_ptr = kobject_get_path(&dev_ptr->dev.kobj, GFP_ATOMIC);
	if (NULL != kobj_path_ptr) {
		snprintf(pi.device_path, ESIF_PATH_LEN, "%s", kobj_path_ptr);
		kfree(kobj_path_ptr);
	}

	err = pci_enable_device(dev_ptr);
	if (err) {
		ESIF_TRACE_ERROR("%s: failed to enable pci device\n",
				 ESIF_FUNC);
		goto error_cleanup;
	}

	enabled = 1;
	ESIF_TRACE_DEBUG("%s: pci device enabled\n", ESIF_FUNC);

	resource_start = pci_resource_start(dev_ptr, 0);
	if (!resource_start) {
		ESIF_TRACE_ERROR("%s: failed to find iomem\n", ESIF_FUNC);
		err = -ENOMEM;
		goto error_cleanup;
	}
	ESIF_TRACE_DEBUG("%s: resource_start %llu\n", ESIF_FUNC,
			 resource_start);

	resource_len = pci_resource_len(dev_ptr, 0);
	ESIF_TRACE_DEBUG("%s: resource_len %llu\n", ESIF_FUNC, resource_len);

	err = pci_request_regions(dev_ptr, DRIVER_NAME);
	if (err) {
		ESIF_TRACE_ERROR("%s: failed to request pci region\n",
				 ESIF_FUNC);
		goto error_cleanup;
	}
	ESIF_TRACE_DEBUG("%s: have pci region\n", ESIF_FUNC);

	pi.mem_size = resource_len;
	pi.mem_base = ioremap_nocache(resource_start, resource_len);
	if (!pi.mem_base) {
		err = -ENOMEM;
		ESIF_TRACE_ERROR("%s: failed to map iomem\n", ESIF_FUNC);
		goto error_cleanup;
	}
	ESIF_TRACE_DEBUG("%s: mem_base %p\n", ESIF_FUNC, pi.mem_base);
	{
		u8 *class_ptr = (u8 *)&dev_ptr->class;
		pi.pci_vendor    = dev_ptr->vendor;
		pi.pci_device    = dev_ptr->device;
		pi.pci_revision  = dev_ptr->revision;
		pi.pci_class     = *(class_ptr + 2);
		pi.pci_sub_class = *(class_ptr + 1);
		pi.pci_prog_if   = *class_ptr;
	}

	switch (dev_ptr->device) {
	default:
		ESIF_TRACE_DEBUG("%s: %s pch thermal device detected\n",
				 ESIF_FUNC, esif_device_str(dev_ptr->device));
		break;
	}

	/* Try To Map ACPI Handle For This PCI Device */
	pi.acpi_handle = DEVICE_ACPI_HANDLE(&dev_ptr->dev);
	if (pi.acpi_handle) {
		struct acpi_buffer acpi_scope = {
			.length = ACPI_ALLOCATE_BUFFER};
		acpi_status status = acpi_get_name(pi.acpi_handle,
						   ACPI_FULL_PATHNAME,
						   &acpi_scope);
		ESIF_TRACE_DEBUG("%s: have acpi handle %p\n",
				 ESIF_FUNC,
				 pi.acpi_handle);

		if (ACPI_FAILURE(status)) {
			ESIF_TRACE_DEBUG("%s: acpi_get_name error %d\n",
					 ESIF_FUNC,
					 status);
		} else {
			strncpy(pi.acpi_scope,
				acpi_scope.pointer,
				ESIF_SCOPE_LEN);
			ESIF_TRACE_DEBUG("%s: name = %s\n",
					 ESIF_FUNC,
					 pi.acpi_scope);
			kfree(acpi_scope.pointer);
		}
		status = acpi_install_notify_handler(pi.acpi_handle, 
				ACPI_ALL_NOTIFY, acpi_notify, &pi);
		if (ACPI_FAILURE(status))
			ESIF_TRACE_DEBUG("%s: acpi_install_notify_handler error %d\n",
					ESIF_FUNC, status);

	}
	rc = esif_lf_register_participant(&pi);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;

error_cleanup:
	if (enabled)
		pci_disable_device(dev_ptr);

	return err;
}


/* Remove */
static void pci_pch_remove(struct pci_dev *dev_ptr)
{
	enum esif_rc rc;

	if (pi.acpi_handle)
		acpi_remove_notify_handler(pi.acpi_handle, ACPI_ALL_NOTIFY, 
				acpi_notify);

	rc = esif_lf_unregister_participant(&pi);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);

	iounmap(pi.mem_base);
	pci_release_region(dev_ptr, 0);
}


/* Shutdonw */
static void pci_pch_shutdown(struct pci_dev *dev_ptr)
{
	enum esif_rc rc;

	rc = pch_send_event(ESIF_EVENT_PARTICIPANT_SHUTDOWN, 'NA', NULL);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
}


/* Suspend */
static int pci_pch_suspend(
	struct pci_dev *dev_ptr,
	pm_message_t state
	)
{
	enum esif_rc rc;

	rc = pch_send_event(ESIF_EVENT_PARTICIPANT_SUSPEND, 'NA', &state);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Resume */
static int pci_pch_resume(struct pci_dev *dev_ptr)
{
	enum esif_rc rc = pch_send_event(ESIF_EVENT_PARTICIPANT_RESUME,
					 'NA',
					 NULL);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* PCI Driver */
static struct pci_driver pci_pch_driver = {
	.name     = DRIVER_NAME,
	.id_table = esif_pci_pch_ids,
	.probe    = pci_pch_probe,
	.remove   = pci_pch_remove,
	.shutdown = pci_pch_shutdown,
	.suspend  = pci_pch_suspend,
	.resume   = pci_pch_resume
};

#else /* ESIF_ATTR_PLATFORM */

/*
** Platform Bus Driver
*/

/* Probe */
static int platform_pch_probe(struct platform_device *dev_ptr)
{
	enum esif_rc rc     = ESIF_OK;
	char *kobj_path_ptr = NULL;

	ESIF_TRACE_DEBUG("%s: dev %p\n", ESIF_FUNC, dev_ptr);

	/* Grab Linux Device Driver Not Platform Driver */
	pi.device = &dev_ptr->dev;

	snprintf(pi.driver_name, ESIF_NAME_LEN, "%s.ko", DRIVER_NAME);
	snprintf(pi.device_name, ESIF_NAME_LEN, "%s", dev_name(&dev_ptr->dev));

	kobj_path_ptr = kobject_get_path(&dev_ptr->dev.kobj, GFP_ATOMIC);
	if (NULL != kobj_path_ptr) {
		snprintf(pi.device_path, ESIF_PATH_LEN, "%s", kobj_path_ptr);
		kfree(kobj_path_ptr);
	}

	rc = esif_lf_register_participant(&pi);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Remove */
static int platform_pch_remove(struct platform_device *dev_ptr)
{
	enum esif_rc rc;

	rc = esif_lf_unregister_participant(&pi);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Shutdown */
static void platform_pch_shutdown(struct platform_device *dev_ptr)
{
	enum esif_rc rc;

	rc = pch_send_event(ESIF_EVENT_PARTICIPANT_RESUME, 'NA', NULL);
	ESIF_TRACE_DEBUG("%s dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
}


/* Suspend */
static int platform_pch_suspend(
	struct platform_device *dev_ptr,
	pm_message_t state
	)
{
	enum esif_rc rc;

	rc = pch_send_event(ESIF_EVENT_PARTICIPANT_SUSPEND, 'NA', &state);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Resume */
static int platform_pch_resume(struct platform_device *dev_ptr)
{
	enum esif_rc rc;

	rc = pch_send_event(ESIF_EVENT_PARTICIPANT_RESUME, 'NA', NULL);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Platform Driver */
static struct platform_driver platform_pch_driver = {
	.driver = {
		.name  = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe    = platform_pch_probe,
	.remove   = platform_pch_remove,
	.shutdown = platform_pch_shutdown,
	.suspend  = platform_pch_suspend,
	.resume   = platform_pch_resume
};

static struct platform_device *g_pd_ptr;

#endif

/* Load */
static int __init dptf_pch_load(void)
{
	int rc = 0;

	ESIF_TRACE_INFO("%s: %s\n", ESIF_FUNC, ESIF_PARTICIPANT_PCH_DESC);
	ESIF_TRACE_INFO("%s: %s\n", ESIF_FUNC, ESIF_COPYRIGHT);

#ifndef ESIF_ATTR_PLATFORM
	rc = pci_register_driver(&pci_pch_driver);
#else /* ESIF_ATTR_PLATFORM */
	ESIF_TRACE_INFO("%s: %s\n", ESIF_FUNC, ESIF_PLATFORM_MSG);
	rc = platform_driver_register(&platform_pch_driver);
	/* Simulate Device Insert */

	g_pd_ptr = platform_device_register_simple(DRIVER_NAME, 0, NULL, 0);
	if (NULL == g_pd_ptr)
		rc = -ENOMEM;
#endif
	return rc;
}


/* Unload */
static void __exit dptf_pch_unload(void)
{
#ifndef ESIF_ATTR_PLATFORM
	pci_unregister_driver(&pci_pch_driver);
#else /* ESIF_ATTR_PLATFORM */
	/* Simulate Device Removal */
	if (NULL != g_pd_ptr)
		platform_device_unregister(g_pd_ptr);

	platform_driver_unregister(&platform_pch_driver);
#endif
}


/*
 ******************************************************************************
 * PUBLIC
 ******************************************************************************
 */

/* Module Entry / Exit */
module_init(dptf_pch_load);
module_exit(dptf_pch_unload);

MODULE_VERSION("X1.0.1.0");
MODULE_DESCRIPTION(ESIF_PARTICIPANT_PCH_DESC);
MODULE_LICENSE(ESIF_LICENSE);
MODULE_AUTHOR(ESIF_AUTHOR);

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
