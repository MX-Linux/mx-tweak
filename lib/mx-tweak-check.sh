#!/bin/bash
if [ -n "$@" ]; then
    echo "no arguements accepted"
	exit 2
fi
test=$(passwd --status root|cut -d' ' -f2)
if [ "$test" = "NP" ] || [ "$test" = "L" ]; then
	echo "NP"
fi
