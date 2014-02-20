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

#include "esif_lf.h"
#include "esif_ipc.h"

/* Debug */
#define ESIF_DEBUG_MODULE ESIF_DEBUG_MOD_LINUX
#define DRIVER_NAME "esif_lf"

/* Event Receive */
static enum esif_rc dptf_recv_event(
	enum esif_event_type type,
	u16 domain,
	struct esif_data *data_ptr
	)
{
	ESIF_TRACE_DEBUG("%s: event domain %d received %d\n",
			 ESIF_FUNC,
			 domain,
			 type);
	return ESIF_OK;
}


/* ESIF Particpant INTERFACE */
static struct esif_participant_iface pi = {
	.version     = ESIF_PARTICIPANT_VERSION,
	.class_guid  = ESIF_PARTICIPANT_DPTF_CLASS_GUID,
#ifndef ESIF_ATTR_PLATFORM
	.enumerator  = ESIF_PARTICIPANT_ENUM_ACPI,
#else /* ESIF_ATTR_PLATFORM */
	.enumerator  = ESIF_PARTICIPANT_ENUM_PLAT,
#endif
	.flags       = ESIF_FLAG_DPTFZ,	/* DPTF Zone is Participant 0 */
	.name        = ESIF_PARTICIPANT_DPTF_NAME,
	.desc        = ESIF_PARTICIPANT_DPTF_DESC,
	.driver_name = "?",	/* Filled In Dynamically By Driver */
	.device_name = "?",	/* Filled In Dynamically By Driver */
	.device_path = "NA",	/* Filled In Dynamically By Driver */
	.device      = NULL,	/* Driver Assigned                 */
 	.mem_base    = NULL,    /* Driver Assigned                 */
        .mem_size    = 0,       /* Driver Assigned                 */
	.acpi_handle = NULL,	/* Driver Assigned                 */

	/* EVENT */
	.recv_event = dptf_recv_event,
	.send_event = NULL,	/* Filled In Dynamically By esif_lf */

	/* ACPI */
	.acpi_device = "?",	/* Override From ACPI Name Space  */
	.acpi_scope  = "?",	/* Override From ACPI Name Space  */
	.acpi_uid    = ESIF_PARTICIPANT_INVALID_UID,	/* Not Applicable
							 *                */
	.acpi_type   = ESIF_PARTICIPANT_INVALID_TYPE,	/* Not Applicable
							 *                */

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
 * Linux DPTF Class Device
 */

/* Relese DPTF Device */
static void dptf_release(struct device *dev_ptr) {}

/* DPTF Device Class */
static struct class dptf_class = {
	.name        = "dptf",
	.dev_release = dptf_release,
};

/*
** LINUX Instrument VIA SYSFS
*/

/* Instrument Cooling Device */

/* Get Max State */
static int dptf_get_max_state(
	struct thermal_cooling_device *cdev_ptr,
	unsigned long *state_ptr
	)
{
	*state_ptr = 100;
	return 0;
}


struct esif_data_binary_fst_package {
	union esif_data_variant  revision;
	union esif_data_variant  control;
	union esif_data_variant  speed;
};


/* Get Current State */
static int dptf_get_cur_state(
	struct thermal_cooling_device *cdev_ptr,
	unsigned long *state_ptr
	)
{
	enum esif_rc rc;
	u16 fst_size = sizeof(struct esif_data_binary_fst_package);
	struct esif_data_binary_fst_package fst_package;
	struct esif_lp_domain *lpd_ptr    =
		(struct esif_lp_domain *)cdev_ptr->devdata;
	struct esif_primitive_tuple tuple = {GET_FAN_STATUS, lpd_ptr->id, 255};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_BINARY, &fst_package, fst_size};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	if (ESIF_OK == rc) {
		*state_ptr = fst_package.control.integer.value;
		ESIF_TRACE_DEBUG(
			"linux_%s: Primitive GET_FAN_STATUS, result %ld\n",
			ESIF_FUNC, *state_ptr);
	} else {
		ESIF_TRACE_ERROR(
			"linux_%s: Primitive GET_FAN_STATUS, error rc %s(%d)\n",
			 ESIF_FUNC, esif_rc_str(rc), rc);
		*state_ptr = 0;
	}
	return 0;
}


/* Set Current Stte */
static int dptf_set_cur_state(
	struct thermal_cooling_device *cdev_ptr,
	unsigned long state
	)
{
	return 0;
}


/* Cooling Device Operations */
static struct thermal_cooling_device_ops dptf_cooling_ops = {
	.get_max_state = dptf_get_max_state,
	.get_cur_state = dptf_get_cur_state,
	.set_cur_state = dptf_set_cur_state,
};

/* Instrument Thermal Zone Sensor */

/* Get Temperature */
static int dptf_get_temp(
	struct thermal_zone_device *tzd_ptr,
	unsigned long *temp_ptr
	)
{
	struct esif_lp_domain *lpd_ptr    =
		(struct esif_lp_domain *)tzd_ptr->devdata;
	struct esif_primitive_tuple tuple = {GET_TEMPERATURE, lpd_ptr->id, 255};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_TEMPERATURE, temp_ptr,
					sizeof(*temp_ptr)};

	*temp_ptr = 0;

	esif_execute_primitive(lpd_ptr->lp_ptr,
			       &tuple,
			       &req_data,
			       &rsp_data,
			       NULL);

	if (*temp_ptr > 100)
		*temp_ptr = 100;/* Cap This So WE Don't Shutdown For Now */

	*temp_ptr = *temp_ptr * 1000;
	ESIF_TRACE_DEBUG("linux_%s: Primitive GET_TEMPERATURE, result %ld\n",
			 ESIF_FUNC, *temp_ptr);
	return 0;
}


/* Get Mode */
static int dptf_get_mode(
	struct thermal_zone_device *tzd_ptr,
	enum thermal_device_mode *mode_ptr
	)
{
	*mode_ptr = THERMAL_DEVICE_ENABLED;
	return 0;
}


/* Set Mode */
static int dptf_set_mode(
	struct thermal_zone_device *tzd_ptr,
	enum thermal_device_mode mode
	)
{
	return 0;
}


/* Get Trip Point Type */
static int dptf_get_trip_type(
	struct thermal_zone_device *tzd_ptr,
	int trip,
	enum thermal_trip_type *type_ptr
	)
{
	switch (trip) {
	/* 10 Active Trip Points */
	/*TODO: Fix This !!! Only MAP Five: Linux Assumption of MAX 12
	 * Trip Points
	 */
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		*type_ptr = THERMAL_TRIP_ACTIVE;
		break;

	/* 1 Passive Trip Point */
	case 5:
		*type_ptr = THERMAL_TRIP_PASSIVE;
		break;

	/* 1 Hot Trip Point */
	case 6:
		*type_ptr = THERMAL_TRIP_HOT;
		break;

	/* 1 Critical Trip Point */
	case 7:
		*type_ptr = THERMAL_TRIP_CRITICAL;
		break;

	default:
		*type_ptr = THERMAL_TRIP_ACTIVE; /* TODO: Handle EINVAL? */
	}
	return 0;
}


