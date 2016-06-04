#!/bin/bash

# main.command
# Sort Items

PATH=/bin:/usr/bin:/sbin:/usr/sbin export PATH

case "$sortDirection" in
0)	dir=	;;
1)	dir="-r"	;;
esac

case "$sortStyle" in
0)	s=	;;
1)	s="-n"	;;
esac

# note when you use "-k", the first column is 1, not 0

sort $dir $s -k $sortColumn

exit 0