**_Note: Major API Changes. For older projects, you may use the [v0.9 branch](https://github.com/bluekitchen/btstack/tree/v0.9). 
Please see [Migration notes](https://github.com/bluekitchen/btstack/blob/master/doc/manual/docs/appendix/migration.md)_**

# Welcome to BTstack

BTstack is [BlueKitchen's](http://bluekitchen-gmbh.com) implementation of the official Bluetooth stack. 
It is well suited for small, resource-constraint devices 
such as 8 or 16 bit embedded systems as it is highly configurable and comes with an ultra small memory footprint. 
A minimal configuration for an SPP server on a MSP430 can run in 32 kB FLASH and only 4 kB of RAM.

Targeting a variety of platforms is as simple as providing the necessary UART, CPU, and CLOCK implementations. BTstack is currently capable of connecting to Bluetooth-modules via: (H2) HCI USB, (H4) HCI UART + TI's eHCILL, and (H5) HCI Three-Wire UART.

On smaller embedded systems, a minimal run loop implementation allows to use BTstack without a Real Time OS (RTOS). 
If a RTOS is already provided, BTstack can be integrated and run as a single thread. 

On larger systems, BTstack provides a daemon that connects to a Bluetooth module. 
Multiple applications can communicate with this daemon over different inter-process communication methods.

BTstack supports the Central and the Peripheral Role of Bluetooth 4.2 Low Energy specification. 
It can be configured to run as either single-mode stack or a dual-mode stack.

BTstack is free for non-commercial use. However, for commercial use, <a href="mailto:contact@bluekitchen-gmbh.com">tell us</a> a bit about your project to get a quote.

**Documentation:** [HTML](http://bluekitchen-gmbh.com/btstack/develop/), [PDF](http://bluekitchen-gmbh.com/btstack_develop.pdf)

**Discussion and Community Support:** [BTstack Google Group](http://groups.google.com/group/btstack-dev)

### Supported Protocols and Profiles

**Protocols:** L2CAP, RFCOMM, SDP, BNEP, ATT, SM (incl. LE Secure Connections).

**Profiles** GAP, IOP, HFP, HSP, SPP, PAN, GATT.

**Coming next** A2DP, AVRCP, HID, HOGP, BLE Mesh, and more.

It has been qualified with the the Bluetooth SIG for GAP, IOP, HFP, HSP, SPP, PAN profiles and 
GATT, SM of the Bluetooth 4.2 LE Central and Peripheral roles (QD ID 25340). For information on MFi/iAP2 support, please <a href="mailto:contact@bluekitchen-gmbh.com">contact us</a>.




## Evaluation Platforms

#### Embedded Platforms:      
Status               | Platform
--------------       | ------ 
[<img src="http://buildbot.bluekitchen-gmbh.com/btstack/badge.png?builder=port-ez430-rf2560-develop">](https://buildbot.bluekitchen-gmbh.com/btstack/builders/port-ez430-rf2560-develop)                       | [EZ430-RF256x Bluetooth Evaluation Tool for MSP430](http://www.ti.com/tool/ez430-rf256x)
[<img src="http://buildbot.bluekitchen-gmbh.com/btstack/badge.png?builder=port-msp-exp430f5438-cc2564b-develop">](https://buildbot.bluekitchen-gmbh.com/btstack/builders/port-msp-exp430f5438-cc2564b-develop) | [MSP430F5438 Experimenter Board for MSP430](http://www.ti.com/tool/msp-exp430f5438) with [Bluetooth CC2564 Module Evaluation Board](http://www.ti.com/tool/cc2564modnem)
[<img src="http://buildbot.bluekitchen-gmbh.com/btstack/badge.png?builder=port-msp430f5229lp-cc2564b-develop">](https://buildbot.bluekitchen-gmbh.com/btstack/builders/port-msp430f5229lp-cc2564b-develop)     | [MSP-EXP430F5529LP LaunchPad](http://www.ti.com/ww/en/launchpad/launchpads-msp430-msp-exp430f5529lp.html#tabs) with [Bluetooth CC2564 Module Evaluation Board](http://www.ti.com/tool/cc2564modnem) and [EM Adapter BoosterPack](http://www.ti.com/tool/boost-ccemadapter) with additional 32768Hz quartz oscillator
[<img src="http://buildbot.bluekitchen-gmbh.com/btstack/badge.png?builder=port-stm32-f103rb-nucleo-develop">](https://buildbot.bluekitchen-gmbh.com/btstack/builders/port-stm32-f103rb-nucleo-develop)         | [STM32 Nucleo development board NUCLEO-F103RB](http://www.st.com/web/catalog/tools/FM116/SC959/SS1532/LN1847/PF259875) with [Bluetooth CC2564 Module Evaluation Board](http://www.ti.com/tool/cc2564modnem) and [EM Adapter BoosterPack](http://www.ti.com/tool/boost-ccemadapter) with additional 32768Hz quartz oscillator
[<img src="http://buildbot.bluekitchen-gmbh.com/btstack/badge.png?builder=port-pic32-harmony-develop">](https://buildbot.bluekitchen-gmbh.com/btstack/builders/port-pic32-harmony-develop)                     | [Microchip's PIC32 Bluetooth Audio Development Kit](http://www.microchip.com/Developmenttools/ProductDetails.aspx?PartNO=DV320032)
[<img src="http://buildbot.bluekitchen-gmbh.com/btstack/badge.png?builder=port-wiced-develop">](https://buildbot.bluekitchen-gmbh.com/btstack/builders/port-wiced-develop)                                     | [RedBear Duo](https://github.com/redbear/WICED-SDK) with Broadcom BCM43438 A1


#### Other Platforms:     
Status               | Platform
--------------       | ------ 
[<img src="http://buildbot.bluekitchen-gmbh.com/btstack/badge.png?builder=port-posix-h4-develop">](https://buildbot.bluekitchen-gmbh.com/btstack/builders/port-posix-h4-develop) | posix: Unix-based system connected to Bluetooth module via serial port   
[<img src="http://buildbot.bluekitchen-gmbh.com/btstack/badge.png?builder=port-libusb-develop">](https://buildbot.bluekitchen-gmbh.com/btstack/builders/port-libusb-develop)     | libusb: Unix-based system with dedicated USB Bluetooth dongle
.. | windows-h4: Win32-based system connected to Bluetooth module via serial port   
.. | windows-winusb: Win32-based system with dedicated USB Bluetooth dongle
[<img src="http://buildbot.bluekitchen-gmbh.com/btstack/badge.png?builder=port-daemon-develop">](https://buildbot.bluekitchen-gmbh.com/btstack/builders/port-daemon-develop)     | daemon: TCP and Unix domain named socket client-server architecture supporting multiple clients
[<img src="http://buildbot.bluekitchen-gmbh.com/btstack/badge.png?builder=java-develop">](https://buildbot.bluekitchen-gmbh.com/btstack/builders/java-develop)                   | java: Java wrapper for daemon 
[<img src="http://buildbot.bluekitchen-gmbh.com/btstack/badge.png?builder=port-ios-develop">](https://buildbot.bluekitchen-gmbh.com/btstack/builders/port-ios-develop)           | iOS: daemon for iOS jailbreak devices, C client-server API
[<img src="http://buildbot.bluekitchen-gmbh.com/btstack/badge.png?builder=port-mtk-develop">](https://buildbot.bluekitchen-gmbh.com/btstack/builders/port-mtk-develop)           | mtk: daemon for rooted Android devices, based on Mediatek MT65xx processor, Java and C client-server API
[<img src="http://buildbot.bluekitchen-gmbh.com/btstack/badge.png?builder=port-wiced-develop">](https://buildbot.bluekitchen-gmbh.com/btstack/builders/port-wiced-develop)       | wiced: Broadcom platforms that support the WICED SDK

## Supported Chipsets

Chipset              | Type      | HCI Transport   | SCO over HCI (2) | BTstack folder | Comment 
-------------------- |-----------| ----------------|------------------|----------------|---------
Broadcom UART        | Dual mode | H4, H5          | Probably         | bcm            | Max UART baudrate 2 mbps
Broadcom USB Dongles | Dual mode | USB             | Yes              | bcm            | 
CSR UART             | Dual mode | H4, H5          | No (didn't work) | csr            | 
CSR USB Dongles      | Dual mode | USB             | Yes              | csr            |
EM 9301              | LE        | SPI             | n.a.             | em9301         | Custom HCI SPI implementation
Nordic nRF           | LE        | H4              | n.a.             |                | Requires custom HCI firmware
STM STLC2500D        | Classic   | H4              | No (didn't try)  | stlc2500d      | Custom deep sleep management not supported
Toshiba TC35661      | Dual mode | H4              | No (didn't try)  | tc3566         | HCI version not tested.
TI CC256x, WL183x    | Dual mode | H4, H5, eHCILL  | Yes              | cc256x         | Also WL185x, WL187x, and WL189x

[More infos on supported chipsets](https://bluekitchen-gmbh.com/btstack/develop/chipsets/)

## Source Tree Overview
Path				| Description
--------------------|---------------
chipset             | Support for individual Bluetooth chipsets
doc                 | Sources for BTstack documentation
example             | Example applications available for all ports
platform            | Support for special OSs and/or MCU architectures
port                | Complete port for a MCU + Chipset combinations
src                 | Bluetooth stack implementation
test                | Unit and PTS tests
tool                | Helper tools for BTstack
