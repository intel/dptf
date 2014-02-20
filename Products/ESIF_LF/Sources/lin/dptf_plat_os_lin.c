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
#define DRIVER_NAME "dptf_plat"

#include "esif_lf.h"

/* Event Receive */
static enum esif_rc plat_recv_event(
	enum esif_event_type type,
	u16 domain,
	struct esif_data *data_ptr
	)
{
	ESIF_TRACE_DEBUG("%s: event received domain %d type %d\n",
			 ESIF_FUNC,
			 domain,
			 type);
	return ESIF_OK;
}


/* ESIF Particpant INTERFACE */
static struct esif_participant_iface pi = {
	.version     = ESIF_PARTICIPANT_VERSION,
	.class_guid  = ESIF_PARTICIPANT_PLAT_CLASS_GUID,
	.enumerator  = ESIF_PARTICIPANT_ENUM_PLAT,
	.flags       =                              0x0,
	.name        = ESIF_PARTICIPANT_PLAT_NAME,
	.desc        = ESIF_PARTICIPANT_PLAT_DESC,
	.driver_name = "?",	/* Filled In Dynamically By Driver */
	.device_name = "?",	/* Filled In Dynamically By Driver */
	.device_path = "NA",	/* Filled In Dynamically By Driver */
	.device      = NULL,	/* Driver Assigned                 */
	.mem_base    = NULL,	/* Driver Assigned                 */
	.mem_size    = 0,	/* Driver Assigned                 */
	.acpi_handle = NULL,	/* Driver Assigned                 */

	/* EVENT */
	.recv_event = plat_recv_event,
	.send_event = NULL,	/* Filled In Dynamically By esif_lf */

	/* ACPI */
	.acpi_device = "*",	/* Don't Care */
	.acpi_scope  = "*",	/* Don't Care */
	.acpi_uid    = ESIF_PARTICIPANT_INVALID_UID,	/* Don't Care */
	.acpi_type   = ESIF_PARTICIPANT_INVALID_TYPE,	/* Don't Care */

	/* PCI */
	.pci_vendor     = 0,	/* Not Used */
	.pci_device     = 0,	/* Not Used */
	.pci_bus        = 0,	/* Not Used */
	.pci_bus_device = 0,	/* Not Used */
	.pci_function   = 0,	/* Not Used */
	.pci_revision   = 0,	/* Not Used */
	.pci_class      = 0,	/* Not Used */
	.pci_sub_class  = 0,	/* Not Used */
	.pci_prog_if    = 0,	/* Not Used */
};

/*
 ******************************************************************************
 * LINUX Driver Code Below Here.  Implemetned As Platform Driver For
 * Prototype to avoid the need for a CRB.
 ******************************************************************************
 */

/*
 ******************************************************************************
 * PRIVATE
 ******************************************************************************
 */

/* Event Send */
static enum esif_rc acpi_send_event(
	enum esif_event_type type,
	u16 domain,
	void *data_ptr
	)
{
	if (NULL == pi.send_event)
		return ESIF_E_CALLBACK_IS_NULL;

	return pi.send_event(&pi, type, domain, data_ptr);
}


/* Probe */
static int platform_plat_probe(struct platform_device *dev_ptr)
{
	enum esif_rc rc = ESIF_OK;
	char *kobj_path_ptr = NULL;
	ESIF_TRACE_DEBUG("%s: dev %p\n", ESIF_FUNC, dev_ptr);

	/* Grab Linux Device Driver Not Platform Driver */
	pi.device = &dev_ptr->dev;

	snprintf(pi.driver_name, ESIF_NAME_LEN, "%s.ko", DRIVER_NAME);
	snprintf(pi.device_name,
		 ESIF_NAME_LEN,
		 "%s.%d",
		 dev_ptr->name,
		 dev_ptr->id);

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
static int platform_plat_remove(struct platform_device *dev_ptr)
{
	enum esif_rc rc;

	rc = esif_lf_unregister_participant(&pi);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Shutdown */
static void platform_plat_shutdown(struct platform_device *dev_ptr)
{
	enum esif_rc rc;

	rc = acpi_send_event(ESIF_EVENT_PARTICIPANT_SHUTDOWN, 'NA', NULL);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
}


/* Suspend */
static int platform_plat_suspend(
	struct platform_device *dev_ptr,
	pm_message_t state
	)
{
	enum esif_rc rc;

	rc = acpi_send_event(ESIF_EVENT_PARTICIPANT_SUSPEND, 'NA', &state);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Resume */
static int platform_plat_resume(struct platform_device *dev_ptr)
{
	enum esif_rc rc;

	rc = acpi_send_event(ESIF_EVENT_PARTICIPANT_RESUME, 'NA', NULL);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Platform Driver */
static struct platform_driver platform_plat_driver = {
	.driver        = {
		.name  = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe    = platform_plat_probe,
	.remove   = platform_plat_remove,
	.shutdown = platform_plat_shutdown,
	.suspend  = platform_plat_suspend,
	.resume   = platform_plat_resume
};

/* Platform Device */
static struct platform_device *g_pd_ptr;

/* Load */
static int __init dptf_plat_load(void)
{
	ESIF_TRACE_INFO("%s: %s\n", ESIF_FUNC, ESIF_PARTICIPANT_PLAT_DESC);
	ESIF_TRACE_INFO("%s: %s\n", ESIF_FUNC, ESIF_COPYRIGHT);
	ESIF_TRACE_INFO("%s: %s\n", ESIF_FUNC, ESIF_PLATFORM_MSG);

	platform_driver_register(&platform_plat_driver);

	/* Simulate Device Insert */
	/* platform_device_register(&platform_plat_device); */
	g_pd_ptr = platform_device_register_simple(DRIVER_NAME, 0, NULL, 0);
	if (NULL == g_pd_ptr)
		return -ENOMEM;

	return 0;
}


/* Unload */
static void __exit dptf_plat_unload(void)
{
	/* Simulate Device Removal */
	if (NULL != g_pd_ptr)
		platform_device_unregister(g_pd_ptr);

	platform_driver_unregister(&platform_plat_driver);
}


/*
 ******************************************************************************
 * PUBLIC
 ******************************************************************************
 */

/* Module Entry / Exit */
module_init(dptf_plat_load);
module_exit(dptf_plat_unload);

MODULE_VERSION("X1.0.1.0");
MODULE_DESCRIPTION(ESIF_PARTICIPANT_PLAT_DESC);
MODULE_LICENSE(ESIF_LICENSE);
MODULE_AUTHOR(ESIF_AUTHOR);

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

