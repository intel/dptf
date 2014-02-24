Intel (R) Dynamic Platform and Thermal Framework (Intel (R) DPTF) 
for Chromium OS - Alpha Release 

README

-------------------------------------------------------------------------------

This document describes how to build DPTF for Chromimum OS. The complete
DPTF package consists of several modules. At the bottom layer there are ESIF
(Eco-System Independent Framework) Linux kernel modules that interact with 
both system BIOS and upper ESIF layer. The upper ESIF layer is a user 
space application that provides an ESIF shell environment, allowing users
to issue ESIF commands directly to system BIOS, and/or load DPTF policies. 
At the very top layer there are DPTF policies in the form of shared libraries,
which can be loaded through ESIF shell. Together all these components form
the entire DPTF package. This document will walk through the steps to build
all these components including policy libraries, ESIF application/shell, and
lower layer ESIF kernel modules.

This document assumes that the user has already set up the Chromium OS SDK on
their development machine. Otherwise, please refer to Google's "Chromium OS 
Developer Guide" available at:

	http://www.chromium.org/chromium-os/developer-guide

-------------------------------------------------------------------------------
BUILDING DPTF POLICY LIBRARIES
-------------------------------------------------------------------------------

Step 1 - Download DPTF code from its github repository, and copy the code to
the Chrome OS chroot directory, for example, to <chromimumos>/chroot/home/<user>

Step 2 - Enter the Chromium OS SDK chroot environment by running the command:

	sudo <chromimumos root>/chromite/bin/cros_sdk

Step 3 - Check if CMake has been installed, and if not, run this command 
to install:

	sudo emerge cmake

Step 4 - Go to the Linux subdirectory of DPTF
	 (<DPTF archive root>/Products/DPTF/Linux/build) and run the command:

	CC="x86_64-cros-linux-gnu-gcc" CXX="x86_64-cros-linux-gnu-g++"
	   CXXFLAGS='-g -O0' cmake -DCHROMIUM_BUILD=YES -DBUILD_ARCH=64bit ..

This command will invoke cmake and generate all the GNU make files for each 
sub-modules of DPTF user space libraries.

Step 5 - Run make to build all DPTF shared libraries.

	make

The generated shared libraries will be located under 
<DPTF root>/Products/DPTF/Linux/build/x64 directory. Users can disregard
the static .a libraries as these static libraries are only used to build the
shared library. Here is the break down of the generated shared libraries that
are needed to run DPTF for this Alpha release on Chromium.

	* Dptf.so
	* DptfPolicyCritical.so
	* DptfPolicyPassive.so

-------------------------------------------------------------------------------
BUILDING ESIF UPPER FRAMEWORK (SHELL APPLICATION)
-------------------------------------------------------------------------------
Still in chroot, we can now build the ESIF shell application. Simply go to the 
<DPTF archive root>/Products/ESIF_UF/Chrome64 directory, and 
run the following commands: 

	make clean
	CC='x86_64-cros-linux-gnu-gcc' CFLAGS='-g -O0 -DESIF_ATTR_DEBUG
	     -DESIF_ATTR_MEMTRACE' make

After the build is complete, the esif_uf binary executable will be generated in
the current directory.

-------------------------------------------------------------------------------
BUILDING ESIF LOWER FRAMEWORK (LINUX KERNEL MODULES)
-------------------------------------------------------------------------------
The ESIF kernel modules will also be built in the chroot environment. 

Step 1 - Locate the makefile for ESIF_LF. Depending on if the user wants to 
build the debug or the release version, this file should be located under
<DPTF archive root>/Products/ESIF_LF/Linuxx64Atom/Debug or 
<DPTF archive root>/Products/ESIF_LF/Linuxx64Atom/Release directory.

Step 2 - Setup a symbolic link at /usr/src/linux that points to the desired
kernel headers.  Example:

	mkdir /usr/src (if the directory does not exist)
	ln -s /home/<user>/trunk/src/third_party/kernel/3.10 /usr/src/linux

