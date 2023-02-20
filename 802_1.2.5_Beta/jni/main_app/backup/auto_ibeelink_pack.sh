##########################################################
# Copyright:    www.ibeelink.com 
# author:       aiden.xiang@ibeelink.com
# data:         2020.05.30
##########################################################

#! /bin/bash
expect -c "
    spawn make image-ota
    expect {
        \"Start scripts:\" {send \"start.sh\r\"; exp_continue}
        \"End scripts:\" {send \"end.sh\r\"; exp_continue}
        \"Make ipl ?(yes/no)\" {send \"no\r\"; exp_continue}
        \"Make ipl_cust ?(yes/no)\" {send \"no\r\"; exp_continue}
        \"Make uboot ?(yes/no)\" {send \"no\r\"; exp_continue}
        \"Make logo ?(yes/no)\" {send \"no\r\"; exp_continue}
        \"Make kernel ?(yes/no)\" {send \"no\r\"; exp_continue}
        \"Make miservice ?(yes/no)\" {send \"no\r\"; exp_continue}
        \"Make customer ?(yes/no)\" {send \"yes\r\"; exp_continue}
        \"Make appconfigs ?(yes/no)\" {send \"no\r\"; exp_continue}
        \"overwrite (y or n)?\" {send \"y\r\"; exp_continue}
    }"
echo "============================================================="
echo "Congratulations, auto pack success !"
echo "============================================================="
