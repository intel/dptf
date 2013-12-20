/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_UF_FPC_
#define _ESIF_UF_FPC_

#include "esif_primitive.h"


/* Must Be Aligned With DSP's dsp_descriptor.h: dsp_descriptor{} */
#define MAX_CODE_NAME_STRING_LENGTH 13
#define MAX_NAME_STRING_LENGTH 32
#define MAX_DESCRIPTION_STRING_LENGTH 128
#define MAX_TYPE_STRING_LENGTH 64
#define MAX_ACPI_DEVICE_STRING_LENGTH 32
#define MAX_ACPI_UID_STRING_LENGTH 32
#define MAX_ACPI_TYPE_STRING_LENGTH 32
#define MAX_ACPI_SCOPE_STRING_LENGTH 32
#define MAX_VENDOR_ID_STRING_LENGTH 32
#define MAX_DEVICE_ID_STRING_LENGTH 32
#define MAX_REVISION_STRING_LENGTH 16
#define MAX_CLASS_STRING_LENGTH 16
#define MAX_SUBCLASS_STRING_LENGTH 16
#define MAX_PROGIF_STRING_LENGTH 16
#define MAX_BUS_STRING_LENGTH 8
#define MAX_DEVICE_STRING_LENGTH 8
#define MAX_FUNCTION_STRING_LENGTH 8

/*
 * FPC Binary File Format
 *   { DSP Package {size, pkg_descriptor, number_of_domains, number_of_algorithms},
 *     Domain      {name, domain_descriptor, number_of_primitives, capability_for_domain},
 *     Primitive   {size, <type, qualifier, instance>, operation, req_type, req_schema[],
 *                  res_type, res_schema[], number_actions},
 *     Action(s)   {size, type, priority, is_kernel, param_valid[5], param_offset[5]},
 *                 p1, p2, p3, p4, p5,
 *
 *               < ... more actions ... >
 *
 *           < ... more primitive ... >
 *
 *               < ... more actions ... >
 *
 *     < ... more domains ... >
 *
 *           < ... more primtive ... >
 *
 *               < ... more actions ... >
 *
 *
 *     Algorithm   {action_type, temp_xform, tempC1, tempC2, power_xform, time_xform, size}
 *
 *     < ... more algorithms ... >
 *
 */

/*--- FPC ---*/
/* Must Be Aligned With DSP's dsp_descriptor.h: dsp_descriptor {} */
struct esif_fpc_descriptor {
	char  code[MAX_CODE_NAME_STRING_LENGTH];
	unsigned char  ver_major;
	unsigned char  ver_minor;
	char  name[MAX_NAME_STRING_LENGTH];
	char  description[MAX_DESCRIPTION_STRING_LENGTH];
	char  type[MAX_TYPE_STRING_LENGTH];
	char  acpi_device[MAX_ACPI_DEVICE_STRING_LENGTH];
	char  acpi_UID[MAX_ACPI_UID_STRING_LENGTH];
	char  acpi_type[MAX_ACPI_TYPE_STRING_LENGTH];
	char  acpi_scope[MAX_ACPI_SCOPE_STRING_LENGTH];
	char  pci_vendor_id[MAX_VENDOR_ID_STRING_LENGTH];
	char  pci_device_id[MAX_DEVICE_ID_STRING_LENGTH];
	unsigned int bus_enum;
	char  pci_bus[MAX_BUS_STRING_LENGTH];
	char  pci_device[MAX_DEVICE_STRING_LENGTH];
	char  pci_function[MAX_FUNCTION_STRING_LENGTH];
	char  pci_revision[MAX_REVISION_STRING_LENGTH];
	char  pci_class[MAX_CLASS_STRING_LENGTH];
	char  pci_subClass[MAX_SUBCLASS_STRING_LENGTH];
	char  pci_progIf[MAX_PROGIF_STRING_LENGTH];
};

