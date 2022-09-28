Kvaser CANlibSDK
~~~~~~~~~~~~~~~~

Windows (32-bit)
~~~~~~~~~~~~~~~

Download the CANlibSDK from:

https://www.kvaser.com/download/?utm_source=software&utm_ean=7330130980150&utm_status=latest

and install it to the preferred path.

Then copy the following files:

- INC/*
- Lib/MS/canlib32.lib (Windows 32-bit)

respectively to:

- windows/inc
- windows/lib/

folders of this README file.

To create the VS solution do:

- cmake -A Win32 .

Linux
~~~~~

The required library is shipped with the drivers, so installing the drivers should place the binary and the header in the right place to build the CAN-server (see DRIVERS.TXT):

- header: /usr/include/canlib.h
- lib   : /usr/lib/libcanlib.so