/* Get Trip Point Temp */
static int dptf_get_trip_temp(
	struct thermal_zone_device *tzd_ptr,
	int trip,
	unsigned long *temp_ptr
	)
{
	struct esif_lp_domain *lpd_ptr    =
		(struct esif_lp_domain *)tzd_ptr->devdata;
	struct esif_primitive_tuple tuple = {0, lpd_ptr->id, 255};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_TEMPERATURE, temp_ptr,
					sizeof(*temp_ptr)};

	switch (trip) {
	case 0:
		tuple.id = GET_TRIP_POINT_ACTIVE;
		tuple.instance = 0;
		break;

	case 1:
		tuple.id = GET_TRIP_POINT_ACTIVE;
		tuple.instance = 1;
		break;

	case 2:
		tuple.id = GET_TRIP_POINT_ACTIVE;
		tuple.instance = 2;
		break;

	case 3:
		tuple.id = GET_TRIP_POINT_ACTIVE;
		tuple.instance = 3;
		break;

	case 4:
		tuple.id = GET_TRIP_POINT_ACTIVE;
		tuple.instance = 4;
		break;

	case 5:
		tuple.id = GET_TRIP_POINT_PASSIVE;
		tuple.instance = 255;
		break;

	case 6:
		tuple.id = GET_TRIP_POINT_HOT;
		tuple.instance = 255;
		break;

	case 7:
		tuple.id = GET_TRIP_POINT_CRITICAL;
		tuple.instance = 255;
		break;

	default:
		return -EINVAL;
	}

	*temp_ptr = 0;
	esif_execute_primitive(lpd_ptr->lp_ptr,
			       &tuple,
			       &req_data,
			       &rsp_data,
			       NULL);

	/* Temporary To Keep Linux From Shuttind Down */
	if (7 == trip && *temp_ptr < 100)
		*temp_ptr = 105;

	*temp_ptr = *temp_ptr * 1000;
	ESIF_TRACE_DEBUG("linux_%s: Primitive %s, result type %d temp %ld\n",
			 ESIF_FUNC, esif_primitive_str(tuple.id),
			 rsp_data.type, *temp_ptr);
	return 0;
}


/* Get Critical Trip Point */
static inline int dptf_get_crit_temp(
	struct thermal_zone_device *tzd_ptr,
	unsigned long *temp_ptr
	)
{
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr    =
		(struct esif_lp_domain *)tzd_ptr->devdata;
	struct esif_primitive_tuple tuple = {
		GET_TRIP_POINT_CRITICAL, lpd_ptr->id, 255};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_TEMPERATURE, temp_ptr,
					sizeof(*temp_ptr)};

	*temp_ptr = 0;
	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				   &tuple,
				   &req_data,
				   &rsp_data,
				   NULL);

	*temp_ptr = *temp_ptr * 1000;
	ESIF_TRACE_DEBUG(
		"linux_%s: Primitive GET_TRIP_POINT_CRITICAL, temp %lu rc %d\n",
		ESIF_FUNC, *temp_ptr, rc);

	return 0;
}


/* thermal Zone Operations */
static struct thermal_zone_device_ops tzd_ops = {
	.get_temp      = dptf_get_temp,
	.get_mode      = dptf_get_mode,
	.set_mode      = dptf_set_mode,
	.get_trip_type = dptf_get_trip_type,
	.get_trip_temp = dptf_get_trip_temp,
	.get_crit_temp = dptf_get_crit_temp
};

/*
** DPTF SYSFS Device Class Handlers
*/

/* RAPL */
static ssize_t dptf_device_rapl_duty_cycle_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 duty_cycle = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_DUTY_CYCLE, lpd_ptr->id,
						255};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &duty_cycle,
					sizeof(duty_cycle)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_RAPL_DUTY_CYCLE, type %d duty_cycle %d rc %d\n",
		 ESIF_FUNC, rsp_data.type, duty_cycle, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", duty_cycle);
	else
		return 0;
}


static ssize_t dptf_device_rapl_turbo_priority_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 turbo_priority = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_DOMAIN_PRIORITY, lpd_ptr->id,
						255};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &turbo_priority,
					sizeof(turbo_priority)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_PROC_DOMAIN_PRIORITY, type %d turbo %d rc %d\n",
		ESIF_FUNC, rsp_data.type, turbo_priority, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", turbo_priority);
	else
		return 0;
}


static ssize_t dptf_device_rapl_power_default_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 power_default = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr    = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_POWER_DEFAULT,
						lpd_ptr->id, 255};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &power_default,
					sizeof(power_default)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_RAPL_POWER_DEFAULT, type %d pwr_def %d rc %d\n",
		 ESIF_FUNC, rsp_data.type, power_default, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", power_default);
	else
		return 0;
}


static ssize_t dptf_device_rapl_power_limit_lock_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 lock = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr    = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_POWER_LIMIT_LOCK,
						lpd_ptr->id, 255};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &lock, sizeof(lock)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_RAPL_POWER_LIMIT_LOCK, type %d lock %d rc %d\n",
		 ESIF_FUNC, rsp_data.type, lock, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", lock);
	else
		return 0;
}


static ssize_t dptf_device_rapl_power_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 power = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr    = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_POWER, lpd_ptr->id, 255};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_POWER, &power, sizeof(power)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: Primitive GET_RAPL_POWER, type %d power %d rc %d\n",
		 ESIF_FUNC, rsp_data.type, power, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", power);
	else
		return 0;
}


static ssize_t dptf_device_rapl_pl1_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 pl1 = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr    = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_POWER_LIMIT, lpd_ptr->id,
						1};
	struct esif_data req_data  = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data  = {ESIF_DATA_UINT32, &pl1, sizeof(pl1)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_RAPL_POWER_LIMIT_1, type %d RAPL PL1 %d rc %d\n",
		 ESIF_FUNC, rsp_data.type, pl1, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", pl1);
	else
		return 0;
}


static ssize_t dptf_device_rapl_pl2_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 pl2 = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr    = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_POWER_LIMIT,
						lpd_ptr->id, 2};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &pl2, sizeof(pl2)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_RAPL_POWER_LIMIT_2, type %d RAPL PL2 %d rc %d\n",
		 ESIF_FUNC, rsp_data.type, pl2, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", pl2);
	else
		return 0;
}


static ssize_t dptf_device_rapl_pl1_enable_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 pl1_enable = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr    = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_POWER_LIMIT_ENABLE,
						lpd_ptr->id, 1};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &pl1_enable,
					sizeof(pl1_enable)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_RAPL_POWER_LIMIT_1_ENABLE, type %d PL1 %d rc %d\n",
		 ESIF_FUNC, rsp_data.type, pl1_enable, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", pl1_enable);
	else
		return 0;
}


static ssize_t dptf_device_rapl_pl2_enable_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 pl2_enable = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_POWER_LIMIT_ENABLE,
						lpd_ptr->id, 2};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &pl2_enable,
					sizeof(pl2_enable)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_RAPL_POWER_LIMIT_2_ENABLE, type %d PL2 %d rc %d\n",
		 ESIF_FUNC, rsp_data.type, pl2_enable, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", pl2_enable);
	else
		return 0;
}


static ssize_t dptf_device_rapl_time_window_1_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 window_1 = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_TIME_WINDOW, lpd_ptr->id,
						 1};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &window_1,
					sizeof(window_1)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_RAPL_POWER_1_WINDOW, type %d window1 %u rc %d\n",
		 ESIF_FUNC, rsp_data.type, window_1, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", window_1);
	else
		return 0;
}


static ssize_t dptf_device_rapl_time_window_2_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 window_2 = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr    = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_TIME_WINDOW, lpd_ptr->id,
						 2};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &window_2,
					sizeof(window_2)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_RAPL_POWER_2_WINDOW, type %d window2 %u rc %d\n",
		 ESIF_FUNC, rsp_data.type, window_2, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", window_2);
	else
		return 0;
}


static ssize_t dptf_device_rapl_power_min_1_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 min_1 = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr    = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_POWER_MIN, lpd_ptr->id,
						1};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &min_1, sizeof(min_1)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_RAPL_POWER_1_MIN, type %d RAPL_pwr_min %u rc %d\n",
		 ESIF_FUNC, rsp_data.type, min_1, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", min_1);
	else
		return 0;
}


