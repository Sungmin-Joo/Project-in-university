mount -t nfs 192.168.1.70:/nfsroot /mnt/nfs -o tcp,nolock

insmod fpga_text_lcd_driver.ko
mknod /dev/fpga_text_lcd c 263 0
insmod fpga_push_switch_driver.ko
mknod /dev/fpga_push_switch c 265 0
insmod fpga_led_driver.ko
mknod /dev/fpga_led c 260 0

