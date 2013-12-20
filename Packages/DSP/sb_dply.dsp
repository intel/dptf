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
# DSP Package: sb_dply
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
H,"sb_dply",1,0,"Display Device","2C23236C-0E0A-4601-AAF4-0CD84E913304",0,"INT3406","","","","","","","","","","","",""
T,1,6,,,1,8
T,4,5,,,0,8
T,29,6,,,1,8
#
E,"DCAP",0x80,9,3,24,0xF1CDA338-0F3C-4F8D-A1D9-8033E672F672
E,"DSTS",0x81,10,3,24,0xBDB4F356-CF69-4152-99A9-1DCE4972AB9D
#
D,"DFL","Default Domain","D0","E3B59D8D-54B2-4E05-81D4-2CBBF87295EC",0,10
C,0xC08 ,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0
#
# GET_CLOCK_COUNT
P,185,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,64,32
A,1,1,True,1,,,,
# GET_CLOCK_ORIGINAL_FREQUENCY
P,187,0,1,24,"<schema></schema>",4,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,128,32
A,1,1,True,1000000,,,,
# GET_CLOCK_ORIGINAL_FREQUENCY
P,187,1,1,24,"<schema></schema>",4,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,224,32
# GET_CLOCK_ORIGINAL_FREQUENCY
P,187,2,1,24,"<schema></schema>",4,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,320,32
# GET_CLOCK_ORIGINAL_FREQUENCY
P,187,3,1,24,"<schema></schema>",4,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,416,32
# GET_CLOCK_ORIGINAL_FREQUENCY
P,187,4,1,24,"<schema></schema>",4,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,512,32
# GET_CLOCK_SPREAD_DIRECTION
P,191,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,36,2
A,1,1,True,3,,,,
# GET_CLOCK_SPREAD_PERCENTAGE
P,190,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,40,4
A,1,1,True,20,,,,
# GET_CLOCK_SPREAD_SUBHARMONICS
P,192,0,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,1,True,1,,,,
# GET_CLOCK_SSC_DISABLED_FREQUENCY
P,189,0,1,24,"<schema></schema>",4,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,192,32
A,1,1,True,1200000,,,,
# GET_CLOCK_SSC_DISABLED_FREQUENCY
P,189,1,1,24,"<schema></schema>",4,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,288,32
# GET_CLOCK_SSC_DISABLED_FREQUENCY
P,189,2,1,24,"<schema></schema>",4,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,384,32
# GET_CLOCK_SSC_DISABLED_FREQUENCY
P,189,3,1,24,"<schema></schema>",4,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,480,32
# GET_CLOCK_SSC_DISABLED_FREQUENCY
P,189,4,1,24,"<schema></schema>",4,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,576,32
# GET_CLOCK_SSC_ENABLED_FREQUENCY
P,188,0,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,160,32
A,1,1,True,1100000,,,,
# GET_CLOCK_SSC_ENABLED_FREQUENCY
P,188,1,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,256,32
# GET_CLOCK_SSC_ENABLED_FREQUENCY
P,188,2,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,352,32
# GET_CLOCK_SSC_ENABLED_FREQUENCY
P,188,3,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,448,32
# GET_CLOCK_SSC_ENABLED_FREQUENCY
P,188,4,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,544,32
# GET_DEVICE_HARDWARE_ID
P,53,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,4,True,"_HID",,,,
# GET_DEVICE_STATUS
P,88,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,4,True,"_STA",,,,
# GET_DEVICE_UNIQUE_ID
P,67,255,1,24,"<schema></schema>",8,"<schema></schema>"
A,0,4,True,"_UID",,,,
# GET_DISPLAY_BRIGHTNESS_LEVELS
P,158,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,32,False,0x4C424154,,,,
A,1,4,True,"_BCL",,,,
# GET_DISPLAY_BRIGHTNESS_SOFT
P,157,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,32,False,0x54495242,,,,
A,1,4,True,"_BQC",,,,
# GET_DISPLAY_CAPABILITY
P,159,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,4,True,"DDPC",,,,
# GET_DISPLAY_CLOCK_DEVIATION
P,181,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,0,32
A,1,1,True,20,,,,
# GET_DISPLAY_DEPTH_LIMIT
P,160,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,32,False,0x48545044,,,,
A,1,4,True,"DDDL",,,,
# GET_DISPLAY_PANEL_TYPE
P,179,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,44,1
A,1,1,True,1,,,,
# GET_GRAPHICS_CHIPSET_CHANNEL_TYPE
P,182,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,35,1
A,1,1,True,0,,,,
# GET_GRAPHICS_CHIPSET_SSC_ENABLED
P,184,255,1,24,"<schema></schema>",27,"<schema></schema>"
A,0,30,False,0x5f840822,0x6D6E7000,76,34,1
A,1,1,True,1,,,,
# SET_CLOCK_SSC_DISABLED
P,194,0,2,27,"<schema></schema>",24,"<schema></schema>"
A,0,30,False,0x5f850822,0x6D6E7000,68,128,32
# SET_CLOCK_SSC_DISABLED
P,194,1,2,27,"<schema></schema>",24,"<schema></schema>"
A,0,30,False,0x5f850822,0x6D6E7000,68,224,32
# SET_CLOCK_SSC_DISABLED
P,194,2,2,27,"<schema></schema>",24,"<schema></schema>"
A,0,30,False,0x5f850822,0x6D6E7000,68,320,32
# SET_CLOCK_SSC_DISABLED
P,194,3,2,27,"<schema></schema>",24,"<schema></schema>"
A,0,30,False,0x5f850822,0x6D6E7000,68,416,32
# SET_CLOCK_SSC_DISABLED
P,194,4,2,27,"<schema></schema>",24,"<schema></schema>"
A,0,30,False,0x5f850822,0x6D6E7000,68,512,32
# SET_CLOCK_SSC_ENABLED
P,193,0,2,27,"<schema></schema>",24,"<schema></schema>"
A,0,30,False,0x5f850822,0x6D6E7000,68,96,32
# SET_CLOCK_SSC_ENABLED
P,193,1,2,27,"<schema></schema>",24,"<schema></schema>"
A,0,30,False,0x5f850822,0x6D6E7000,68,192,32
# SET_CLOCK_SSC_ENABLED
P,193,2,2,27,"<schema></schema>",24,"<schema></schema>"
A,0,30,False,0x5f850822,0x6D6E7000,68,288,32
# SET_CLOCK_SSC_ENABLED
P,193,3,2,27,"<schema></schema>",24,"<schema></schema>"
A,0,30,False,0x5f850822,0x6D6E7000,68,376,32
# SET_CLOCK_SSC_ENABLED
P,193,4,2,27,"<schema></schema>",24,"<schema></schema>"
A,0,30,False,0x5f850822,0x6D6E7000,68,480,32
# SET_DISPLAY_BRIGHTNESS_HARD
P,226,255,2,29,"<schema></schema>",24,"<schema></schema>"
A,0,35,True,0x54495242,,,,
# SET_DISPLAY_BRIGHTNESS_SOFT
P,163,255,2,29,"<schema></schema>",24,"<schema></schema>"
A,0,32,False,0x54495242,,,,
