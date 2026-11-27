# Hekate Nova Pro v34 alpha 4

Fix false emuMMC/emummc.ini save failure before partitioning: write exact bytes without automatic newline conversion. Sync and readback remain enforced. Reproduced the defect and tested the fix with actual Nyx FatFs on virtual FAT32/exFAT. Short writes and write protection still rejected. Alpha 3 UI and features retained; console validation pending.

PC checks passed; physical console validation pending.