static ssize_t dptf_device_rapl_power_max_1_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 max_1 = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_POWER_MAX, lpd_ptr->id,
						1};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &max_1, sizeof(max_1)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_RAPL_POWER_1_MAX, type %d RAPL_pwr_max %u rc %d\n",
		 ESIF_FUNC, rsp_data.type, max_1, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", max_1);
	else
		return 0;
}


static ssize_t dptf_device_rapl_power_min_2_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 min_2 = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_POWER_MIN, lpd_ptr->id,
						 2};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &min_2, sizeof(min_2)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_RAPL_POWER_2_MIN, type %d RAPL_pwr_min2 %u rc %d\n",
		 ESIF_FUNC, rsp_data.type, min_2, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", min_2);
	else
		return 0;
}


static ssize_t dptf_device_rapl_power_max_2_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 max_2 = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_POWER_MAX, lpd_ptr->id,
						2};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &max_2, sizeof(max_2)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_RAPL_POWER_2_MAX, type %d RAPL_pwr_max2 %u rc %d\n",
		 ESIF_FUNC, rsp_data.type, max_2, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", max_2);
	else
		return 0;
}


static ssize_t dptf_device_rapl_power_max_time_window_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 max_time_window = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr    = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_POWER_MAX_TIME_WINDOW,
						lpd_ptr->id, 255};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &max_time_window,
					sizeof(max_time_window)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
	   "linux_%s: GET_RAPL_POWER_MAX_TIME_WINDOW, type %d val %u rc %d\n",
		 ESIF_FUNC, rsp_data.type, max_time_window, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", max_time_window);
	else
		return 0;
}


static ssize_t dptf_device_rapl_time_unit_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 unit = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr    = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_TIME_UNIT, lpd_ptr->id,
						255};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &unit, sizeof(unit)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_RAPL_TIME_UNIT, type %d rapl_time_unit %u rc %d\n",
		 ESIF_FUNC, rsp_data.type, unit, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", unit);
	else
		return 0;
}


static ssize_t dptf_device_rapl_energy_unit_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 unit = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr    = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_ENERGY_UNIT, lpd_ptr->id,
						 255};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &unit, sizeof(unit)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_RAPL_ENERGY_UNIT, type %d rapl_enrgy_unit %u rc %d\n",
		 ESIF_FUNC, rsp_data.type, unit, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", unit);
	else
		return 0;
}


static ssize_t dptf_device_rapl_energy_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 energy = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr    = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_ENERGY, lpd_ptr->id, 255};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &energy, sizeof(energy)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_RAPL_ENERGY, type %u rapl_energy %u rc %d\n",
		 ESIF_FUNC, rsp_data.type, energy, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%u\n", energy);
	else
		return 0;
}


static ssize_t dptf_device_rapl_power_unit_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 unit = 0;
	enum esif_rc rc;
	struct esif_lp_domain *lpd_ptr    = dev_get_platdata(dev_ptr);
	struct esif_primitive_tuple tuple = {GET_RAPL_POWER_UNIT, lpd_ptr->id,
						255};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &unit, sizeof(unit)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_RAPL_POWER_UNIT, type %d rapl_power %u rc %d\n",
		 ESIF_FUNC, rsp_data.type, unit, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", unit);
	else
		return 0;
}


/* Show Device Capability */
static ssize_t dptf_device_capability_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	struct esif_lp_domain *lpd_ptr = dev_get_platdata(dev_ptr);
	esif_flags_t capability = lpd_ptr->capabilities;
	ssize_t num = sprintf(buf_ptr, "Capabilities Bit Mask = 0x%08x\n",
			      capability);

	/* Decode */
	if (capability & ESIF_CAPABILITY_TEMP_STATUS)
		num = sprintf(buf_ptr, "%sREPORTS_TEMPERATURE\n", buf_ptr);

	if (capability & ESIF_CAPABILITY_POWER_STATUS)
		num = sprintf(buf_ptr, "%sREPORTS_POWER\n", buf_ptr);

	if (capability & ESIF_CAPABILITY_POWER_CONTROL)
		num = sprintf(buf_ptr, "%sPOWER_CONTROLS_AVAILABLE\n", buf_ptr);

	if (capability & ESIF_CAPABILITY_PERF_CONTROL) {
		num = sprintf(buf_ptr, "%sPERFORMANCE_CONTROLS_AVAILABLE\n",
			      buf_ptr);
	}

	if (capability & ESIF_CAPABILITY_ACTIVE_CONTROL) {
		num = sprintf(buf_ptr, "%sACTIVE_CONTROLS_AVAILABLE\n",
			      buf_ptr);
	}

	if (capability & ESIF_CAPABILITY_UTIL_STATUS)
		num = sprintf(buf_ptr, "%sREPORTS_UTILIZATION\n", buf_ptr);

	if (capability & ESIF_CAPABILITY_CORE_CONTROL)
		num = sprintf(buf_ptr, "%sCORE_CONTROLS_AVAILABLE\n", buf_ptr);

	if (capability & ESIF_CAPABILITY_CTDP_CONTROL) {
		num = sprintf(buf_ptr, "%sCONFIG_TDP_CONTROLS_AVAILABLE\n",
			      buf_ptr);
	}

	if (capability & ESIF_CAPABILITY_DOMAIN_PRIORITY)
		num = sprintf(buf_ptr, "%sREPORTS_DOMAIN_PRIORITY\n", buf_ptr);

	if (capability & ESIF_CAPABILITY_DISPLAY_CONTROL) {
		num = sprintf(buf_ptr, "%sDISPLAY_CONTROLS_AVAILABLE\n",
			       buf_ptr);
	}

	return num;
}


/* Show Device Description */
static ssize_t dptf_device_desc_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	struct esif_lp_domain *lpd_ptr = dev_get_platdata(dev_ptr);
	return sprintf(buf_ptr, "%s\n", lpd_ptr->desc_ptr);
}


/* Show Device Name */
static ssize_t dptf_device_name_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	struct esif_lp_domain *lpd_ptr = dev_get_platdata(dev_ptr);
	return sprintf(buf_ptr, "%s\n", lpd_ptr->name_ptr);
}


/* Show Device Mode */
static ssize_t dptf_device_mode_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	struct esif_lp_domain *lpd_ptr = dev_get_platdata(dev_ptr);
	return sprintf(buf_ptr,
		       "%s\n",
		       lpd_ptr->lp_ptr->enable == 1 ? "enabled" : "disabled");
}


/* Show Device Type */
static ssize_t dptf_device_type_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	struct esif_lp_domain *lpd_ptr = dev_get_platdata(dev_ptr);

	/* FORMAT: 69CB804A-A35B-11E1-94EB-3A316188709B */
	/* Reverse the bits as needed */
	return sprintf(buf_ptr, "%08X-%02X-%02X-%02X-%08X%02X\n",
		ntohl(*((u32 *)&lpd_ptr->lp_ptr->pi_ptr->class_guid[0])),
		ntohs(*((u16 *)&lpd_ptr->lp_ptr->pi_ptr->class_guid[4])),
		ntohs(*((u16 *)&lpd_ptr->lp_ptr->pi_ptr->class_guid[6])),
		ntohs(*((u16 *)&lpd_ptr->lp_ptr->pi_ptr->class_guid[8])),
		ntohl(*((u32 *)&lpd_ptr->lp_ptr->pi_ptr->class_guid[12])),
		ntohs(*((u16 *)&lpd_ptr->lp_ptr->pi_ptr->class_guid[14])));

}


