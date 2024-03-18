Following third-party components are being used for all variants of CAN-Server:

| Component Name | Version | Folder  | SPDX-Identifier / License name | Compliance checked by | Comment |
|----------------|---------|---------|--------------------------------|-----------------------|---------|
| pthread        | 2006-12-20 | lib/pthread | GNU LGPL v2.1              | Martin Wodok          |         |
| yasper         | 1.04    | src/yasper  | Zlib / zlib License        | Martin Wodok          | Modified to work without C++-exceptions. Comments are inline to mark the modifications done. |


Depending on the used CAN-device some of the following libraries/driver have to be downloaded and put into the respective folder:

| Component Name | Version | Folder  | SPDX-Identifier / License name | Compliance checked by | Comment |
|----------------|---------|---------|--------------------------------|-----------------------|---------|
| kvaser         | latest  | lib/kvaser | canlib / CANlib SDK license | Has to be checked by you! | If using this variant, you have to install the SDK/drivers/libraries and check the license if it fits for your needs, together with the license of the CAN-Server itself!  |
| PCAN-Basic     | latest  | lib/pcan  | PEAK proprietary license     | Martin Wodok          | Can be used with original PEAK hardware, no restrictions. |
