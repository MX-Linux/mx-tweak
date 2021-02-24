#!/bin/bash
test=$(passwd --status root|cut -d' ' -f2)
if [ "$test" = "NP" ]; then
	echo $test
fi
