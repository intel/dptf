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
#define DRIVER_NAME "dptf_cpu"

#include "esif_lf.h"
#include <linux/interrupt.h>


/* Event Receive */
static enum esif_rc cpu_recv_event(
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
	.class_guid  = ESIF_PARTICIPANT_CPU_CLASS_GUID,
#ifndef ESIF_ATTR_PLATFORM
	.enumerator  = ESIF_PARTICIPANT_ENUM_PCI,
#else /* ESIF_ATTR_PLATFORM */
	.enumerator  = ESIF_PARTICIPANT_ENUM_PLAT,
#endif
	.flags       =                             0x0,
	.name        = ESIF_PARTICIPANT_CPU_NAME,
	.desc        = ESIF_PARTICIPANT_CPU_DESC,
	.driver_name = "?",	/* Filled In Dynamically By Driver */
	.device_name = "?",	/* Filled In Dynamically By Driver */
	.device_path = "NA",	/* Filled In Dynamically By Driver */
	.device      = NULL,	/* Driver Assigned                 */
	.mem_base    = NULL,	/* Driver Assigned                 */
	.acpi_handle = NULL,	/* Driver Assigned                 */

	/* EVENT */
	.recv_event = cpu_recv_event,
	.send_event = NULL,	/* Filled In Dynamically By esif_lf */

	/* ACPI */
	.acpi_device = "NA",	/* NA For CPU Device             */
	.acpi_scope  = "?",	/* Override From ACPI Name Space */
	.acpi_uid    = ESIF_PARTICIPANT_INVALID_UID,	/* NA For CPU Device */
	.acpi_type   = ESIF_PARTICIPANT_INVALID_TYPE,	/* NA For CPU Device */

	/* PCI */
	.pci_vendor     = 0, /* Filled In *Dynamically By *Driver  */
	.pci_device     = 0, /* Filled In *Dynamically By *Driver  */
	.pci_bus        = 0, /* Hardware Bus                       */
	.pci_bus_device = 4, /* Hardware Device On Bus             */
	.pci_function   = 0, /* Hardware Function Of Device On Bus */
	.pci_revision   = 0, /* Filled In Dynamically By Driver    */
	.pci_class      = 0, /* Filled In Dynamically By Driver    */
	.pci_sub_class  = 0, /* Filled In Dynamically By Driver    */
	.pci_prog_if    = 0  /* Filled In Dynamically By Driver    */
};

/*
 ******************************************************************************
 * LINUX Driver Code Below Here.  Implemetned As Platform Driver For
 * Prototype to avoid the need for a CRB.
 ******************************************************************************
 * Two Versions Can Be Built
 * PCI - Requires Platform Hardware Support For Enum
 * Platform - Will Enum On Any Platform and Emulate HW
 */

/*
 *******************************************************************************
 * PRIVATE
 *******************************************************************************
 */

