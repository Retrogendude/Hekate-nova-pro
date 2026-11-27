# Hekate Nova Pro v34 alpha 4

修复分区前保存 emuMMC/emummc.ini 的误报：按原始字节写入，不自动转换换行。保留同步和回读验证。已用真实 Nyx FatFs 在虚拟 FAT32/exFAT 上重现故障并验证修复。不完整写入和写保护仍会阻止操作。保留 alpha 3 界面与功能，仍需实体主机验证。

PC checks passed; physical console validation pending.
