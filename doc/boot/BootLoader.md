# 引导程序BootLoader

引导程序（英语：boot loader）是指引导操作系统的程序。在普通的个人电脑上，引导程序通常分为两部分：第一阶段引导程序位于主引导记录（MBR），用以引导位于某个分区上的第二阶段引导程序，如NTLDR、BOOTMGR和GNU GRUB等。

BIOS引导完成后，bootloader就接手初始化硬件设备、创建存储器空间的映射，以便为操作系统内核准备好正确的软硬件环境。

过去的BootLoader较小, 能够放在MBR中, 但是随着计算机操作系统越来越复杂，位于主引导记录的空间已经放不下引导操作系统的代码，于是就有了第二阶段的引导程序，而MBR中代码的功能也从直接引导操作系统变为了引导第二阶段的引导程序。

对于UEFI系统，由EFI应用程序（即EFI系统分区中的.efi文件）取代MBR和第二阶段引导程序，UEFI固件会加载引导程序的.efi文件，再由引导程序加载操作系统。 

## BootLoader作用
1. 提供菜单项，是用户可以选择不同的启动项目，譬如可以显示多个操作系统供用户选择启动。
2. 加载内核文件，用来直接启动操作系统
3. 转交启动管理功能给其他loader。