Step 3 - Obtain the kernel config file. The kernel config file can be generated
by going to the kernel source root directory, then running the command:

	./chromeos/scripts/kernelconfig genconfig

The generated kernel config can then be located under the CONFIGS subdirectory
from the kernel source root. Copy the one that matches the target (in our 
case, x86_64-chromiumos-x86_64.flavour.config) to the kernel source root, 
and then rename it to .config. 

Step 4 - Run "make modules" command in order to generate the Module.symvers
file. This file is needed when building ESIF_IF kernel modules.

Step 5 - After all the above steps are done, the user can now issue "make clean"
followed by "make" commands in the ESIF_LF directory to build the ESIF kernel
modules. Depending on if the user needs the debug or release version, the 
makefile is located under ESID/Products/ESIF_LF/Linuxx64/Debug or 
ESID/Products/ESIF_LF/Linuxx64/Release directory. The kernel modules will 
be generated in the current local diretory. There will be 5 files generated,
but only 4 are needed and they are:

	* esif_lf.ko
	* dptf_acpi.ko
	* dptf_cpu.ko
	* dptf_pch.ko

Step 6 - The ESIF lower framework drivers are dependent on the transport
MailBox Interface (MBI) driver to exchange information with BIOS on
Intel (R) Atom (TM) based systems. This driver is already included in the
official Rambi build from Google's chromium.org tree, but if you need to
test DPTF on other target platforms, you may need to build this driver
manually. The source code of this driver can be found here:

The header file:
https://chromium.googlesource.com/chromiumos/third_party/kernel-next/+/chromeos-3.10/arch/x86/include/asm/iosf_mbi.h

The source code:
https://chromium.googlesource.com/chromiumos/third_party/kernel-next/+/chromeos-3.10/arch/x86/kernel/iosf_mbi.c

The kernel config file that enables the building of this driver:
https://chromium.googlesource.com/chromiumos/third_party/kernel-next/+/chromeos-3.10/chromeos/config/x86_64/common.config
Note that CONFIG_IOSF_MBI is set to y in this file, but you can also set it
to 'm' to build the driver as a loadable module.

The actual kernel make file that builds this module based on the config
file settings:
https://chromium.googlesource.com/chromiumos/third_party/kernel-next/+/chromeos-3.10/arch/x86/kernel/Makefile
Please note the line in this file that builds the driver based on kernel
config file settings:
obj-$(CONFIG_IOSF_MBI) += iosf_mbi.o

-------------------------------------------------------------------------------
CREATING DPTF TAR BALL PACKAGE
-------------------------------------------------------------------------------
Once we have generated all the binaries for DPTF, we can now package them up
by running the included pack.sh shell script. This script will gather all
the binaries that we have built so far, put them in a particular layout,
and create the tar ball package. For the script to successfully run two
paths must be set in the script itself.  They are:

	CHROMIUM_SDK_ROOT - This is the full system path to the home directory
		in the chroot environment.
	BUILD_DIRECTORY - This is the path to where the compiled binaries
		should be placed.

Once the variables are set and the script is run, you should see the gzipped
tar ball file in the form of dptf_chrome_build.xxx.tar.gz in the current
directory. Copy this file to target platform and untar to the desired location
(for example, /root), and you will be ready to run DPTF on the target 
system.

-------------------------------------------------------------------------------
RUNNING DPTF ON TARGET PLATFORM
-------------------------------------------------------------------------------
After you untar the tar ball on the target platform, you will notice that the
following directory structure are created for DPTF:

    chrome_build
         |
         |----cmd
         |----drivers
         |----dsp
         |----log
         |----ufx64

Step 1 -  Install all the kernel modules in the drivers sub-directory:
Before we can run DPTF, all the kernel modules in the drivers sub-directory
must be installed first. Due to the dependency relationships among these
kernel modules, the intel_baytrail.ko must be installed first (which should
be built by the user outside of DPTF, as described by Step 6 in the BUILDING
ESIF LOWER FRAMEWORK section above). Next the esif_lf.ko shall be installed,
and then followed by the rest of the modules (there are no dependencies among 
dptf_acpi, dptf_cpu and dptf_pch, therefore any order of installing these
three modules is fine).

