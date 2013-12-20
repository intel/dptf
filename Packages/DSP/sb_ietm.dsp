################################################################################
## Copyright (c) 2013 Intel Corporation All Rights Reserved
##
## Licensed under the Apache License, Version 2.0 (the "License"); you may not
## use this file except in compliance with the License.
##
## You may obtain a copy of the License at
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS"BASIS, WITHOUT
## WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##
## See the License for the specific language governing permissions and
## limitations under the License.
##
################################################################################
#
# Eco-System Independent Framework v1 (ESIF)
# ESIF Device Support Package (DSP)
#
# DSP Package: sb_ietm
#      Format: DSP Compiler
#     Version: 1.0
#        HMAC: 0000000000000000000000000000000000000000
#
# =====                            ======
# ===== AUTO GENERATED DO NOT EDIT ======
# =====                            ======
#
#
# H=Name, VerMaj, VerMin, Description, Type, BusEnum, acpiDevice, acpiUID, acpiType, acpiScope, pciVendorID, pciDeviceID, pciBus, pciDevice, pciFunction, pciRevision, pciClass, pciSubClass, pciProgIF
# T=actionType, tempXform, tempC1, tempC2, powerXform, timeXform
# E=alias, eventKey, eventType, eventGroup, eventDataType, eventGUID
# D=name, description, qualifier, domainType, priority
# C=capability, capabilityByte Array
# P=Primitive, Qualifier, Instance, Operation, RequestType, ReturnType
# A=ActionPriority, ActionType, IsKernel, p1, p2, p3, p4, p5
#
H,"sb_ietm",1,0,"DPTF Manager Device","A6866C8E-0486-4797-9EA4-BBDF87DEA95D",0,"INT3400","","","","","","","","","","","",""
#
E,"ART",0x84,2,3,24,0xC7C5FC88-8AAC-42C2-8B51-21777033E75D
E,"CMAL",0x63c39116-4f72-11dc-8314-0800200c9a66,25,1,3,0x0CB6C2E2-3242-40FC-845F-17F824FB857E
E,"CMPL",0x4a44b800-4f72-11dc-8314-0800200c9a66,22,1,3,0xDBF7B2CF-3B16-4773-9CA9-DD74FF91D6BF
E,"CSE",0x917B3B17-5447-4447-B092-3CE756C36AC9,0,4,24,0xFD34F756-F7B6-47DD-B3D5-0011A34E4337
E,"CSX",0xAB9463CA-7D63-4FBB-9996-72951E172616,1,4,24,0x9604508D-F4AA-4716-83D9-6EE951EBBEA9
E,"FG",0x334589E9-2E41-4BC0-B9F9-D37ED44026B6,4,4,8,0x88E419E3-609B-4BDA-9A17-83DE899831FD
E,"LPM",0x85,30,3,24,0xDDADD3BF-2385-4E3B-B242-2793B81293AA
E,"OCTD",0x07029cd8-4664-4698-95d8-43b2e9666596,31,1,3,0x07029cd8-4664-4698-95d8-43b2e9666596
E,"OLPM",0xb29c73e0-1a8b-46fd-b4ae-1ce5a3d6d871,23,1,3,0x5569447B-6E8F-4FE2-94DE-C31DA011ECF7
E,"PSVT",0x86,24,3,24,0x661C68E1-B73E-4D02-859B-F1C1505F90D1
E,"SCP",0x94d3a615-a899-4ac5-ae2b-e4d8f634367f,29,1,3,0x5C7D591E-2EA8-4DA1-85A4-476191404650
E,"SOC",0x2AF4D02D-8954-4429-AF3A-1B91C6E34CCE,26,2,3,0x019C3571-3560-4EC6-BDED-884F6125B5F9
E,"SPC",0xC29F3A66-3FE4-462C-A8F5-7EDED93E6C29,28,2,3,0xC7C83E34-519B-4650-A8B2-640E31F5BB0A
E,"SSOC",0xCA267DF9-852E-4A29-BCCF-23652611004B,27,2,3,0x164B8FD7-C165-4C86-8E9B-4464B6EEC015
E,"TRT",0x83,3,3,24,0x7E99E90E-0A22-4EEC-AD6C-908DEB21E9A9
#
D,"DFL","Default Domain","D0","CCB4AB18-81DC-4CD1-966F-7C90CA56468A",0,14
C,0x0 ,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
#
# GET_ACTIVE_RELATIONSHIP_TABLE
P,89,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_art",,,
A,1,4,True,"_ART",,,,
# GET_CURRENT_LOW_POWER_MODE
P,94,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/clpm",,,
A,1,4,True,"CLPM",,,,
# GET_DEVICE_HARDWARE_ID
P,53,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,4,True,"_HID",,,,
# GET_DEVICE_STATUS
P,88,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,4,True,"_STA",,,,
# GET_LPM_TABLE
P,237,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/lpmt",,,
A,1,4,True,"LPMT",,,,
# GET_SUPPORTED_POLICIES
P,92,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,4,True,"IDSP",,,,
# GET_THERMAL_RELATIONSHIP_TABLE
P,91,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_trt",,,
A,1,4,True,"_TRT",,,,
# SET_ACTIVE_RELATIONSHIP_TABLE
P,230,255,2,7,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_art",,,
# SET_CURRENT_LOW_POWER_MODE
P,236,255,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/clpm",,,
# SET_LPM_TABLE
P,238,255,2,7,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/lpmt",,,
# SET_OPERATING_SYSTEM_CAPABILITIES
P,93,255,2,7,"<schema></schema>",24,"<schema></schema>"
A,0,4,True,"_OSC",,,,
# SET_SYSTEM_HIBERNATE
P,175,255,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,31,False,"SYSTEM_HIBERNATE","","","",""
# SET_SYSTEM_SHUTDOWN
P,173,255,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,31,False,"SYSTEM_SHUTDOWN","","","",""
# SET_SYSTEM_SLEEP
P,174,255,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,31,False,"SYSTEM_SLEEP","","","",""
# SET_THERMAL_RELATIONSHIP_TABLE
P,231,255,2,7,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_trt",,,