/* Show Device Version */
static ssize_t dptf_device_version_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	struct esif_lp_domain *lpd_ptr = dev_get_platdata(dev_ptr);
	return sprintf(buf_ptr, "%d\n", lpd_ptr->lp_ptr->pi_ptr->version);
}


/*
 * These are thermal FS attribute extensions so we have to find our data
 */
#define to_thermal_zone(_dev) \
	container_of(_dev, struct thermal_zone_device, device)

/* Show Device Hysteresis */
static ssize_t dptf_device_hysteresis_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 hysteresis = 0;
	enum esif_rc rc;
	struct thermal_zone_device *tzd_ptr = to_thermal_zone(dev_ptr);
	struct esif_lp_domain *lpd_ptr      = tzd_ptr->devdata;
	struct esif_primitive_tuple tuple = {
			GET_TEMPERATURE_THRESHOLD_HYSTERESIS, lpd_ptr->id, 255};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &hysteresis,
					sizeof(hysteresis)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_TEMPERATURE_THRESHOLD_HYSTERESIS, ret %d, rc %d\n",
		ESIF_FUNC, hysteresis, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", hysteresis);
	else
		return 0;
}


/* Show Device Threshold 0 */
static ssize_t dptf_device_aux0_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 aux0 = 0;
	enum esif_rc rc;
	struct thermal_zone_device *tzd_ptr = to_thermal_zone(dev_ptr);
	struct esif_lp_domain *lpd_ptr      = tzd_ptr->devdata;
	struct esif_primitive_tuple tuple   = {GET_TEMPERATURE_THRESHOLDS,
						lpd_ptr->id, 0};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &aux0, sizeof(aux0)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_TEMPERATURE_THRESHOLD_AUX0, result %d, rc %d\n",
		ESIF_FUNC, aux0, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", aux0);
	else
		return 0;
}


/* Store Device Threshold 0 */
static ssize_t dptf_device_aux0_store(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	const char *buf_ptr,
	size_t count
	)
{
	u32 aux0;
	struct thermal_zone_device *tzd_ptr = to_thermal_zone(dev_ptr);
	struct esif_lp_domain *lpd_ptr      = tzd_ptr->devdata;
	struct esif_primitive_tuple tuple = {SET_TEMPERATURE_THRESHOLDS,
						lpd_ptr->id, 0};
	struct esif_data req_data = {ESIF_DATA_VOID, &aux0, sizeof(aux0)};
	struct esif_data rsp_data = {ESIF_DATA_VOID, NULL, 0};

	if (!sscanf(buf_ptr, "%d\n", &aux0))
		return -EINVAL;

	ESIF_TRACE_DEBUG("linux_%s: Primitive DPTF_SET_AUX0, value %d\n",
			 ESIF_FUNC, aux0);

	esif_execute_primitive(lpd_ptr->lp_ptr,
			       &tuple,
			       &req_data,
			       &rsp_data,
			       NULL);

	return count;
}


/* Show Device Threshold 1 */
static ssize_t dptf_device_aux1_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 aux1 = 0;
	enum esif_rc rc;
	struct thermal_zone_device *tzd_ptr = to_thermal_zone(dev_ptr);
	struct esif_lp_domain *lpd_ptr    = tzd_ptr->devdata;
	struct esif_primitive_tuple tuple = {GET_TEMPERATURE_THRESHOLDS,
						lpd_ptr->id, 1};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &aux1, sizeof(aux1)};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);

	ESIF_TRACE_DEBUG(
		"linux_%s: GET_TEMPERATURE_THRESHOLD_AUX1, result %d, rc %d\n",
		ESIF_FUNC, aux1, rc);

	if (ESIF_OK == rc)
		return sprintf(buf_ptr, "%d\n", aux1);
	else
		return 0;
}


/* Store Device Threshold 1 */
static ssize_t dptf_device_aux1_store(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	const char *buf_ptr,
	size_t count
	)
{
	u32 aux1;
	struct thermal_zone_device *tzd_ptr = to_thermal_zone(dev_ptr);
	struct esif_lp_domain *lpd_ptr    = tzd_ptr->devdata;
	struct esif_primitive_tuple tuple = {SET_TEMPERATURE_THRESHOLDS,
						lpd_ptr->id, 1};
	struct esif_data req_data = {ESIF_DATA_VOID, &aux1, sizeof(aux1)};
	struct esif_data rsp_data = {ESIF_DATA_VOID, NULL, 0};

	if (!sscanf(buf_ptr, "%d\n", &aux1))
		return -EINVAL;

	ESIF_TRACE_DEBUG("linux_%s: Primitive DPTF_SET_AUX1, value %d\n",
			 ESIF_FUNC,
			 aux1);

	esif_execute_primitive(lpd_ptr->lp_ptr,
			       &tuple,
			       &req_data,
			       &rsp_data,
			       NULL);

	return count;
}


/*
** Debug Attributes
*/

/* Store Device Event */
static ssize_t dptf_device_event_store(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	const char *buf_ptr,
	size_t count
	)
{
	struct esif_lp_domain *lpd_ptr = dev_get_platdata(dev_ptr);
	u32 event;

	if (!sscanf(buf_ptr, "%d\n", &event))
		return -EINVAL;

	lpd_ptr->lp_ptr->pi_ptr->send_event(lpd_ptr->lp_ptr->pi_ptr,
					    event,
					    lpd_ptr->id,
					    NULL);

	ESIF_TRACE_DEBUG("linux_%s: %d\n", ESIF_FUNC, event);
	return count;
}


/* Show Device DSP's */
static ssize_t dptf_device_state_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	struct esif_lp_domain *lpd_ptr  = dev_get_platdata(dev_ptr);
	enum esif_pm_participant_state state =
		esif_lf_pm_lp_get_state(lpd_ptr->lp_ptr);

	return sprintf(buf_ptr,
		       "State: %s(%d)\n",
		       esif_pm_participant_state_str(state), state);
}


/* Show Debug Modules */
static ssize_t dptf_debug_modules_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u32 modules;

	esif_debug_get_modules(&modules);
	return sprintf(buf_ptr, "0x%08X\n", modules);
}


/* Show Debug Module Level */
static ssize_t dptf_debug_module_level_show(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	char *buf_ptr
	)
{
	u8 i    = 0;
	u32 len = 0;
	u32 modules;
	u32 level;
	char *state;

	esif_debug_get_modules(&modules);

	for (i = 0; i < 32; i++) {
		if (modules & 0x1)
			state = "ENABLED";
		else
			state = "DISABLED";
		modules = modules >> 1;
		esif_debug_get_module_category(i, &level);
		len     = sprintf(buf_ptr,
				  "%smodule %02d: State: %8s Level: 0x%08X\n",
				  buf_ptr,
				  i,
				  state,
				  level);
	}
	return len;
}


/* Store Debug Modules */
static ssize_t dptf_debug_modules_store(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	const char *buf_ptr,
	size_t count
	)
{
	u32 modules;

	if (!sscanf(buf_ptr, "%x\n", &modules))
		return -EINVAL;

	esif_debug_set_modules(modules);
	return count;
}


/* Store Debug Module Level */
static ssize_t dptf_debug_module_level_store(
	struct device *dev_ptr,
	struct device_attribute *attr_ptr,
	const char *buf_ptr,
	size_t count
	)
{
	u32 module;
	u32 level;

	if (!sscanf(buf_ptr, "%x %x\n", &module, &level))
		return -EINVAL;

	esif_debug_set_module_category(module, level);

	return count;
}


