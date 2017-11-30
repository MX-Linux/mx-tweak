#!/bin/bash

#this app is part of mx-tweak for enabling hibernate
#this will update the initramfs of all kernels with the resume swap parition

log=/var/log/tweak.log

echo "" |tee -a $log
date |tee -a $log
update-initramfs -u -k all 2>&1 |tee -a $log
echo "See /var/log/tweak.log if there are problems"
sleep 3

exit 0
