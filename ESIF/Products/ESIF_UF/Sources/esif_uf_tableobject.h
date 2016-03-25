/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_UF_TABLEOBJECT_
#define _ESIF_UF_TABLEOBJECT_
#include "esif_lib_esifdata.h"

#define VARIANT_MAX_FIELDS 50
#define TABLE_OBJECT_MAX_NAME_LEN	255
#define TABLE_OBJECT_MAX_BINARY_LEN	0x7FFFFFFF

enum tableMode {
	GET = 0,
	SET
};

enum tableType {
	BINARY = 0,
	VIRTUAL,
	DATAVAULT
};

typedef struct TableField_s {
	char *name;
	char *label;
	EsifDataType dataType;
	UInt32 getPrimitive;
	UInt32 setPrimitive;
	UInt8 instance;
	char *dataVaultKey;
	int  notForXML;
} TableField;

typedef struct TableObject_s {
	int numFields;
	int participantId;
	esif_flags_t options;
	enum tableType type;
	EsifDataType dataType;
	char *name;
	char *domainQualifier;
	char *dataSource;
	char *dataMember;
	char *dataXML;
	char *dataText;
	u8 *binaryData;
	UInt32 binaryDataSize;
	UInt32 getPrimitive;
	UInt32 setPrimitive;
	eEsifEventType changeEvent;
	char *dataVaultCategory;
	TableField fields[VARIANT_MAX_FIELDS];
	int dynamicColumnCount;
	UInt64 version;
	enum tableMode mode;
} TableObject;

struct esif_data_binary_fst_package {
	union esif_data_variant revision;
	union esif_data_variant control;
	union esif_data_variant speed;
};

/* Bit Flags for TableObject.options */
#define TABLEOPT_CONTAINS_REVISION		0x00000001	// Contains Table Revision
#define TABLEOPT_ALLOW_SELF_DEFINE		0x00000002	// Allow Self-Define
#define TABLEOPT_HAS_NOPERSIST			0x00000004	// Table may have a /nopersist/%nm%/name value in OVERRIDE

void TableField_Construct(TableField *dataField, char *fieldName, char *fieldLabel, EsifDataType dataType, int forXML);
void TableField_Destroy(TableField *dataField);
void TableObject_Construct(TableObject *self, char *dataName, char *domainQualifier, char *dataSource, char *dataMember, char *dataText, int participantId, enum tableMode mode);
void TableObject_Destroy(TableObject *self);
eEsifError TableObject_LoadData(TableObject *self);
eEsifError TableObject_LoadSchema(TableObject *self);
eEsifError TableObject_LoadAttributes(TableObject *self);
eEsifError TableObject_Save(TableObject *self);
eEsifError TableObject_Delete(TableObject *self);
eEsifError TableObject_LoadXML(TableObject *self);
eEsifError TableObject_Convert(TableObject *self);

#endif