/*
** Setup Attributes For Linux SYSFS
*/
#define RW S_IRWXUGO  
#define RO S_IRUGO
#define WO S_IWUGO

static DEVICE_ATTR(rapl_power, RO, dptf_device_rapl_power_show, NULL);
static DEVICE_ATTR(rapl_power_default,
		   RO,
		   dptf_device_rapl_power_default_show,
		   NULL);
static DEVICE_ATTR(rapl_power_limit_lock,
		   RO,
		   dptf_device_rapl_power_limit_lock_show,
		   NULL);
static DEVICE_ATTR(rapl_duty_cycle, RO, dptf_device_rapl_duty_cycle_show, NULL);
static DEVICE_ATTR(rapl_turbo_priority,
		   RO,
		   dptf_device_rapl_turbo_priority_show,
		   NULL);
static DEVICE_ATTR(rapl_pl1, RO, dptf_device_rapl_pl1_show, NULL);
static DEVICE_ATTR(rapl_pl2, RO, dptf_device_rapl_pl2_show, NULL);
static DEVICE_ATTR(rapl_pl1_enable, RO, dptf_device_rapl_pl1_enable_show, NULL);
static DEVICE_ATTR(rapl_pl2_enable, RO, dptf_device_rapl_pl2_enable_show, NULL);
static DEVICE_ATTR(rapl_time_window_1,
		   RO,
		   dptf_device_rapl_time_window_1_show,
		   NULL);
static DEVICE_ATTR(rapl_time_window_2,
		   RO,
		   dptf_device_rapl_time_window_2_show,
		   NULL);
static DEVICE_ATTR(rapl_power_min_1, RO, dptf_device_rapl_power_min_1_show,
		   NULL);
static DEVICE_ATTR(rapl_power_min_2, RO, dptf_device_rapl_power_min_2_show,
		   NULL);
static DEVICE_ATTR(rapl_power_max_1, RO, dptf_device_rapl_power_max_1_show,
		   NULL);
static DEVICE_ATTR(rapl_power_max_2, RO, dptf_device_rapl_power_max_2_show,
		   NULL);
static DEVICE_ATTR(rapl_power_max_time_window,
		   RO,
		   dptf_device_rapl_power_max_time_window_show,
		   NULL);
static DEVICE_ATTR(rapl_time_unit, RO, dptf_device_rapl_time_unit_show, NULL);
static DEVICE_ATTR(rapl_energy_unit, RO, dptf_device_rapl_energy_unit_show,
		   NULL);
static DEVICE_ATTR(rapl_energy, RO, dptf_device_rapl_energy_show, NULL);
static DEVICE_ATTR(rapl_power_unit, RO, dptf_device_rapl_power_unit_show, NULL);
static DEVICE_ATTR(capability, RO, dptf_device_capability_show, NULL);
static DEVICE_ATTR(desc, RO, dptf_device_desc_show, NULL);
static DEVICE_ATTR(name, RO, dptf_device_name_show, NULL);
static DEVICE_ATTR(hysteresis, RO, dptf_device_hysteresis_show, NULL);
static DEVICE_ATTR(mode, RO, dptf_device_mode_show, NULL);
static DEVICE_ATTR(programmable_threshold_0,
		   RW,
		   dptf_device_aux0_show,
		   dptf_device_aux0_store);
static DEVICE_ATTR(programmable_threshold_1,
		   RW,
		   dptf_device_aux1_show,
		   dptf_device_aux1_store);
static DEVICE_ATTR(type, RO, dptf_device_type_show, NULL);
static DEVICE_ATTR(version, RO, dptf_device_version_show, NULL);
static DEVICE_ATTR(state, RO, dptf_device_state_show, NULL);
static DEVICE_ATTR(_event, WO, NULL, dptf_device_event_store);
static DEVICE_ATTR(_debug_modules,
		   RW,
		   dptf_debug_modules_show,
		   dptf_debug_modules_store);
static DEVICE_ATTR(_debug_module_level,
		   RW,
		   dptf_debug_module_level_show,
		   dptf_debug_module_level_store);

/* Instrument Class Thermal */
enum esif_rc esif_lf_instrument_capability(struct esif_lp_domain *lpd_ptr)
{
	int rc = 0;
	/* Only Instrument Once */
	if (NULL != lpd_ptr->tzd_ptr || NULL != lpd_ptr->cdev_ptr)
		return ESIF_E_UNSPECIFIED;

	/* Provide Thermal FS? */
	if (lpd_ptr->capabilities & ESIF_CAPABILITY_TEMP_STATUS) {
		ESIF_TRACE_DEBUG("linux_%s: Providing Thermal Zone\n",
				 ESIF_FUNC);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0)
		lpd_ptr->tzd_ptr = thermal_zone_device_register(
					lpd_ptr->name_ptr,
					8, 0, lpd_ptr, &tzd_ops, 0, 0, 0);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0)
		lpd_ptr->tzd_ptr = thermal_zone_device_register(
					lpd_ptr->name_ptr,
					8, 0, lpd_ptr, &tzd_ops, 0, 0, 0, 0);
#else /* LINUX_VERSION_CODE */
		lpd_ptr->tzd_ptr = thermal_zone_device_register(
					lpd_ptr->name_ptr,
					8, lpd_ptr, &tzd_ops, 0, 0, 0, 0);
#endif

		if (IS_ERR(lpd_ptr->tzd_ptr))
			ESIF_TRACE_ERROR("%s: tzd_error\n", ESIF_FUNC);

		/* Enhance Thermal File Sysem For NOW */
		rc = sysfs_create_link(&lpd_ptr->tzd_ptr->device.kobj,
				  &lpd_ptr->device.kobj,
				  "device");

		if (rc)
			return ESIF_E_UNSPECIFIED;


		rc = sysfs_create_link(&lpd_ptr->device.kobj,
				  &lpd_ptr->tzd_ptr->device.kobj,
				  "tzd");

		if (rc)
			return ESIF_E_UNSPECIFIED;

		if (lpd_ptr->capabilities & ESIF_CAPABILITY_TEMP_STATUS) {
			device_create_file(&lpd_ptr->tzd_ptr->device,
					   &dev_attr_hysteresis);
			device_create_file(&lpd_ptr->tzd_ptr->device,
					   &dev_attr_programmable_threshold_0);
			device_create_file(&lpd_ptr->tzd_ptr->device,
					   &dev_attr_programmable_threshold_1);
		}
	}

	/* Provide Cooling Device ? */
	if (lpd_ptr->capabilities & ESIF_CAPABILITY_ACTIVE_CONTROL ||
	    lpd_ptr->capabilities & ESIF_CAPABILITY_PERF_CONTROL) {
		ESIF_TRACE_DEBUG("linux_%s: Provide Cooling Device\n",
				 ESIF_FUNC);

		lpd_ptr->cdev_ptr = thermal_cooling_device_register(
				lpd_ptr->name_ptr,
				lpd_ptr,
				&dptf_cooling_ops);
		if (IS_ERR(lpd_ptr->cdev_ptr))
			ESIF_TRACE_DEBUG("linux_%s: cdev_error\n", ESIF_FUNC);


		/* Enhance Cooling Device For NOW */
		rc = sysfs_create_link(&lpd_ptr->cdev_ptr->device.kobj,
				  &lpd_ptr->device.kobj,
				  "device");

		if (rc)
			return ESIF_E_UNSPECIFIED;

		rc = sysfs_create_link(&lpd_ptr->device.kobj,
				  &lpd_ptr->cdev_ptr->device.kobj,
				  "cdev");

		if (rc)
			return ESIF_E_UNSPECIFIED;
	}

	/* Provide RAPL ? */
	if (lpd_ptr->capabilities & ESIF_CAPABILITY_POWER_STATUS) {
		device_create_file(&lpd_ptr->device, &dev_attr_rapl_power);
		device_create_file(&lpd_ptr->device,
				   &dev_attr_rapl_power_default);
		device_create_file(&lpd_ptr->device, &dev_attr_rapl_duty_cycle);
		device_create_file(&lpd_ptr->device,
				   &dev_attr_rapl_turbo_priority);
		device_create_file(&lpd_ptr->device,
				   &dev_attr_rapl_power_limit_lock);
		device_create_file(&lpd_ptr->device, &dev_attr_rapl_pl1);
		device_create_file(&lpd_ptr->device, &dev_attr_rapl_pl2);
		device_create_file(&lpd_ptr->device, &dev_attr_rapl_pl1_enable);
		device_create_file(&lpd_ptr->device, &dev_attr_rapl_pl2_enable);
		device_create_file(&lpd_ptr->device,
				   &dev_attr_rapl_time_window_1);
		device_create_file(&lpd_ptr->device,
				   &dev_attr_rapl_time_window_2);
		device_create_file(&lpd_ptr->device,
				   &dev_attr_rapl_power_min_1);
		device_create_file(&lpd_ptr->device,
				   &dev_attr_rapl_power_min_2);
		device_create_file(&lpd_ptr->device,
				   &dev_attr_rapl_power_max_1);
		device_create_file(&lpd_ptr->device,
				   &dev_attr_rapl_power_max_2);
		device_create_file(&lpd_ptr->device,
				   &dev_attr_rapl_power_max_time_window);
		device_create_file(&lpd_ptr->device, &dev_attr_rapl_time_unit);
		device_create_file(&lpd_ptr->device,
				   &dev_attr_rapl_energy_unit);
		device_create_file(&lpd_ptr->device, &dev_attr_rapl_energy);
		device_create_file(&lpd_ptr->device, &dev_attr_rapl_power_unit);
	}
	return ESIF_OK;
}


