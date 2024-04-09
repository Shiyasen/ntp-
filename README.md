# ntp-

启动脚本：

#!/bin/bash

systemctl disable systemd-timesyncd
timedatectl set-timezone Asia/Shanghai
# 设置需要执行的程序路径
program_path="/home/jetson/ntp/build-ntp-ocr-Release/NtpClient"

# 设置程序需要的参数  
#参数构成 <IP> <PORT> <TIME>
program_args="10.168.2.100 123 60"

# 执行程序
$program_path $program_args

 

# 退出脚本
exit 0
