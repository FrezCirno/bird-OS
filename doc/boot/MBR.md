# 主引导记录MBR (引导扇区)

主引导记录(Master Boot Record, MBR)，用来引导系统的启动。

计算机在启动时首先会加载BIOS(basic input/output system, 基本输入输出系统)，进行机器的自检，读取CMOS中的各项硬件参数，然后会依据用户配置的顺序去找寻能够启动的设备，例如启动光盘，启动U盘，硬盘等。这些设备能否启动，就是看其第一个扇区是否存在MBR。

以硬盘为例，在硬盘的0柱面，0磁道，1扇区的位置(即硬盘开头, 扇区编号从1开始)，512字节的扇区有三部分内容：

1. MBR，446字节。

2. DPT(Disk Partition Table)，硬盘分区表，64字节。

3. 结束标志，2字节，内容为55AA，主引导扇区是否有效的标志。


## 标准引导扇区结构
| 地址 |      |     | 描述                               | 长度（字节）   |
|------|------|-----|------------------------------------|----------------|
| Hex  | Oct  | Dec |                                    |                |
| 0000 | 0000 | 0   | 代码区                             | 440 (最大 446) |
| 01B8 | 0670 | 440 | 可选磁盘标志                       | 4              |
| 01BC | 0674 | 444 | 一般为空值; 0x0000                 | 2              |
| 01BE | 0676 | 446 | 分区表 (四个16 byte的主分区表入口) | 64             |
| 01FE | 0776 | 510 | 55h                                | 1              |
| 01FF | 0777 | 511 | AAh                                | 1              |
|      |      |     | 总大小                             | 512            |

## 主引导扇区的读取流程

- 系统开机或者重启。

1. BIOS加电（台湾用语：引导）自检（Power On Self Test -- POST）。BIOS执行内存地址为FFFF:0000H处的跳转指令，跳转到固化在ROM中的自检程序处，对系统硬件（包括内存）进行检查。
2. 读取主引导记录（MBR）。当BIOS检查到硬件正常并与CMOS中的设置相符后，按照CMOS中对启动设备的设置顺序检测可用的启动设备。**BIOS将相应启动设备的第一个扇区（也就是MBR扇区）读入内存地址为0000:7C00H处。**
3. 检查0000:7DFEH-0000:7DFFH（MBR的结束标志位）是否等于55AAH，若不等于则转去尝试其他启动设备，如果没有启动设备满足要求则显示"NO ROM BASIC"然后死机。
4. 当检测到有启动设备满足要求后，BIOS将控制权交给相应启动设备。启动设备的MBR将自己复制到0000:0600H处，然后继续执行。
5. 根据MBR中的引导代码启动引导程序。

- 事实上，BIOS不仅检查0000:7DFEH-0000:7DFFH（MBR的结束标志位）是否等于55AAH，往往还对磁盘是否有写保护、主引导扇区中是否存在活动分区等进行检查。如果发现磁盘有写保护，则显示磁盘写保护出错信息；如果发现磁盘中不存在活动分区，则显示类似如下的信息“Remove disk or other media Press any key to restart”。