/* Send All Events For UMDF Driver */
enum esif_rc esif_lf_send_all_events_in_queue_to_uf_by_ipc(void)
{
	/* Don't need it on Linux. LF exepcts it only in UMDF driver */
	return ESIF_E_NOT_IMPLEMENTED;
}


/* u16 To String */
static char *esif_domain_str(
	u16 domain,
	char *str_ptr
	)
{
	u8 *ptr = (u8 *)&domain;

	snprintf(str_ptr, 3, "%c%c", *ptr, *(ptr + 1));
	return str_ptr;
}


/* Instrument Domain */
static enum esif_rc esif_instrument_domain(struct esif_lp_domain *lpd_ptr)
{
	int rc      = 0;
	char str[8] = "";
	struct device *dev_ptr = NULL;

	if (NULL == lpd_ptr ||
	    NULL == lpd_ptr->lp_ptr ||
	    NULL == lpd_ptr->lp_ptr->pi_ptr ||
	    NULL == lpd_ptr->lp_ptr->pi_ptr->device)
		return ESIF_E_PARAMETER_IS_NULL;

	dev_ptr = lpd_ptr->lp_ptr->pi_ptr->device;

	ESIF_TRACE_DEBUG("linux_%s: lpq %p, dev %p\n",
			 ESIF_FUNC,
			 lpd_ptr,
			 dev_ptr);

	lpd_ptr->device.class         = &dptf_class;
	lpd_ptr->device.platform_data = lpd_ptr;

	/* Our instances are not zero based offset here for Linux */
	if (lpd_ptr->lp_ptr->pi_ptr->flags & ESIF_FLAG_DPTFZ) {
		dev_set_name(&lpd_ptr->device, "dptfz");
	} else {
		dev_set_name(&lpd_ptr->device,
			     "participant%d.%s",
			     lpd_ptr->lp_ptr->instance,
			     esif_domain_str(lpd_ptr->id, str));
	}

	rc = device_register(&lpd_ptr->device);
	if (rc)
		return ESIF_E_UNSPECIFIED;

	/* Create SYSFS Attributes / Files  */
	rc = device_create_file(&lpd_ptr->device, &dev_attr_capability);
	if (rc)
		return ESIF_E_UNSPECIFIED;

	rc = device_create_file(&lpd_ptr->device, &dev_attr_name);
	if (rc)
		return ESIF_E_UNSPECIFIED;

	rc = device_create_file(&lpd_ptr->device, &dev_attr_desc);
	if (rc)
		return ESIF_E_UNSPECIFIED;

	rc = device_create_file(&lpd_ptr->device, &dev_attr_mode);
	if (rc)
		return ESIF_E_UNSPECIFIED;

	rc = device_create_file(&lpd_ptr->device, &dev_attr_type);
	if (rc)
		return ESIF_E_UNSPECIFIED;

	rc = device_create_file(&lpd_ptr->device, &dev_attr_version);
	if (rc)
		return ESIF_E_UNSPECIFIED;

	rc = device_create_file(&lpd_ptr->device, &dev_attr_state);
	if (rc)
		return ESIF_E_UNSPECIFIED;

	rc = device_create_file(&lpd_ptr->device, &dev_attr__event);
	if (rc)
		return ESIF_E_UNSPECIFIED;

	/* Links */
	rc = sysfs_create_link(&lpd_ptr->device.kobj, &dev_ptr->kobj, "device");
	if (rc)
		return ESIF_E_UNSPECIFIED;

	return ESIF_OK;
}


/* Instrument Participant */
enum esif_rc esif_lf_instrument_participant(struct esif_lp *lp_ptr)
{
	u8 i;

	for (i = 0; i < lp_ptr->domain_count; i++)
		esif_instrument_domain(&lp_ptr->domains[i]);

	return ESIF_OK;
}


/* UNInstrument Thermal */
void esif_lf_uninstrument_capability(struct esif_lp_domain *lpd_ptr)
{
	ESIF_TRACE_DEBUG("linux_%s: buf %p\n", ESIF_FUNC, lpd_ptr);

	/* Unregister Thermal Zone If present */
	if (NULL != lpd_ptr->tzd_ptr) {
		sysfs_remove_link(&lpd_ptr->tzd_ptr->device.kobj, "device");
		sysfs_remove_link(&lpd_ptr->device.kobj, "tzd");

		if (lpd_ptr->capabilities & ESIF_CAPABILITY_TEMP_STATUS) {
			device_remove_file(&lpd_ptr->tzd_ptr->device,
					   &dev_attr_hysteresis);
			device_remove_file(&lpd_ptr->tzd_ptr->device,
					   &dev_attr_programmable_threshold_0);
			device_remove_file(&lpd_ptr->tzd_ptr->device,
					   &dev_attr_programmable_threshold_1);
		}
		thermal_zone_device_unregister(lpd_ptr->tzd_ptr);
		lpd_ptr->tzd_ptr = NULL;
	}

	/* Unregister Cooling Device If present */
	if (NULL != lpd_ptr->cdev_ptr) {
		sysfs_remove_link(&lpd_ptr->cdev_ptr->device.kobj, "device");
		sysfs_remove_link(&lpd_ptr->device.kobj, "cdev");
		thermal_cooling_device_unregister(lpd_ptr->cdev_ptr);
		lpd_ptr->cdev_ptr = NULL;
	}

	/* Unregister RAPL */
	if (lpd_ptr->capabilities & ESIF_CAPABILITY_POWER_STATUS) {
		device_remove_file(&lpd_ptr->device, &dev_attr_rapl_power);
		device_remove_file(&lpd_ptr->device,
				   &dev_attr_rapl_power_default);
		device_remove_file(&lpd_ptr->device,
				   &dev_attr_rapl_power_limit_lock);
		device_remove_file(&lpd_ptr->device, &dev_attr_rapl_duty_cycle);
		device_remove_file(&lpd_ptr->device,
				   &dev_attr_rapl_turbo_priority);
		device_remove_file(&lpd_ptr->device, &dev_attr_rapl_pl1);
		device_remove_file(&lpd_ptr->device, &dev_attr_rapl_pl2);
		device_remove_file(&lpd_ptr->device, &dev_attr_rapl_pl1_enable);
		device_remove_file(&lpd_ptr->device, &dev_attr_rapl_pl2_enable);
		device_remove_file(&lpd_ptr->device,
				   &dev_attr_rapl_time_window_1);
		device_remove_file(&lpd_ptr->device,
				   &dev_attr_rapl_time_window_2);
		device_remove_file(&lpd_ptr->device,
				   &dev_attr_rapl_power_min_1);
		device_remove_file(&lpd_ptr->device,
				   &dev_attr_rapl_power_min_2);
		device_remove_file(&lpd_ptr->device,
				   &dev_attr_rapl_power_max_1);
		device_remove_file(&lpd_ptr->device,
				   &dev_attr_rapl_power_max_2);
		device_remove_file(&lpd_ptr->device,
				   &dev_attr_rapl_power_max_time_window);
		device_remove_file(&lpd_ptr->device, &dev_attr_rapl_time_unit);
		device_remove_file(&lpd_ptr->device,
				   &dev_attr_rapl_energy_unit);
		device_remove_file(&lpd_ptr->device, &dev_attr_rapl_energy);
		device_remove_file(&lpd_ptr->device, &dev_attr_rapl_power_unit);
	}
}


