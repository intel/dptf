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
#define DRIVER_NAME "dptf_acpi"

#include "esif_lf.h"

#define ESIF_DEBUG_MODULE ESIF_DEBUG_MOD_ACTION_ACPI

/* Event Receive */
static enum esif_rc acpi_recv_event(
	enum esif_event_type type,
	u16 domain,
	struct esif_data *data_ptr
	)
{
	ESIF_TRACE_DEBUG("%s: domain %d %s(%d)\n",
			 ESIF_FUNC,
			 domain,
			 esif_event_type_str(type),
			 type);

	return ESIF_OK;
}


/* ESIF Participant INTERFACE */
static struct esif_participant_iface pi_template = {
	.version     = ESIF_PARTICIPANT_VERSION,
	.class_guid  = ESIF_PARTICIPANT_ACPI_CLASS_GUID,
#ifndef ESIF_ATTR_PLATFORM
	.enumerator  = ESIF_PARTICIPANT_ENUM_ACPI,
#else
	.enumerator  = ESIF_PARTICIPANT_ENUM_PLAT,
#endif
	.flags       =                              0x0,
	.name        = ESIF_PARTICIPANT_ACPI_NAME,
	.desc        = ESIF_PARTICIPANT_ACPI_DESC,
	.driver_name = "?",	/* Filled In Dynamically By Driver  */
	.device_name = "?",	/* Filled In Dynamically By Driver  */
	.device_path = "NA",	/* Filled In Dynamically By Driver  */
	.device      = NULL,	/* Driver Assigned                  */
	.mem_base    = NULL,	/* Driver Assigned                  */
	.mem_size    = 0,	/* Driver Assigned                  */
	.acpi_handle = NULL,	/* Driver Assigned                  */

	/* EVENT */
	.recv_event = acpi_recv_event,
	.send_event = NULL,	/* Filled In Dynamically By esif_lf  */

	/* ACPI */
	.acpi_device = "?",	/* Override From ACPI Name Space */
	.acpi_scope  = "?",	/* Override From ACPI Name Space */
	.acpi_uid    = ESIF_PARTICIPANT_INVALID_UID,
	.acpi_type   = ESIF_PARTICIPANT_INVALID_TYPE,

	/* PCI */
	.pci_vendor     =                                0,	/* Not Used */
	.pci_device     =                                0,	/* Not Used */
	.pci_bus        =                                0,	/* Not Used */
	.pci_bus_device =                                0,	/* Not Used */
	.pci_function   =                                0,	/* Not Used */
	.pci_revision   =                                0,	/* Not Used */
	.pci_class      =                                0,	/* Not Used */
	.pci_sub_class  =                                0,	/* Not Used */
	.pci_prog_if    = 0	/* Not Used */
};

/*
 ******************************************************************************
 * LINUX Driver Code Below Here.  Implemetned As Platform Driver For
 * Prototype to avoid the need for a CRB.
 ******************************************************************************
 * Two Versions Can Be Built
 * ACPI - Requires BIOS Expose DPTF ACPI Generic Sensors (under EC)
 * Platform - Will Enum On Any Platform and Emulate HW
 */

/*
 ******************************************************************************
 * PRIVATE
 ******************************************************************************
 */

