################################################################################
## This file is provided under a dual BSD/GPLv2 license.  When using or 
## redistributing this file, you may do so under either license.
##
## GPL LICENSE SUMMARY
##
## Copyright (c) 2013 Intel Corporation All Rights Reserved
##
## This program is free software; you can redistribute it and/or modify it under 
## the terms of version 2 of the GNU General Public License as published by the
## Free Software Foundation.
##
## This program is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
## FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more 
## details.
##
## You should have received a copy of the GNU General Public License along with
## this program; if not, write to the Free Software  Foundation, Inc., 
## 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
## The full GNU General Public License is included in this distribution in the
## file called LICENSE.GPL.
##
## BSD LICENSE 
##
## Copyright (c) 2013 Intel Corporation All Rights Reserved
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##
## * Redistributions of source code must retain the above copyright notice, this
##   list of conditions and the following disclaimer.
## * Redistributions in binary form must reproduce the above copyright notice,
##   this list of conditions and the following disclaimer in the documentation 
##   and/or other materials provided with the distribution.
## * Neither the name of Intel Corporation nor the names of its contributors may
##   be used to endorse or promote products derived from this software without
##   specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
##
################################################################################

KERNEL_DIR ?= /usr/src/linux
SRC := ../../Sources
LINSRC:= $(SRC)/lin

obj-m := dptf_acpi.o dptf_cpu.o dptf_pch.o dptf_plat.o esif_lf.o esif_uf.o
ccflags-y := -Wno-multichar
ccflags-y += -DESIF_ATTR_KERNEL
ccflags-y += -DESIF_ATTR_OS_LINUX
#ccflags-y += -DESIF_ATTR_PLATFORM
#ccflags-y += -DESIF_ATTR_NO_OUTPUT

# Participants (NATIVE CODE ONLY)
dptf_acpi-objs	:= $(LINSRC)/dptf_acpi_os_lin.o
dptf_cpu-objs   := $(LINSRC)/dptf_cpu_os_lin.o
dptf_pch-objs   := $(LINSRC)/dptf_pch_os_lin.o
dptf_plat-objs   := $(LINSRC)/dptf_plat_os_lin.o

# Lower Framework R0
esif_lf-objs := $(SRC)/esif_lf.o 
esif_lf-objs += $(SRC)/esif_debug.o 
esif_lf-objs += $(SRC)/esif_poll.o 
esif_lf-objs += $(SRC)/esif_pm.o 
esif_lf-objs += $(SRC)/esif_action.o 
esif_lf-objs += $(SRC)/esif_action_const.o 
esif_lf-objs += $(SRC)/esif_action_mmio.o 
esif_lf-objs += $(SRC)/esif_action_acpi.o 
esif_lf-objs += $(SRC)/esif_action_msr.o
esif_lf-objs += $(SRC)/esif_action_var.o
esif_lf-objs += $(SRC)/esif_action_systemio.o
esif_lf-objs += $(SRC)/esif_action_code.o
esif_lf-objs += $(SRC)/esif_command.o  
esif_lf-objs += $(SRC)/esif_event.o  
esif_lf-objs += $(SRC)/esif_queue.o  
esif_lf-objs += $(SRC)/esif_primitive.o
esif_lf-objs += $(SRC)/esif_ipc.o
esif_lf-objs += $(SRC)/esif_dsp.o
esif_lf-objs += $(SRC)/esif_cpc.o
esif_lf-objs += $(SRC)/esif_data.o
esif_lf-objs += $(SRC)/esif_hash_table.o
esif_lf-objs += $(SRC)/esif_link_list.o
#OS Specific
esif_lf-objs += $(LINSRC)/esif_lf_os_lin.o
esif_lf-objs += $(LINSRC)/esif_ipc_os_lin.o

# Upper Framework R0
esif_uf-objs := $(SRC)/esif_uf.o 
esif_uf-objs += $(LINSRC)/esif_uf_os_lin.o 

all:
	make -C $(KERNEL_DIR) SUBDIRS=$(shell pwd) modules
	strip --strip-unneeded *.ko

clean:
#	make -C /lib/modules/$(shell uname -r)/build M=$(PWD)
#clean
	rm Module*.* *.o $(SRC)/*.o $(SRC)/lin/*.o *.ko *mod.c .esif* .dptf* module*.*
