#!/bin/bash
if [ -n "$@" ]; then
    echo "no arguements accepted"
	exit 2
fi
if [ -f /etc/polkit-1/rules.d/10-default-mx.rules ]; then
	exit 0
else
	exit 1
fi