/* Event Send Not Used Yet */
enum
esif_rc acpi_send_event(
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


#ifndef ESIF_ATTR_PLATFORM

/* esif_acpi_ids pulled from autogen.h */
MODULE_DEVICE_TABLE(acpi, esif_acpi_ids);

/*
** ACPI Driver
*/

/* Add */
static int acpi_add(struct acpi_device *acpi_dev_ptr)
{
	enum esif_rc rc        = ESIF_OK;
	char *kobj_path_ptr    = NULL;
	struct esif_participant_iface *pi_ptr = NULL;
	struct device *dev_ptr = &acpi_dev_ptr->dev;

	ESIF_TRACE_DEBUG("%s: acpi_dev %p, dev %p\n",
			 ESIF_FUNC,
			 acpi_dev_ptr,
			 dev_ptr);

	if (NULL == acpi_dev_ptr || NULL == dev_ptr)
		return -ESIF_E_NO_ACPI_SUPPORT;

	pi_ptr = kzalloc(sizeof(struct esif_participant_iface), GFP_ATOMIC);
	if (NULL == pi_ptr)
		return -ENOMEM;

	esif_ccb_memcpy(pi_ptr, &pi_template,
			sizeof(struct esif_participant_iface));
	dev_set_drvdata(dev_ptr, pi_ptr);

	snprintf(pi_ptr->driver_name, ESIF_NAME_LEN, "%s.ko", DRIVER_NAME);
	snprintf(pi_ptr->device_name, ESIF_NAME_LEN, "%s",
		 dev_name(&acpi_dev_ptr->dev));

	kobj_path_ptr = kobject_get_path(&acpi_dev_ptr->dev.kobj, GFP_ATOMIC);

	if (NULL != kobj_path_ptr) {
		snprintf(pi_ptr->device_path, ESIF_PATH_LEN, "%s",
			 kobj_path_ptr);
		kfree(kobj_path_ptr);
	}

	pi_ptr->device      = &acpi_dev_ptr->dev;
	pi_ptr->acpi_handle = acpi_dev_ptr->handle;

	/* Grab Our Scope */
	if (pi_ptr->acpi_handle) {
		struct acpi_buffer acpi_scope = {
			.length = ACPI_ALLOCATE_BUFFER};

		acpi_status status = acpi_get_name(pi_ptr->acpi_handle,
						   ACPI_FULL_PATHNAME,
						   &acpi_scope);

		if (ACPI_FAILURE(status)) {
			ESIF_TRACE_DEBUG("%s: acpi_get_name error %d\n",
					 ESIF_FUNC,
					 status);
		} else {
			strncpy(pi_ptr->acpi_scope,
				(char *)acpi_scope.pointer,
				ESIF_SCOPE_LEN);
			ESIF_TRACE_DEBUG("%s: scope = %s\n",
					 ESIF_FUNC,
					 pi_ptr->acpi_scope);
			kfree(acpi_scope.pointer);
		}
	}

	/* Now the _HID */
	strncpy(pi_ptr->acpi_device, acpi_device_hid(
			acpi_dev_ptr), ESIF_NAME_LEN);

	/* Lookup Description Based On HID Maybe Replaced By _STR Later */
	strncpy(pi_ptr->desc, esif_acpi_device_str(
			pi_ptr->acpi_device), ESIF_DESC_LEN);
	ESIF_TRACE_DEBUG("%s: _HID = %s\n", ESIF_FUNC, pi_ptr->acpi_device);

	strncpy(pi_ptr->name, acpi_device_bid(acpi_dev_ptr), ESIF_NAME_LEN);
	ESIF_TRACE_DEBUG("%s: temp name = %s\n", ESIF_FUNC, pi_ptr->name);

	/* ACPI _STR */
	if (pi_ptr->acpi_handle) {
		struct acpi_buffer acpi_str = {.length = ACPI_ALLOCATE_BUFFER};

		acpi_status status = acpi_evaluate_object(pi_ptr->acpi_handle,
							  "_STR",
							  NULL,
							  &acpi_str);
		if (ACPI_FAILURE(status) || status == AE_NOT_FOUND) {
			ESIF_TRACE_DEBUG(
				"%s: acpi_evaluate_object(_STR) status %d\n",
				ESIF_FUNC,
				status);
		} else {
			union acpi_object *acpi_obj = acpi_str.pointer;
			int len = acpi_obj->string.length / 2;

			esif_ccb_uni2ascii(pi_ptr->desc,
					   ESIF_DESC_LEN,
					   acpi_obj->string.pointer,
					   len);
			ESIF_TRACE_DEBUG("%s: desc = %s\n",
					 ESIF_FUNC,
					 pi_ptr->desc);
			kfree(acpi_str.pointer);
		}
	}

	/* ACPI _UID If Any */
	if (pi_ptr->acpi_handle) {
		struct acpi_buffer acpi_uid = {.length = ACPI_ALLOCATE_BUFFER};

		acpi_status status = acpi_evaluate_object(pi_ptr->acpi_handle,
							  "_UID",
							  NULL,
							  &acpi_uid);
		if (ACPI_FAILURE(status) || status == AE_NOT_FOUND) {
			ESIF_TRACE_DEBUG(
				"%s: acpi_evaluate_integer(_UID) status %d\n",
				ESIF_FUNC,
				status);
		} else {
			union acpi_object *acpi_obj = acpi_uid.pointer;
			pi_ptr->acpi_uid = acpi_obj->integer.value;
			ESIF_TRACE_DEBUG("%s: UID %llu\n",
					 ESIF_FUNC,
					 (u64)pi_ptr->acpi_uid);
			kfree(acpi_uid.pointer);
		}
	}

	/* ACPI Intel PTYP */
	if (pi_ptr->acpi_handle) {
		struct acpi_buffer acpi_type = {.length = ACPI_ALLOCATE_BUFFER};

		acpi_status status = acpi_evaluate_object(pi_ptr->acpi_handle,
							  "PTYP",
							  NULL,
							  &acpi_type);
		if (ACPI_FAILURE(status) || status == AE_NOT_FOUND) {
			ESIF_TRACE_DEBUG(
				"%s: acpi_evaluate_integer(PTYP) status %d\n",
				ESIF_FUNC,
				status);
		} else {
			union acpi_object *acpi_obj = acpi_type.pointer;
			pi_ptr->acpi_type = acpi_obj->integer.value;
			ESIF_TRACE_DEBUG("%s: PTYPE %llu\n",
					 ESIF_FUNC,
					 (u64)pi_ptr->acpi_type);
			kfree(acpi_type.pointer);
		}
	}

	/* APCI Intel PGMB For PCI MMIO BAR */
	if (pi_ptr->acpi_handle) {
		struct acpi_buffer acpi_pgmb = {.length = ACPI_ALLOCATE_BUFFER};
		acpi_status status = acpi_evaluate_object(pi_ptr->acpi_handle,
							  "PGMB",
							  NULL,
							  &acpi_pgmb);
		if (ACPI_FAILURE(status) || status == AE_NOT_FOUND) {
			ESIF_TRACE_DEBUG(
				"%s: acpi_evaluate_integer(PGMB) status %d\n",
				ESIF_FUNC,
				status);
		} else {
			/* TODO works for mem but we need a way to get this */
			u64 resource_len = 0x8000;
			union acpi_object *acpi_obj = acpi_pgmb.pointer;
			/* PGMB will return bits 38 To 15 only */
			u64 phy_addr = ((u64)acpi_obj->integer.value);

			ESIF_TRACE_DEBUG("%s: PGMB bits 35:18 %llx\n",
					 ESIF_FUNC,
					 phy_addr);

			phy_addr <<= 15;
			ESIF_TRACE_DEBUG("%s: PGMB phy_addr %llx\n",
					 ESIF_FUNC,
					 phy_addr);

			ESIF_TRACE_DEBUG("%s: resource_len %llu\n",
					 ESIF_FUNC,
					 resource_len);
			pi_ptr->mem_size = resource_len;
			pi_ptr->mem_base = ioremap_nocache(phy_addr,
							   resource_len);
			if (!pi_ptr->mem_base) {
				ESIF_TRACE_ERROR("%s: failed to map iomem\n",
						 ESIF_FUNC);
			}

			ESIF_TRACE_DEBUG("%s: mem_base %p\n",
					 ESIF_FUNC,
					 pi_ptr->mem_base);
			kfree(acpi_pgmb.pointer);
		}
	}

	rc = esif_lf_register_participant(pi_ptr);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Remove */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3, 8, 0)
static int acpi_remove(struct acpi_device *acpi_dev_ptr, int type)
#else
static int acpi_remove(struct acpi_device *acpi_dev_ptr)
#endif
{
	struct esif_participant_iface *pi_ptr = dev_get_drvdata(
			&acpi_dev_ptr->dev);

	enum esif_rc rc = esif_lf_unregister_participant(pi_ptr);

	kfree(pi_ptr);

	if (ESIF_OK != rc)
		return -EINVAL;
	return 0;
}


/* Event Notifier */
static void acpi_notify(
	struct acpi_device *acpi_dev_ptr,
	u32 event
	)
{
	struct esif_participant_iface *pi_ptr = dev_get_drvdata(
			&acpi_dev_ptr->dev);
	struct esif_data event_data = {
		ESIF_DATA_UINT32, &event, sizeof(event), sizeof(event)};

	ESIF_TRACE_DEBUG("%s: Non INT3400 devices, event %d\n", ESIF_FUNC,
			 event);

	acpi_send_event(pi_ptr, ESIF_EVENT_ACPI, 'NA', &event_data);
}


/* ACPI Driver */
static struct acpi_driver acpi_driver = {
	.name  = DRIVER_NAME,
	.class = "DptfAcpi",
	.ids   = esif_acpi_ids,
	.ops   = {
		.add    = acpi_add,
		.remove = acpi_remove,
		.notify = acpi_notify,
	},
};

#else

/*
** Platform Bus Driver.  Not Rentrant
*/

/* Probe */
static int acpi_platform_probe(struct platform_device *plat_dev_ptr)
{
	enum esif_rc rc        = ESIF_OK;
	char *kobj_path_ptr    = NULL;
	struct device *dev_ptr = &plat_dev_ptr->dev;

	ESIF_TRACE_DEBUG("%s: dev %p\n", ESIF_FUNC, plat_dev_ptr);
	pi_template.device = dev_ptr;
	dev_set_drvdata(dev_ptr, &pi_template);

	snprintf(pi_template.driver_name, ESIF_NAME_LEN, "%s.ko", DRIVER_NAME);
	snprintf(pi_template.device_name, ESIF_NAME_LEN, "%s",
		 dev_name(dev_ptr));

	kobj_path_ptr = kobject_get_path(&dev_ptr->kobj, GFP_ATOMIC);
	if (NULL != kobj_path_ptr) {
		snprintf(pi_template.device_path,
			 ESIF_PATH_LEN,
			 "%s",
			 kobj_path_ptr);
		kfree(kobj_path_ptr);
	}

	rc = esif_lf_register_participant(&pi_template);
	if (ESIF_OK != rc)
		return -EINVAL;
	return 0;
}


/* Remove */
static int acpi_platform_remove(struct platform_device *plat_dev_ptr)
{
	struct esif_participant_iface *pi_ptr = dev_get_drvdata(
			&plat_dev_ptr->dev);
	enum esif_rc rc = esif_lf_unregister_participant(pi_ptr);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, plat_dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;
	return 0;
}


/* Shutdown */
static void acpi_platform_shutdown(struct platform_device *plat_dev_ptr)
{
	struct esif_participant_iface *pi_ptr = dev_get_drvdata(
			&plat_dev_ptr->dev);
	enum esif_rc rc = acpi_send_event(pi_ptr,
					  ESIF_EVENT_PARTICIPANT_RESUME,
					  'NA',
					  NULL);

	ESIF_TRACE_DEBUG("%s dev %p rc = %d\n", ESIF_FUNC, plat_dev_ptr, rc);
	rc = rc;/* Remove Compiler Warning */
}


/* Suspend */
static int acpi_platform_suspend(
	struct platform_device *plat_dev_ptr,
	pm_message_t state
	)
{
	struct esif_participant_iface *pi_ptr = dev_get_drvdata(
			&plat_dev_ptr->dev);
	enum esif_rc rc = acpi_send_event(pi_ptr,
					  ESIF_EVENT_PARTICIPANT_SUSPEND,
					  'NA',
					  NULL);

	ESIF_TRACE_DEBUG("%s: dev %p %d\n", ESIF_FUNC, plat_dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;
	return 0;
}


/* Resume */
static int acpi_platform_resume(struct platform_device *plat_dev_ptr)
{
	struct esif_participant_iface *pi_ptr = dev_get_drvdata(
			&plat_dev_ptr->dev);
	enum esif_rc rc = acpi_send_event(pi_ptr,
					  ESIF_EVENT_PARTICIPANT_RESUME,
					  'NA',
					  NULL);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, plat_dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;
	return 0;
}


/* Platform Driver */
static struct platform_driver platform_driver = {
	.driver        = {
		.name  = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe    = acpi_platform_probe,
	.remove   = acpi_platform_remove,
	.shutdown = acpi_platform_shutdown,
	.suspend  = acpi_platform_suspend,
	.resume   = acpi_platform_resume
};

static struct platform_device *g_pd_ptr;

#endif


/* Load */
static int __init acpi_load(void)
{
	int rc = 0;

	ESIF_TRACE_INFO("%s: %s\n", ESIF_FUNC, ESIF_PARTICIPANT_ACPI_DESC);
	ESIF_TRACE_INFO("%s: %s\n", ESIF_FUNC, ESIF_COPYRIGHT);

#ifndef ESIF_ATTR_PLATFORM
	rc = acpi_bus_register_driver(&acpi_driver);
#else
	rc = platform_driver_register(&platform_driver);
	g_pd_ptr = platform_device_register_simple(DRIVER_NAME, 0, NULL, 0);
	if (NULL == g_pd_ptr)
		rc = -ENOMEM;
#endif
	return rc;
}


/* Unload */
static void __exit acpi_unload(void)
{
	ESIF_TRACE_INFO("%s: instance\n", ESIF_FUNC);

#ifndef ESIF_ATTR_PLATFORM
	acpi_bus_unregister_driver(&acpi_driver);
#else
	if (NULL != g_pd_ptr)
		platform_device_unregister(g_pd_ptr);

	platform_driver_unregister(&platform_driver);
#endif
}


/*
 ******************************************************************************
 * PUBLIC
 ******************************************************************************
 */

/* Module Entry / Exit */
module_init(acpi_load);
module_exit(acpi_unload);

MODULE_VERSION("X1.0.1.0");
MODULE_DESCRIPTION(ESIF_PARTICIPANT_ACPI_DESC);
MODULE_LICENSE(ESIF_LICENSE);
MODULE_AUTHOR(ESIF_AUTHOR);

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