Step 2 - Move the necessary configuration files to /usr/share/dptf.  The
esif_uf application will be looking for various files at this location.
The commands to do the move are as follows (executed from the chrome_build
dir):
	a. mkdir /usr/share/dptf (if it doesn't already exist)
	b. cp -R ./dsp /usr/share/dptf
	c. cp ./ufx64/DptfPolicy*.so /usr/share/dptf
	d. cp ./ufx64/Dptf.so /usr/lib
	e. cp ./ufx64/combined.xsl /usr/share/dptf 
	f. cp ./ufx64/esif_ufd /usr/bin
	g. cp -R ./cmd /usr/share/dptf	

Step 3 - Start ESIF Upper Framework Daemon
To start ESIF upper framework shell, go to the ufx64 subdirectory and run
esif_uf:
	./esif_ufd

If you run the esif_ufd without any arguments, it will run as a daemon that
you cannot interact with.  If you wish to run interactively, pass it the -s
switch.  The help text for the esif_ufd can be viewed by passing it the -h
switch.

Provided that the contents of the /usr/share/dptf/cmd/start file are exactly
"appstart Dptf", DPTF will be automatically started when esif_ufd is executed.
If you wish to manually start DPTF, remove the /usr/share/dptf/cmd/start file.

Once the appstart has been issued, DPTF will be active.

(Optional) Step 4 - Start ESIF Upper Framework Daemon w/command pipe and log
Since the esif_ufd runs as a daemon by default, switches can be passed to
create a pipe that can be used to issue shell commands and to log shell output.
To run the daemon with pipe and output log support, issue the following
command:
	esif_ufd -p -l

This will create a command pipe at /tmp/esifd.cmd and an output log at
/tmp/esifd.log.  You can interact with these files with the following commands:
	cat > /tmp/esifd.cmd
		This allows you to pass commands to the pipe as if it were the
		shell prompt.
	tail -f /tmp/esifd.log
		This will show the live output from the daemon as commands are
		executed.

-------------------------------------------------------------------------------
VERIFY INSTALL/CONFIGURATION
-------------------------------------------------------------------------------
Once you can successfully enter the ESIF shell (by running the esif_uf),
commands can be issues to verify what has been exposed to ESIF.  These commands
include (help text available by typing "help" or "help <command>"):

	participants
		Displays a list of currently available participants.

	apps
		Displays a list of currently running apps.

	dstn <name>
		Switch the active participant to the one specified by the
		name arguement (string).


Command examples:

	1. Get the temperature for Domain 0 of Participant 1:
		dstn TCPU
		getp 14 D0 255
			

-------------------------------------------------------------------------------
KNOWN ISSUES / LIMITATIONS
-------------------------------------------------------------------------------

* The code on GitHub has only been built, tested, and validated on the 
BayTrail-M kernel for Chromium 64 bit.  Instructions on how to build for the 
generic Linux kernels (32 and 64 bit) will be coming shortly.

* Limited testing has only been performed on Intel BayTrail-M based development
platforms and the 4th generation Intel® Core ™ processor based development 
platforms using the UEFI BIOS.

* When running ESIF UF application, the user may encounter minor memory leaks. 
This issue is being addressed and will be fixed in the next revisions.

* Compilation warnings will be noticed during the build process. These are 
being addressed and will be fixed in a future release.
    
* Display brightness control is not currently functional.

* The daemon is intermittently dying after a period of time.  The root cause is
unknown at this time.  The issue is under investigation.

* When the system first boots the daemon will sporadically appear to start but
the DPTF application is not running.  The workaround is to restart the daemon
with "sudo initctl restart dptf".  We have root caused the issue and are 
currently working on a fix for the next release.

* Compiling the code with any level of optimization beyong -O0 is causing the 
code to break.  Do not compile with optimization at this time.  This will be
addressed in the next release.
