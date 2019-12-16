#!/bin/bash

#this script is part of mx-tweak 

main()
{
$function
}


resolutions()
{
for D in $display; do 
    xrandr | sed -nr "/^$D/,/^[^[:space:]]/s/^[[:space:]]*([0-9]+x[0-9]+).*/\1/p"
done

}

refreshrate()
{

LANG=C xrandr | awk "/$display/{flag=1;next}/connected/{flag=0}flag" |grep \*

}

gamma()
{
    LANG=C xrandr --verbose | awk "/$display/{flag=1;next}/CONNECTOR_ID/{flag=0}flag"|grep Gamma
}

display=$1
function=$2

main

exit 0
