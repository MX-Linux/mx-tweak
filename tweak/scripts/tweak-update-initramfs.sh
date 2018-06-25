#!/bin/bash

#this app is part of mx-tweak 
#the purpose is to enable hibernate function
#this will update the initramfs of all kernels with the resume swap parition

log=/var/log/tweak.log

echo "" |tee -a $log
date |tee -a $log
echo "" |tee -a $log
    
if [ -e /etc/uswsusp.conf ]; then
    echo "" |tee -a $log
    date |tee -a $log
    echo "" |tee -a $log
    res_swap=$(blkid -s TYPE -s UUID |grep swap |cut -d\" -f2)
    echo "res_swap is " $res_swap |tee -a $log
    echo ""
    sed -i s/resume.*/resume\ device\ =\ \\/dev\\/disk\\/by-uuid\\/$res_swap/ /etc/uswsusp.conf
    update-initramfs -u -k all |tee -a $log
    echo "" |tee -a $log
    cat /etc/uswsusp.conf |tee -a $log
    echo ""
else
    echo "/etc/uswsusp.conf not found" |tee -a $log
    exit 1
fi

echo "See /var/log/tweak.log if there are problems"

sleep 3

exit 0
