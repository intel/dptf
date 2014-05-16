#!/bin/bash

DPTF_CONFIG_DIR=/usr/share/dptf
DPTF_ETC_DIR=/etc/dptf
LIBRARY_PATH=/usr/lib64
KERNEL_DIR=/lib/modules/`uname -r`/kernel/drivers/platform/x86/intel_dptf

# Ensure we're running as root
if [ "$(id -u)" != "0" ]; then
   echo "This script must be run as root.  Try running with 'sudo'." 1>&2
   exit 1
fi

# Stop the service
initctl stop dptf

# Remove the kernel modules
rm $KERNEL_DIR/*.ko

# Rebuild the module cache
depmod -a

#Remove the configuration directories
if [ -d "$DPTF_CONFIG_DIR" ]; then
	rm -fr $DPTF_CONFIG_DIR
fi

if [ -d "$DPTF_ETC_DIR" ]; then
	rm -fr $DPTF_ETC_DIR
fi

# Remove DPTF
rm $LIBRARY_PATH/Dptf.so
rm $LIBRARY_PATH/libdptf.so
ldconfig

# Remove esif_uf executable
rm /usr/bin/esif_ufd

# Remove initscript
rm /etc/init/dptf.conf

echo "Uninstall complete.  Please reboot."
