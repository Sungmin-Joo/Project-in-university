mount -t nfs 192.168.1.70:/nfsroot /mnt/nfs -o tcp,nolock

insmod fpga_text_lcd_driver.ko
mknod /dev/fpga_text_lcd c 263 0
insmod fpga_push_switch_driver.ko
mknod /dev/fpga_push_switch c 265 0
insmod up_down_pwm.ko
insmod left_right_pwm.ko
mknod /dev/up_down_pwm c 246 0
mknod /dev/left_right_pwm c 245 0
insmod hc-sr04_driver.ko
mknod /dev/us c 244 0
insmod fpga_buzzer_driver.ko
mknod /dev/fpga_buzzer c 264 0
insmod fpga_fnd_driver.ko
mknod /dev/fpga_fnd c 261 0
insmod fpga_dot_driver_arrow.ko
mknod /dev/fpga_dot c 262 0


