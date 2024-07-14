#!/bin/bash
if [ -f /etc/polkit-1/rules.d/10-default-mx.rules ]; then
	exit 0
else
	exit 1
fi
