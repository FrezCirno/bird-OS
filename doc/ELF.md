# ELF 可执行文件格式

## 总览

![](https://chuquan-public-r-001.oss-cn-shanghai.aliyuncs.com/sketch-images/elf-file-format.png?x-oss-process=image/resize,w_600)

## 结构

| 结构     | Structure            |
| -------- | -------------------- |
| ELF 头   | ELF header           |
| 程序头表 | Program header table |
| 节       | Sections             |
| 节头表   | Section header table |

## ELF 文件类型

- **可重定位文件（Relocatable File）**：ETL_REL。一般为.o 文件。可以被链接成可执行文件或共享目标文件。静态链接库属于可重定位文件。
- **可执行文件（Executable File）**：ET_EXEC。可以直接执行的程序。
- **共享目标文件（Shared Object File）**：ET_DYN。一般为.so 文件。有两种情况可以使用。
  - 链接器将其与其他可重定位文件、共享目标文件链接成新的目标文件；
  - 动态链接器将其与其他共享目标文件、结合一个可执行文件，创建进程映像。
