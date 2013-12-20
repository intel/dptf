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
	 (<DPTF archive root>/DPTF/Linux/build) and run the command:

	cmake -DCHROMIUM_BUILD=YES -DBUILD_ARCH=64bit -DCMAKE_BUILD_TYPE=Debug ..

This command will invoke cmake and generate all the GNU make files for each 
sub-modules of DPTF user space libraries.

Step 5 - Run make to build all DPTF shared libraries.

	make

The generated shared libraries will be located under 
<DPTF root>/Products/DPTF/Linux/build/x64/debug directory. Users can disregard
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
<DPTF archive root>/Products/ESIF_UF/Chrome64/<Debug | Release> directory, and 
run the following commands: 

	make clean
	make

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
	ln -s /home/brad/trunk/src/partner_private/kernel-baytrail /usr/src/linux

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
Intel (R) Atom (TM) based systems. The MBI driver has been submitted to 
Linux Kernel Mailing List (LKML.ORG) by Intel Corp. As of this writing,
the latest submission can be found from the link below:

https://lkml.org/lkml/2013/12/6/723

The user is expected to download this driver from lkml.org and build it
from the chroot environment. The generated kernel module is named
intel_baytrail.ko. 

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

Step 2 - Start ESIF Upper Framework Shell
To start ESIF upper framework shell, go to the ufx64 subdirectory and run
esif_uf:
	./esif_uf

ESIF should start and present the user the ESIF shell at this point.

Step 3 - Load Policy Libraries
To start the various policy libraries (passive library, critical libraries,
etc.), run this command under ESIF shell:

	appstart Dptf

DPTF policies will then be loaded and will become active.

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

* Intel DPTF must be started through the shell with this release. It is NOT 
running as a daemon.

* Limited testing has only been performed on Intel BayTrail-M based development
platforms and the 4th generation Intel® Core ™ processor based development 
platforms using the UEFI BIOS.

* When running ESIF UF application, the user may encounter minor memory leaks. 
This issue is being addressed and will be fixed in the next revision.

* Compilation warnings will be noticed during the build process. These are 
being addressed and will be fixed in a future release.
    
* Display brightness control is not currently functional.