/* Event Send */
static enum
esif_rc cpu_send_event(
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

/* All 4-byte long, maybe identical accross CPUs */
#define THERMAL_CAMARILLO 0x5820 /* PACKAGE_THERMAL_CAMARILLO_INTERRUPT */
#define TEMP_TARGET       0x599c /* Thermal Monitor Ref Temp (Tjmax) in
				  * b[23:16] */
#define PKG_TEMP          0x5978 /* Package Temperature in b[7:0] */
#define PP0_TEMP          0x597c /* CPU Temperature in b[7:0] */
#define PP1_TEMP          0x5980 /* Gfx Temperature in b[7:0] */
#define THERMAL_CAMARILLO_STATUS 0x6200

/* esif_pci_cpu_ids pulled from autgen.h */
MODULE_DEVICE_TABLE(pci, esif_pci_cpu_ids);

/* Private data to keep */
struct cpu_priv_data {
	u32  camarillo;
	u32  tjmax;
};

static struct cpu_priv_data cpu_data;


static irqreturn_t pci_cpu_irq(
	int irq,
	void *devid
	)
{
	u32 val, temp;
	struct pci_dev *dev_ptr = devid;

	esif_ccb_mmio_read(pi.mem_base, PP0_TEMP, &temp);

	ESIF_TRACE_DEBUG("%s: CPU Interrupt caught, curr temp is %d!!!\n",
			 ESIF_FUNC,
			 temp);

	/*
	 * continuously sends unwarranted temperature threshold crossing
	 * rc = cpu_send_event(ESIF_EVENT_TYPE_TEMPERATURE_THRESHOLD, NULL);
	 */

	/*
	 * Camarillo status bit 7 and 9 indicate threshold 1 and 2 trip occured,
	 * once processed, these bits as well as PCI interrupt status register
	 * have to be cleared.
	 */
	esif_ccb_mmio_read(pi.mem_base, THERMAL_CAMARILLO_STATUS, &val);
	if (val & 0x00000280) {
		val &= 0xFFFFFD7F;
		esif_ccb_mmio_write(pi.mem_base, THERMAL_CAMARILLO_STATUS, val);
		pci_write_config_byte(dev_ptr, 0xdc, 0x01);
		/* pci_cpu_init_aux(dev_ptr); */
	}
	return IRQ_HANDLED;
}


/* Probe */
static int pci_cpu_probe(
	struct pci_dev *dev_ptr,
	const struct pci_device_id *dev_id_ptr
	)
{
	enum esif_rc rc     = ESIF_OK;
	char *kobj_path_ptr = NULL;
	int err = 0;
	int enabled         = 0;
	u64 resource_start;
	u64 resource_len;

	ESIF_TRACE_DEBUG("%s: dev %p pci_id %p pi %p\n",
			 ESIF_FUNC,
			 dev_ptr,
			 dev_id_ptr,
			 &pi);

	pci_set_drvdata(dev_ptr, &pi);

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

	err = request_irq(dev_ptr->irq,
			  pci_cpu_irq,
			  IRQF_SHARED,
			  DRIVER_NAME,
			  dev_ptr);

	if (err) {
		dev_err(&dev_ptr->dev, "fail to request irq %d\n", err);
		goto error_cleanup;
	}
	ESIF_TRACE_DEBUG("%s: request_irq returns %d\n", ESIF_FUNC, err);

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

	/* Future */
	switch (dev_ptr->device) {
	default:
		ESIF_TRACE_DEBUG("%s: %s cpu thermal device detected\n",
				 ESIF_FUNC, esif_device_str(dev_ptr->device));
		break;
	}

	/* To use ACPI _TMP method, BIOS has to be enabled as temp reporting via
	 * EC.
	 * ACPI PATC/PATx aren't available for CPU participant anyway.
	 */
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
	}

	/*
	 * Save orig camarillo and Tjmax before enabling thermal trip interrupt
	 * bits 15 & 23
	 */
	esif_ccb_mmio_read(pi.mem_base, THERMAL_CAMARILLO, &cpu_data.camarillo);
	writel((cpu_data.camarillo | 0x00808000U),
	       (pi.mem_base + THERMAL_CAMARILLO));

	esif_ccb_mmio_read(pi.mem_base, TEMP_TARGET, &cpu_data.tjmax);
	cpu_data.tjmax = (cpu_data.tjmax >> 16) & 0x000000FF;

	/* pci_cpu_init_aux(dev_ptr); */

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
static void pci_cpu_remove(struct pci_dev *dev_ptr)
{
	enum esif_rc rc = ESIF_OK;

	writel(cpu_data.camarillo, (pi.mem_base + THERMAL_CAMARILLO));
	rc = esif_lf_unregister_participant(&pi);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);

	free_irq(dev_ptr->irq, dev_ptr);
	iounmap(pi.mem_base);
	pci_release_region(dev_ptr, 0);
}


/* Shutdown */
static void pci_cpu_shutdown(struct pci_dev *dev_ptr)
{
	enum esif_rc rc;

	rc = cpu_send_event(ESIF_EVENT_PARTICIPANT_SHUTDOWN, 'NA', NULL);
	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
}


