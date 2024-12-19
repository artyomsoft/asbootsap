# asbootsap

_asbootsap_ is a simple UEFI-based bootloader which supports serveral Linux boot protocols

Currently it supports

- chainload;
- EFI Handover;
- 64-bit Linux boot protocol.

Before the first build please ensure that you install required dependencies:

```
apt install -y build-essential gnu-efi nasm
```

To build:

```
make -C src all
```

To clean build:

```
make -C src clean
```

To install _asbootsap_ bootloader you need to place it as BOOTX64.EFI in the \EFI\BOOT directory of ESP partition on flash drive with Linux. Also you need to place vmlinuz and initrd.img images of your Linux and asbootsap.cfg file to ESP partition.

Sample asbootsap.cfg

```
#
# protocol can be efi_chainload, efi_handover, linux64 or can be ommited at all
# in case of ommited protocol menu for selecting boot protocol appears.
#
protocol efi_chainload
linux \vmlinuz
initrd \initrd.img
cmdline root=UUID=7c1d6f3a-efeb-44b1-bc44-2274e138d1de
```