/* UnInstrument domain */
static void esif_uninstrument_domain(struct esif_lp_domain *lpd_ptr)
{
	ESIF_TRACE_DEBUG("linux_%s: buf %p\n", ESIF_FUNC, lpd_ptr);

	device_remove_file(&lpd_ptr->device, &dev_attr_capability);
	device_remove_file(&lpd_ptr->device, &dev_attr_name);
	device_remove_file(&lpd_ptr->device, &dev_attr_desc);
	device_remove_file(&lpd_ptr->device, &dev_attr_mode);
	device_remove_file(&lpd_ptr->device, &dev_attr_type);
	device_remove_file(&lpd_ptr->device, &dev_attr_version);
	device_remove_file(&lpd_ptr->device, &dev_attr_state);
	device_remove_file(&lpd_ptr->device, &dev_attr__event);

	sysfs_remove_link(&lpd_ptr->device.kobj, "device");
	device_unregister(&lpd_ptr->device);

	/* Alow Reuse of Device */
	esif_ccb_memset(&lpd_ptr->device, 0, sizeof(struct device));
}


/* UnInstrument Qaulifier */
void esif_lf_uninstrument_participant(struct esif_lp *lp_ptr)
{
	u8 i;
	for (i = 0; i < lp_ptr->domain_count; i++)
		esif_uninstrument_domain(&lp_ptr->domains[i]);
}


/*
** Event Handling
*/

/* Send Event */
enum esif_rc esif_lf_send_event(
	const struct esif_participant_iface *pi_ptr,
	const struct esif_event *event_ptr
	)
{
	struct device *dev_ptr     = pi_ptr->device;
	struct kobj_uevent_env *ue = esif_ccb_malloc(sizeof(*ue));

	if (ESIF_EVENT_PARTICIPANT_CREATE == event_ptr->type) {
		add_uevent_var(ue,
			       "EVENT=ESIFv1 %d %d %d %d %d "
				"{TBD PARTICIPANT CREATE EVENT DATA}",
			       event_ptr->version,
			       event_ptr->type,
			       event_ptr->priority,
			       event_ptr->src,
			       event_ptr->dst);
	} else {
		add_uevent_var(ue, "EVENT=ESIFv1 %d %d %d %d %d {}",
			       event_ptr->version,
			       event_ptr->type,
			       event_ptr->priority,
			       event_ptr->src,
			       event_ptr->dst);
	}

	kobject_uevent_env(&dev_ptr->kobj, KOBJ_CHANGE, ue->envp);
	esif_ccb_free(ue);
	return ESIF_OK;
}


/*
 ******************************************************************************
 * LINUX Driver Code Below Here.  Implemetned As Platform Driver For
 * Prototype to avoid the need for a CRB.
 ******************************************************************************
 * Two Versions Can Be Built
 * ACPI - Requires Platform Hardware Support For Enum
 * Platform - Will Enum On Any Platform and Emulate HW
 */

/*
 ******************************************************************************
 * PRIVATE
 ******************************************************************************
 */

