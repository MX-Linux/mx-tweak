#!/bin/bash

main()
{
$CMD1
}


case $1 in
	"true") CMD1="kwriteconfig6 --file /root/.config/kdeglobals --group KDE --key SingleClick true"
	
	"false") CMD1="kwriteconfig6 --file /root/.config/kdeglobals --group KDE --key SingleClick false"
	;;
	*)CMD1="";;
esac

echo $CMD1
main

exit 0
