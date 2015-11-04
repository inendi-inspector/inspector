#!/bin/sh
#
# @file
#
# @copyright (C) Picviz Labs 2010-March 2015
# @copyright (C) ESI Group INENDI April 2015-2015

LOG="log.$$"

#printf "%-14s %12s %20s %20s\n" "Algorithm" "Count" "Time (ms)" "Memory (MiB)"
for COUNT in 1000 100000 1000000 100000000 ; do
    echo "# COUNT = $COUNT"
    for ALGO in 0 1 2 ; do
	./create_quadtree $COUNT $ALGO
# > $LOG
#	WHAT=`grep "Input" $LOG | cut -d\  -f 1 | sed -e 's/://g'`
#	TIME=`grep "Input" $LOG | sed -e 's/^.*in \([^ ]*\).*$/\1/'`
#	MEM=`grep "memory usage" $LOG | sed -e 's/^.*: \([^ ]*\).*$/\1/'`
#	printf "%-14s %12d %20g %20g\n" $WHAT $COUNT $TIME $MEM
    done
done

rm -f "$LOG"