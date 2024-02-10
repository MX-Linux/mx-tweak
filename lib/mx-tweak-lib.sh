#!/bin/bash

#this script is part of mx-tweak 

#intel scripts
#the purpose is to enable/disable intel driver override
enable_intel()
{
    cp /usr/share/mx-tweak/20-intel.conf /etc/X11/xorg.conf.d/20-intel.conf
}

disable_intel()
{
    rm /etc/X11/xorg.conf.d/20-intel.conf
    
}

enable_radeon()
{
    cp /usr/share/mx-tweak/20-radeon.conf /etc/X11/xorg.conf.d/20-radeon.conf
}

disable_radeon()
{
    rm /etc/X11/xorg.conf.d/20-radeon.conf
    
}

enable_amd()
{
    cp /usr/share/mx-tweak/20-amd.conf /etc/X11/xorg.conf.d/20-amd.conf
}

disable_amd()
{
    rm /etc/X11/xorg.conf.d/20-amd.conf
    
}

enable_libinput_touchpad()
{
    cp /usr/share/mx-tweak/30-touchpad.conf /etc/X11/xorg.conf.d/30-touchpad.conf
}

disable_libinput_touchpad()
{
    rm /etc/X11/xorg.conf.d/30-touchpad.conf
    
}

enable_bluetooth()
{
sed -i 's/^AutoEnable=.*/AutoEnable=true/' /etc/bluetooth/main.conf
}

disable_bluetooth()
{
sed -i 's/^AutoEnable=.*/AutoEnable=false/' /etc/bluetooth/main.conf
}

install_recommends()
{
local file
file="/etc/apt/apt.conf"

if [ -e "$file" ]; then
	if [ -n "$(grep "Install-Recommends" "$file")" ]; then
		sed -i 's/APT\:\:Install-Recommends .*/APT\:\:Install-Recommends "1";/' "$file"
	else
		echo "APT::Install-Recommends \"1\";" >> "$file"
	fi
else
	echo "APT::Install-Recommends \"1\";" >> "$file"
fi
}

noinstall_recommends()
{
local file
file="/etc/apt/apt.conf"

if [ -e "$file" ]; then
	if [ -n "$(grep "Install-Recommends" "$file")" ]; then
		sed -i 's/APT\:\:Install-Recommends .*/APT\:\:Install-Recommends "0";/' "$file"
	else
		echo "APT::Install-Recommends \"0\";" >> "$file"
	fi
else
	echo "APT::Install-Recommends \"0\";" >> "$file"
fi
}

#lightdm
#the purpose is to reset the lightdm theme to current system defaults
lightdm_reset()
{
cp /etc/lightdm/lightdm-gtk-greeter.conf /etc/lightdm/lightdm-gtk-greeter.conf.$(date +%Y%m%H%M%S)
cp /etc/lightdm/mx$(grep DISTRIB_RELEASE /etc/lsb-release |cut -d\= -f2|cut -d\. -f1)/lightdm-gtk-greeter.conf /etc/lightdm/lightdm-gtk-greeter.conf

}

#the purpose is to enable/disable user mounting of internal devices
enable_user_mount()
{
    if [ -d /etc/polkit-1/rules.d]; then
    echo "/etc/polkit-1/rules.d found"
    else
    mkdir -p /etc/polkit-1/rules.d
    fi
    
    cp /usr/share/mx-tweak/10-udisks2.rules /etc/polkit-1/rules.d/10-udisks2.rules 
    touch /etc/tweak-udisks.chk
}

disable_user_mount()
{
    rm -f /etc/polkit-1/rules.d/10-udisks2.rules 
    rm -f /etc/tweak-udisks.chk
    
}


#the purpose is to enable hibernate function
#this will update the initramfs of all kernels with the resume swap parition

hibernate()
{
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
}

enable_sudo_override()
{

    rm -f /etc/polkit-1/rules.d/10-default-mx.rules
    
}

