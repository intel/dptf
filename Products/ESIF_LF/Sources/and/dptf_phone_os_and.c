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

#include "../esif.h"		/* Eco-System Independent Framework */
#include "../esif_data.h"	/* ESIF Data Buffer                 */
#include "../esif_primitive.h"	/* ESIF Primitive                   */
#include "../esif_event.h"	/* ESIF Event Routines              */
#include "../esif_participant.h"/* ESIF Participant                 */
#include "../esif_lf.h"		/* ESIF Lower Framework             */

#define PHONE_DEBUG ESIF_TRACE_DEBUG
#define DRIVER_NAME "dptf_phone"
#define ESIF_DEBUG_MODULE ESIF_DEBUG_MOD_GENERAL

/* Event Receive */
static esif_rc_t phone_recv_event(
	enum esif_event_type type,
	void *data
	)
{
	PHONE_DEBUG("%s: event received %d\n", __func__, type);
	return ESIF_OK;
}


/* ESIF Particpant Info */
static struct esif_participant_info pi = {
	.version    = ESIF_PARTICIPANT_VERSION,
	.class_guid =
	{0x69,
	 0xCB,
	 0x80,
	 0x4A,
	 0xA3, 0x5B,
	 0x11,
	 0xE1,
	 0x94,
	 0xEB,
	 0x3A,
	 0x31,
	 0x61,
	 0x88, 0x70,
	 0x9B},
	.enumerator  = ESIF_PARTICIPANT_ENUM_NONE,
	.flags       =
		0x0,
	.capability  = ESIF_CAPABILITY_TEMP_SENSOR |
		ESIF_CAPABILITY_PROG_SENSOR,
	.name        = ESIF_PARTICIPANT_PHONE,
	.driver_name = "?",	/* Filled In Dynamically By Driver  */
	.device_name = "?",	/* Filled In Dynamically By Driver  */
	.driver      = NULL,	/* Driver Assigned                  */
	.mem_base    = NULL,	/* Driver Assigned                  */
	.acpi_handle = NULL,	/* Driver Assigned                  */

	/* EVENT */
	.recv_event = phone_recv_event,
	.send_event = NULL,	/* Filled In Dynamically By esif_lf */

	/* ACPI */
	.acpi_device = "*",	/* Don't Care */
	.acpi_scope  = "*",	/* Don't Care */
	.acpi_type   = "*",	/* Don't Care */

	/* PCI */
	.pci_vendor     = 0,	/* Not Used */
	.pci_device     = 0,	/* Not Used */
	.pci_bus        = 0,	/* Not Used */
	.pci_bus_device = 0,	/* Not Used */
	.pci_function   = 0,	/* Not Used */
};

/*
 *******************************************************************************
 ** LINUX Driver Code Below Here.  Implemetned As Platform Driver For
 ** Prototype to avoid the need for a CRB.
 *******************************************************************************
 */

/* Event Send */
static esif_rc_t phone_send_event(
	enum esif_event_type type,
	void *data
	)
{
	if (NULL != pi.send_event) {
		pi.send_event(&pi, type, data);
	}
	return ESIF_OK;
}


static int platform_phone_probe(struct platform_device *dev)
{
	PHONE_DEBUG("%s: buf %p\n", __func__, dev);

	/* Grab Linux Device Driver Not Platform Driver */
	pi.driver = &dev->dev;
	snprintf(pi.driver_name, ESIF_NAME_LEN, "%s.ko", DRIVER_NAME);
	snprintf(pi.device_name, ESIF_NAME_LEN, "%s.%d", dev->name, dev->id);

	esif_lf_register_participant(&pi);
	return 0;
}


static int platform_phone_remove(struct platform_device *dev)
{
	PHONE_DEBUG("%s: buf %p\n", __func__, dev);
	esif_lf_unregister_participant(&pi);
	return 0;
}


static void platform_phone_shutdown(struct platform_device *dev)
{
	PHONE_DEBUG("%s: buf %p\n", __func__, dev);
	phone_send_event(ESIF_EVENT_TYPE_SHUTDOWN_UF_PARTICIPANT, NULL);
}


static int platform_phone_suspend(
	struct platform_device *dev,
	pm_message_t state
	)
{
	PHONE_DEBUG("%s: buf %p\n", __func__, dev);
	phone_send_event(ESIF_EVENT_TYPE_SUSPEND_UF_PARTICIPANT, &state);
	return 0;
}


static int platform_phone_resume(struct platform_device *dev)
{
	PHONE_DEBUG("%s: buf %p\n", __func__, dev);
	phone_send_event(ESIF_EVENT_TYPE_RESUME_UF_PARTICIPANT, NULL);
	return 0;
}


/* Platform Driver */
static struct platform_driver platform_phone_driver = {
	.driver        = {
		.name  = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe    = platform_phone_probe,
	.remove   = platform_phone_remove,
	.shutdown = platform_phone_shutdown,
	.suspend  = platform_phone_suspend,
	.resume   = platform_phone_resume
};

/* Keep Linux Happy */
static void platform_phone_device_release(struct device *dev)
{}

/* Platform Device */
static struct platform_device platform_phone_device =
{
	.name        = DRIVER_NAME,
	.dev.release = platform_phone_device_release
};

/* Load */
static int __init dptf_phone_load(void)
{
	ESIF_TRACE_INFO("%s: %s\n", __func__, ESIF_PARTICIPANT_PHONE);
	ESIF_TRACE_INFO("%s: %s\n", __func__, ESIF_COPYRIGHT);
	platform_driver_register(&platform_phone_driver);
	/* Simulate Device Insert */
	platform_device_register(&platform_phone_device);
	return 0;
}


/* Unload */
static void __exit dptf_phone_unload(void)
{
	/* Simulate Device Removal */
	platform_device_unregister(&platform_phone_device);
	platform_driver_unregister(&platform_phone_driver);
}


/* Module Entry / Exit */
module_init(dptf_phone_load);
module_exit(dptf_phone_unload);

MODULE_VERSION("X1.0.0.2");
MODULE_DESCRIPTION(ESIF_PARTICIPANT_PHONE);
MODULE_LICENSE(ESIF_LICENSE);
MODULE_AUTHOR(ESIF_AUTHOR);