/* Suspend */
static int pci_cpu_suspend(
	struct pci_dev *dev_ptr,
	pm_message_t state
	)
{
	enum esif_rc rc;

	rc = cpu_send_event(ESIF_EVENT_PARTICIPANT_SUSPEND, 'NA', &state);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Resume */
static int pci_cpu_resume(struct pci_dev *dev_ptr)
{
	enum esif_rc rc;

	rc = cpu_send_event(ESIF_EVENT_PARTICIPANT_RESUME, 'NA', NULL);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


static struct pci_driver pci_cpu_driver = {
	.name     = DRIVER_NAME,
	.id_table = esif_pci_cpu_ids,
	.probe    = pci_cpu_probe,
	.remove   = pci_cpu_remove,
	.shutdown = pci_cpu_shutdown,
	.suspend  = pci_cpu_suspend,
	.resume   = pci_cpu_resume
};

#else /* ESIF_ATTR_PLATFORM */

/*
 * Platform Bus Driver
 */

/* Probe */
static int platform_cpu_probe(struct platform_device *dev_ptr)
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
static int platform_cpu_remove(struct platform_device *dev_ptr)
{
	enum esif_rc rc;

	rc = esif_lf_unregister_participant(&pi);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Shutdown */
static void platform_cpu_shutdown(struct platform_device *dev_ptr)
{
	enum esif_rc rc;

	rc = cpu_send_event(ESIF_EVENT_PARTICIPANT_RESUME, 'NA', NULL);
	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
}


/* Suspend */
static int platform_cpu_suspend(
	struct platform_device *dev_ptr,
	pm_message_t state
	)
{
	enum esif_rc rc;

	rc = cpu_send_event(ESIF_EVENT_PARTICIPANT_SUSPEND, 'NA', &state);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Resume */
static int platform_cpu_resume(struct platform_device *dev_ptr)
{
	enum esif_rc rc;

	rc = cpu_send_event(ESIF_EVENT_PARTICIPANT_RESUME, 'NA', NULL);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Platform Driver */
static struct platform_driver platform_cpu_driver = {
	.driver        = {
		.name  = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe    = platform_cpu_probe,
	.remove   = platform_cpu_remove,
	.shutdown = platform_cpu_shutdown,
	.suspend  = platform_cpu_suspend,
	.resume   = platform_cpu_resume
};

static struct platform_device *g_pd_ptr;

#endif

/* Load */
static int __init dptf_cpu_load(void)
{
	int rc = 0;

	ESIF_TRACE_INFO("%s: %s\n", ESIF_FUNC, ESIF_PARTICIPANT_CPU_DESC);
	ESIF_TRACE_INFO("%s: %s\n", ESIF_FUNC, ESIF_COPYRIGHT);

#ifndef ESIF_ATTR_PLATFORM
	rc = pci_register_driver(&pci_cpu_driver);
#else /* ESIF_ATTR_PLATFORM */
	ESIF_TRACE_INFO("%s: %s\n", ESIF_FUNC, ESIF_PLATFORM_MSG);

	rc = platform_driver_register(&platform_cpu_driver);

	/* Simulate Device Insert */
	g_pd_ptr = platform_device_register_simple(DRIVER_NAME, 0, NULL, 0);
	if (NULL == g_pd_ptr)
		rc = -ENOMEM;
#endif
	return rc;
}


/* Unload */
static void __exit dptf_cpu_unload(void)
{
	/* Simulate Device Removal */
#ifndef ESIF_ATTR_PLATFORM
	pci_unregister_driver(&pci_cpu_driver);
#else /* ESIF_ATTR_PLATFORM */
	if (NULL != g_pd_ptr)
		platform_device_unregister(g_pd_ptr);

	platform_driver_unregister(&platform_cpu_driver);
#endif
}


/*
 *******************************************************************************
 * PUBLIC
 *******************************************************************************
 */

/* Module Entry / Exit */
module_init(dptf_cpu_load);
module_exit(dptf_cpu_unload);

MODULE_VERSION("X1.0.1.0");
MODULE_DESCRIPTION(ESIF_PARTICIPANT_CPU_DESC);
MODULE_LICENSE(ESIF_LICENSE);
MODULE_AUTHOR(ESIF_AUTHOR);

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
