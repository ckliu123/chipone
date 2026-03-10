hdc target mount
timeout 1
hdc shell mount -o rw,remount /
::hdc shell mount -o rw,remount /dev
hdc shell mount -o rw,remount /vendor
hdc shell mount -o rw,remount /chip_prod
hdc shell setenforce 0
hdc shell rm /chip_prod/etc/firmware/ts/libafehalP471B72900.so
hdc shell rm /chip_prod/etc/firmware/ts/icnt9288.bin
hdc file send .\libafehalP471B72900.so /chip_prod/etc/firmware/ts/libafehalP471B72900.so

timeout 2
hdc shell tpd reboot
timeout 4
hdc shell tpd version

pause