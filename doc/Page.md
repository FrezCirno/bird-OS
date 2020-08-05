# x86分页机制

![](https://chyyuu.gitbooks.io/simple_os_book/content/zh/chapter-3/figures/3.png)

二级页表 - PDE

一级页表 - PTE



----------------------------------------------------------------------------
 分页机制使用的常量说明
----------------------------------------------------------------------------
PG_P		EQU	1	 页存在属性位
PG_RWR		EQU	0	 R/W 属性位值, 读/执行
PG_RWW		EQU	2	 R/W 属性位值, 读/写/执行
PG_USS		EQU	0	 U/S 属性位值, 系统级
PG_USU		EQU	4	 U/S 属性位值, 用户级
----------------------------------------------------------------------------


 =========================================
 FLAGS - Intel 8086 Family Flags Register
 =========================================

      |11|10|F|E|D|C|B|A|9|8|7|6|5|4|3|2|1|0|
        |  | | | | | | | | | | | | | | | | '---  CF……Carry Flag
        |  | | | | | | | | | | | | | | | '---  1
        |  | | | | | | | | | | | | | | '---  PF……Parity Flag
        |  | | | | | | | | | | | | | '---  0
        |  | | | | | | | | | | | | '---  AF……Auxiliary Flag
        |  | | | | | | | | | | | '---  0
        |  | | | | | | | | | | '---  ZF……Zero Flag
        |  | | | | | | | | | '---  SF……Sign Flag
        |  | | | | | | | | '---  TF……Trap Flag  (Single Step)
        |  | | | | | | | '---  IF……Interrupt Flag
        |  | | | | | | '---  DF……Direction Flag
        |  | | | | | '---  OF……Overflow flag
        |  | | | '-----  IOPL……I/O Privilege Level  (286+ only)
        |  | | '-----  NT……Nested Task Flag  (286+ only)
        |  | '-----  0
        |  '-----  RF……Resume Flag (386+ only)
        '------  VM……Virtual Mode Flag (386+ only)

        注: see   PUSHF  POPF  STI  CLI  STD  CLD
