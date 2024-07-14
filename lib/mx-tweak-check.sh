#!/bin/bash
test=$(passwd --status root|cut -d' ' -f2)
if [ "$test" = "NP" ] || [ "$test" = "L" ]; then
	echo "NP"
fi
