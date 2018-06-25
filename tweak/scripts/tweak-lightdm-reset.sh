#!/bin/bash

#this script is part of mx-tweak 
#the purpose is to reset the lightdm theme to current system defaults

cp /etc/lightdm/lightdm-gtk-greeter.conf /etc/lightdm/lightdm-gtk-greeter.conf.$(date +%Y%m%H%M%S)
cp /etc/lightdm/mx$(lsb_release -rs)/lightdm-gtk-greeter.conf /etc/lightdm/lightdm-gtk-greeter.conf


exit 0