/* Must Be Aligned With DSP's compact_dsp.h: compact_dsp{} */
struct esif_fpc {
	unsigned int  size;
	struct esif_fpc_descriptor header;
	unsigned int  number_of_domains;
	unsigned int  number_of_algorithms;
	unsigned int  number_of_events;
	// domains are laid out beyond this point
	// algorithms come after the domains
	// TBD: remove OLD: unsigned int number_of_basic_primitives;
	// events are after algorithms
};

#define MAX_NAME_STRING_LENGTH 32
#define MAX_DOMAIN_TYPE_STRING_LENGTH2 16
#define MAX_DESCRIPTION_STRING_LENGTH 128

/* Must Be Aligned With DSP's domain.h: domain_descriptor{} */
struct esif_domain_descriptor {
	char  name[MAX_NAME_STRING_LENGTH];
	char  description[MAX_DESCRIPTION_STRING_LENGTH];
	unsigned short domain;
	char  guid[MAX_DOMAIN_TYPE_STRING_LENGTH2];
	unsigned int  priority;
	enum esif_domain_type domainType;
};

#define MAX_MASK_SIZE 32

/* Must Be Aligned With DSP's capability.h: capability{} */
struct esif_domain_capability {
	unsigned int   capability_flags;
	unsigned char  number_of_capability_flags;
	unsigned char  reserved[3];
	unsigned char  capability_mask[MAX_MASK_SIZE];
};


/* Must Be Aligned With DSP's domain.h: domain{} */
struct esif_fpc_domain {
	unsigned int  size;
	struct esif_domain_descriptor descriptor;
	unsigned int  number_of_primitives;
	struct esif_domain_capability  capability_for_domain;
	// primitives are laid out beyond this point
};

/* Must Be Aligned With DSP's algorithm.h: algorithm{} */
struct esif_fpc_algorithm {
	enum esif_action_type  action_type;
	unsigned int  temp_xform;
	unsigned int  tempC1;
	unsigned int  tempC2;
	unsigned int  power_xform;
	unsigned int  time_xform;
	unsigned int  size;
};

/* Must Be Aligned With DSP's event.h: event{} */
struct esif_fpc_event {
	char  name[32];
	u8    event_key[ESIF_GUID_LEN];					/* Event ID */
	enum esif_event_type esif_event;	/* ESIF Event */
	u8    event_guid[ESIF_GUID_LEN];	/* Event GUID */
	enum esif_event_group  esif_group;	/* ESIF Event Group */
	enum esif_data_type    esif_group_data_type;
};


/* Must Be Aligned With DSP's primitive.h: primitive{} */
#define MAX_SCHEMA_STRING_LENGTH 128
typedef struct esif_fpc_primitive {	// _t_EsifFpcPrimitive
	unsigned int  size;
	struct esif_primitive_tuple tuple;
	unsigned int  operation;
	unsigned int  request_type;
	char request_schema[MAX_SCHEMA_STRING_LENGTH];
	unsigned int  result_type;
	char result_schema[MAX_SCHEMA_STRING_LENGTH];
	unsigned int  num_actions;
	/* action(s) falls after the offsets */
} EsifFpcPrimitive, *EsifFpcPrimitivePtr, **EsifFpcPrimitivePtrLocation;

/* Must Be Aligned With DSP's action.h: action {} */
#define NUMBER_OF_PARAMETERS_FOR_AN_ACTION 5
typedef struct esif_fpc_action {// _t_EsifFpcAction
	unsigned int   size;
	enum esif_action_type type;
	unsigned int   priority;
	int is_kernel;
	unsigned char  param_valid[NUMBER_OF_PARAMETERS_FOR_AN_ACTION];
	unsigned int   param_offset[NUMBER_OF_PARAMETERS_FOR_AN_ACTION];
	/* parameters are laid out beyond the offset table */
} EsifFpcAction, *EsifFpcActionPtr, **EsifFpcActionPtrLocation;

#endif /* _ESIF_UF_FPC_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

