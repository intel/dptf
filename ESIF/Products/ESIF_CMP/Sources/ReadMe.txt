========================================================================
esif_cmp - ESIF Compression Library
Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
Portions Public Domain Igor Pavlov
========================================================================

This project contains an OS-agnostic loadable library (esif_cmp.dll or esif_cmp.so)
of the LZMA Compression algorithm based on Public Domain code included in
LZMA_SDK v18.01 by Igor Pavlov which is available for download at these links:

http://www.7-zip.org/sdk.html
https://sourceforge.net/projects/sevenzip/files/LZMA%20SDK/

In order to maintain compatibility with future releases of LZMA_SDK as
well as comply with Intel SDL requirements, the code is divided up into
the following components:

esif_cmp.c = ESIF Exported LZMA Abstraction Functions and DLL Main
Alloc.c    = LZMA Memory Allocation Routines
LzFind.c   = LZMA Match Finder Routines
LzmaDec.c  = LZMA Decoder
LzmaEnc.c  = LZMA Encoder

NOTES

1. esif_cmp.c is owned by ESIF and implments LZMA Wrapper functions to
abstract the compression algorithm and compression configuration parameters
from the caller. This includes EsifSdl.h, which is included by all modules.
Exported Loadable Library Functions are:

    a. EsifCompress
	b. EsifDecompress

2. All other modules are owned by the LZMA_SDK implementation by
Igor Pavlov with the following changes made in order to conform with
Intel SDL Requirements:

	a. Addition of EsifSdl.h to all .c files as the last #include in the module
	b. Change all memcpy calls to MyMemcpy in LzmaDec.c and LzmaEnc.c (see EsifSdl.h)
	c. Changes to LzmaEnc.c to pass Klocwork buffer overrun issues (see SDL_PROB and SDL_SYMBOLIDX)
	d. Changes to LzFind.c to call _mm_lfence() to avoid SPECTRE.VARIANT1 Warning (See SDL_SPECTRE_FENCE)
	e. Changes to LzFind.c to move (size_t) cast inside expression to avoid Overflow Warnings (See SDL_COMMENT)

3. LZMA Compression Library is compiled for Single-Threaded support only in
order to reduce code size, complexity, and OS Abstraction. (_7ZIP_ST option).
This significantly affects the Compression speed but barely Decompression speed.

4. In order to maintain maximum compatibility with the LZMA_SDK sample code
and application (lzma.exe), all LZMA compression parameters are hardcoded in
esif_cmp.c and correspond to the following lzma.exe command line options, thus
allowing data that has been compressed with lzma.exe to be decompressed by
esif_cmp.dll and vice versa (lzma.exe defaults indicated by [default]):

	-a1    = Compression Level (-a0=1, -a1=9) [default]
	-d24   = Dictionary Size (1<<N) [default = -d17]
	-lc3   = Literal Context Bits [default]
	-lp0   = Literal Pos Bits [default]
	-pb2   = Number of Pos Bits [default]
	-fb128 = Number of Fast Bytes [default]
	-mt1   = CPU Threads (Single-Threaded Only)
   
Note that the -d -lc -lp -pb options affect the standard 5-byte LZMA data header,
which is why these options were chosen so that we may identify data that is
compressed by looking at the data header. The above options correspond to the
following 13-byte LZMA file header (where XX is the 64-bit uncompressed data size):

5D 00 00 00 01 XX XX XX XX XX XX XX XX

[Encoding: 5D = -lc3 -lp0 -pb2 and 00 00 00 01 = -d24]

Note that the -a -fb -mt options DO NOT affect the standard 5-byte LZMA data header,
although they may affect the compressed data, so these were chosen to yield the best
compression ratios in most situations. It is safe to change these three values without
affecting the LZMA data header.
