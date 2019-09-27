About
=====

LibMonetra is the C library for communicating Monetra Technologies, LLC products
that utilize the Monetra IP/SSL protocol. It was created by Monetra Technologies, LLC
for use in their Monetra Payment Engine and associated products. Such
as, UniTerm.

v8 Changes
----------

The v8 version of this library is a ground-up rewrite depending on the mstdlib library.  It 
contains a new API which makes public use of mstdlib objects, therefore to use the new
API one must also familiarize themselves with mstdlib.  The new API is an event based
architecture designed for low-latency and high-volume purposes.  The prior API was a
polling-based API which was often used in a blocking manner.

A full compatibility API for the prior (v3+) interfaces are also available that aim to be
fully API and ABI  compatible.  There may be some slight behavioral differences with the
compatibility API that are not expected to cause issues such as automatic reconnects,
automatic hardening of returned transaction ids, and always-on thread-safety.

As of the v8 release proxy is no longer supported.

Building
========

CMake is the preferred method to build and should be used whenever possible.
However, CMake is not available on all supported platforms. Autotools is also
supported for Unix systems. NMake Makefiles are also available for Windows.
Autotools and NMake Makefiles are provided only as a fallback when CMake cannot
be used. That said, Autotools and NMake Make files can lag behind being updated
and there is no  timeline on how long they will be supported.

The following features are supported by each build system:

Feature                                | CMake | Autotools | NMake Makefiles
:--------------------------------------|:-----:|:---------:|:--------------:
Shared build                           | Y     | Y         | Y
Static build                           | Y     | Y         | N
Disabling building non-base components | Y     | Y         | N
Installation (header and library)      | Y     | Y         | N
Disabling header installation          | Y     | N         | N
Disabling library installation         | Y     | N         | N
Tests                                  | Y     | Y         | N

CMake
-----

    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_BUILD_TYPE=<DEBUG|RELEASE> ..
    $ make


Dependencies
------------

The only dependency is mstdlib available from https://github.com/Monetra/mstdlib/.
It should either be compiled and installed prior to libmonetra, or it can be
chain built if the source is placed in `thirdparty/mstdlib`. 