disable_sudo_override()
{
    if [ -d /etc/polkit-1/rules.d/ ]; then
    echo "/etc/polkit-1/rules.d found"
    else
    mkdir -p /etc/polkit-1/rules.d
    fi
    
    cp /usr/share/mx-tweak/10-default-mx.rules /etc/polkit-1/rules.d

}

enable_sandbox()
{
	if [ ! -d /etc/sysctl.d/ ]; then
		mkdir -p /etc/systclt.d
	fi
	sysctl kernel.unprivileged_userns_clone=1
	#sysctl kernel.yama.ptrace_scope=1
	echo "kernel.unprivileged_userns_clone=1"> /etc/sysctl.d/99-sandbox-mx.conf
	#echo "kernel.yama.ptrace_scope=1">>/etc/sysctl.d/99-sandbox-mx.conf
}

disable_sandbox()
{
	sysctl kernel.unprivileged_userns_clone=0
	#sysctl kernel.yama.ptrace_scope=0
	echo "kernel.unprivileged_userns_clone=0"> /etc/sysctl.d/99-sandbox-mx.conf
	#echo "kernel.yama.ptrace_scope=0">>/etc/sysctl.d/99-sandbox-mx.conf
}

hold_debian_kernel_updates()
{
if [ -n "$(LC_ALL=C dpkg --status linux-image-686 2>/dev/null | grep "ok installed")" ]; then
	echo "found linux-image-686"
	apt-mark hold linux-image-686 linux-headers-686  2>/dev/null
fi
if [ -n "$(LC_ALL=C dpkg --status linux-image-686-pae 2>/dev/null| grep "ok installed")" ]; then
	echo "found linux-image-686-pae"
	apt-mark hold linux-image-686-pae linux-headers-686-pae 2>/dev/null
fi
if [ -n "$(LC_ALL=C dpkg --status linux-image-amd64 2>/dev/null| grep "ok installed")" ]; then
	echo "found linux-image-amd64"
	apt-mark hold linux-image-amd64 linux-headers-amd64 2>/dev/null
fi
}
unhold_debian_kernel_updates(){
if [ -n "$(LC_ALL=C dpkg --status linux-image-686 2>/dev/null| grep "ok installed")" ]; then
	echo "found linux-image-686"
	apt-mark unhold linux-image-686 linux-headers-686 2>/dev/null
fi
if [ -n "$(LC_ALL=C dpkg --status linux-image-686-pae 2>/dev/null| grep "ok installed")" ]; then
	echo "found linux-image-686-pae"
	apt-mark unhold linux-image-686-pae linux-headers-686-pae 2>/dev/null
fi
if [ -n "$(LC_ALL=C dpkg --status linux-image-amd64 2>/dev/null| grep "ok installed")" ]; then
	echo "found linux-image-amd64"
	apt-mark unhold linux-image-amd64 linux-headers-amd64 2>/dev/null
fi

}
hold_liquorix_kernel_updates()
{
if [ -n "$(LC_ALL=C dpkg --status linux-image-liquorix-amd64 2>/dev/null| grep "ok installed")" ]; then
	echo "found linux-image-liquorix-amd64"
	apt-mark hold linux-image-liquorix-amd64 linux-headers-liquorix-amd64 2>/dev/null
fi
}
unhold_liquroix_kernel_updates()
{
if [ -n "$(LC_ALL=C dpkg --status linux-image-liquorix-amd64 2>/dev/null| grep "ok installed")"]; then
	echo "found linux-image-liquorix-amd64"
	apt-mark unhold linux-image-liquorix-amd64 linux-headers-liquorix-amd64 2>/dev/null
fi
}

main()
{
$CMD1
$CMD2
$CMD3
$CMD4
$CMD5
$CMD6
$CMD7
$CMD8
$CMD9

}

CMD1=$1
CMD2=$2
CMD3=$3
CMD4=$4
CMD5=$5
CMD6=$6
CMD7=$7
CMD8=$8
CMD9=$9

main

exit 0