/* Event Send */
static enum
esif_rc dptf_send_event(
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
** ACPI Bus Driver
*/

/* ACPI Framework Device */
static const struct acpi_device_id acpi_device_ids[] = {
	{"INT3400",               0},	/*DPTF Zone */
	/* Must Be Null Terminated  */
	{"",                      0},
};
MODULE_DEVICE_TABLE(acpi, acpi_device_ids);


/* Add */
static int acpi_add(struct acpi_device *dev_ptr)
{
	enum esif_rc rc     = ESIF_OK;
	char *kobj_path_ptr = NULL;

	ESIF_TRACE_DEBUG("%s: acpi_dev %p\n", ESIF_FUNC, dev_ptr);

	pi.device      = &dev_ptr->dev;
	pi.acpi_handle = dev_ptr->handle;

	snprintf(pi.driver_name, ESIF_NAME_LEN, "%s.ko", DRIVER_NAME);
	snprintf(pi.device_name, ESIF_NAME_LEN, "%s", dev_name(&dev_ptr->dev));

	kobj_path_ptr = kobject_get_path(&dev_ptr->dev.kobj, GFP_ATOMIC);
	if (NULL != kobj_path_ptr) {
		snprintf(pi.device_path, ESIF_PATH_LEN, "%s", kobj_path_ptr);
		kfree(kobj_path_ptr);
	}

	/* Grab Our Scope */
	if (pi.acpi_handle) {
		struct acpi_buffer acpi_scope = {.length =
							ACPI_ALLOCATE_BUFFER};

		acpi_status status = acpi_get_name(pi.acpi_handle,
						   ACPI_FULL_PATHNAME,
						   &acpi_scope);
		if (ACPI_FAILURE(status)) {
			ESIF_TRACE_DEBUG("%s: acpi_get_name error %d\n",
					 ESIF_FUNC,
					 status);
		} else {
			strncpy(pi.acpi_scope,
				(char *)acpi_scope.pointer,
				ESIF_SCOPE_LEN);
			ESIF_TRACE_DEBUG("%s: scope = %s\n",
					 ESIF_FUNC,
					 pi.acpi_scope);
			kfree(acpi_scope.pointer);
		}
	}
	/* Now the HID */
	strncpy(pi.acpi_device, acpi_device_hid(dev_ptr), ESIF_NAME_LEN);
	ESIF_TRACE_DEBUG("%s: _HID = %s\n", ESIF_FUNC, pi.acpi_device);

	/* USE Bid Until We Can Get UNICODE String Support */
	strncpy(pi.name, acpi_device_bid(dev_ptr), ESIF_NAME_LEN);
	ESIF_TRACE_DEBUG("%s: temp name = %s\n", ESIF_FUNC, pi.name);

	device_create_file(&dev_ptr->dev, &dev_attr__debug_modules);
	device_create_file(&dev_ptr->dev, &dev_attr__debug_module_level);

	rc = esif_lf_register_participant(&pi);
	rc = esif_ipc_init(pi.device);

	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Remove */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3, 8, 0)
static int acpi_remove(struct acpi_device *dev_ptr, int type)
#else
static int acpi_remove(struct acpi_device *dev_ptr)
#endif
{
	enum esif_rc rc = ESIF_OK;
	ESIF_TRACE_DEBUG("%s: acpi_dev %p\n", ESIF_FUNC, dev_ptr);

	device_remove_file(&dev_ptr->dev, &dev_attr__debug_modules);
	device_remove_file(&dev_ptr->dev, &dev_attr__debug_module_level);

	esif_ipc_exit(pi.device);
	rc = esif_lf_unregister_participant(&pi);

	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Notify
 * Expected ACPI event for this INT3400 device
 *  ACPI_NOTIFICATION_THERMAL_RELATIONSHIP_CHANGED 0x83 // used by passive
 *     ACPI_NOTIFICATION_ACTIVE_RELATIONSHIP_CHANGED  0x84 // used by active
 *     ACPI_NOTIFICATION_LPM_MODE_CHANGE_EVENT        0x85 // used by lpm policy
 */
static void acpi_notify(
	struct acpi_device *dev_ptr,
	u32 event
	)
{
	struct esif_data event_data = {ESIF_DATA_UINT32, &event, sizeof(event),
					 sizeof(event)};

	ESIF_TRACE_DEBUG("%s: acpi_dev %p (INT3400), event %d\n",
			 ESIF_FUNC,
			 dev_ptr,
			 event);

	dptf_send_event(ESIF_EVENT_ACPI, 'NA', &event_data);
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0)
/* Suspend */
static int acpi_suspend(
	struct acpi_device *dev_ptr,
	pm_message_t state
	)
{
	enum esif_rc rc = dptf_send_event(ESIF_EVENT_PARTICIPANT_SUSPEND,
					'NA', &state);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;
	return 0;
}


/* Resume */
static int acpi_resume(struct acpi_device *dev_ptr)
{
	enum esif_rc rc = dptf_send_event(ESIF_EVENT_PARTICIPANT_RESUME,
					  'NA',
					  NULL);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;
	return 0;
}


#endif

/* ACPI Driver */
static struct acpi_driver dptf_acpi_driver = {
	.name  = DRIVER_NAME,
	.class = "dptf",
	.ids   = acpi_device_ids,
	.ops   = {
		.add     = acpi_add,
		.remove  = acpi_remove,
		.notify  = acpi_notify,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0)
		.suspend = acpi_suspend,
		.resume  = acpi_resume,
#endif
	}
};

#else /* ESIF_ATTR_PLATFORM */

/* Probe */
static int dptf_platform_probe(struct platform_device *dev_ptr)
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

	device_create_file(&dev_ptr->dev, &dev_attr__debug_modules);
	device_create_file(&dev_ptr->dev, &dev_attr__debug_module_level);

	rc = esif_lf_register_participant(&pi);
	rc = esif_ipc_init(pi.device);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Remove */
static int dptf_platform_remove(struct platform_device *dev_ptr)
{
	enum esif_rc rc = ESIF_OK;

	esif_ipc_exit(pi.device);
	rc = esif_lf_unregister_participant(&pi);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Shudown */
static void dptf_platform_shutdown(struct platform_device *dev_ptr)
{
	enum esif_rc rc = dptf_send_event(ESIF_EVENT_PARTICIPANT_RESUME,
					  'NA',
					  NULL);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
	rc = rc; /* Remove Compiler Warning */
}


/* Suspend */
static int dptf_platform_suspend(
	struct platform_device *dev_ptr,
	pm_message_t state
	)
{
	enum esif_rc rc = dptf_send_event(ESIF_EVENT_PARTICIPANT_SUSPEND,
					  'NA',
					  &state);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev_ptr, rc);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Resume */
static int dptf_platform_resume(struct platform_device *dev_ptr)
{
	enum esif_rc rc = dptf_send_event(ESIF_EVENT_PARTICIPANT_RESUME,
					  'NA',
					  NULL);

	ESIF_TRACE_DEBUG("%s: dev %p rc = %d\n", ESIF_FUNC, dev, rc);
	if (ESIF_OK != rc)
		return -EINVAL;

	return 0;
}


/* Platform Driver */
static struct platform_driver dptf_platform_driver = {
	.driver        = {
		.name  = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe    = dptf_platform_probe,
	.remove   = dptf_platform_remove,
	.shutdown = dptf_platform_shutdown,
	.suspend  = dptf_platform_suspend,
	.resume   = dptf_platform_resume
};

static struct platform_device *g_pd_ptr;

#endif

static int debug;
extern int g_background; /* = 10000; 10 Seconds */

/* Load */
static int __init esif_lf_load(void)
{
	int rc = 0;

	ESIF_TRACE_INFO("%s: %s, Ver: %s\n",
			ESIF_FUNC,
			ESIF_LOWER_FRAMEWORK,
			ESIF_VERSION);
	ESIF_TRACE_INFO("%s: %s %s\n", ESIF_FUNC, ESIF_COPYRIGHT, ESIF_AUTHOR);
	ESIF_TRACE_INFO("%s: OS: %s Debug Mask: %08x Background Timer %d\n",
			ESIF_FUNC, ESIF_ATTR_OS, debug, g_background);

	class_register(&dptf_class);
	esif_lf_init(debug);

#ifndef ESIF_ATTR_PLATFORM
	rc = acpi_bus_register_driver(&dptf_acpi_driver);
#else /* ESIF_ATTR_PLATFORM */
	ESIF_TRACE_INFO("%s: %s\n", ESIF_FUNC, ESIF_PLATFORM_MSG);
	rc  = platform_driver_register(&dptf_platform_driver);
	/* Simulate Device Insert */
	g_pd_ptr = platform_device_register_simple(DRIVER_NAME, 0, NULL, 0);
	if (NULL == g_pd_ptr)
		rc = -ENOMEM;
#endif
	ESIF_TRACE_INFO(
		"linux_%s: Export Symbol esif_lf_register_participant\n",
		ESIF_FUNC);
	ESIF_TRACE_INFO(
		"linux_%s: Export Symbol esif_lf_unregister_participant\n",
		ESIF_FUNC);
	return rc;
}


/* Unload */
static void __exit esif_lf_unload(void)
{
	ESIF_TRACE_INFO("%s: entered, dptf_class %p\n", ESIF_FUNC, &dptf_class);

#ifndef ESIF_ATTR_PLATFORM
	acpi_bus_unregister_driver(&dptf_acpi_driver);
#else /* ESIF_ATTR_PLATFROM */
	/* Simulate Device Removal */
	if (NULL != g_pd_ptr)
		platform_device_unregister(g_pd_ptr);
	platform_driver_unregister(&dptf_platform_driver);
#endif
	esif_lf_exit();
	class_unregister(&dptf_class);
}


/*
 ******************************************************************************
 * PUBLIC
 ******************************************************************************
 */

/* Expose Linux Symbols * / */
EXPORT_SYMBOL_GPL(esif_lf_unregister_participant);
EXPORT_SYMBOL_GPL(esif_lf_register_participant);

/* Module Entry / Exit */
module_init(esif_lf_load);
module_exit(esif_lf_unload);

module_param(debug, int, S_IRUGO);
module_param(g_background, int, S_IRUGO);

MODULE_DESCRIPTION("ESIF Lower Framework");
MODULE_VERSION("X1.0.1.0");
MODULE_LICENSE(ESIF_LICENSE);
MODULE_AUTHOR(ESIF_AUTHOR);

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
