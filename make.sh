#!/bin/sh

PROJECT=rant

VERBOSITY=0
VERBOSITYFLAGS=""
while test "$1" = "-v"; do
	VERBOSITY=$((VERBOSITY+1))
	VERBOSITYFLAGS="$VERBOSITYFLAGS -v"
	shift
done

run()
{
	if test $VERBOSITY -gt 1; then echo "$@"; fi
	"$@" || exit 1
}

printv()
{
	if test $VERBOSITY -gt 0; then echo "$@"; fi
}

CFLAGS="-Wall -Wextra -Wpedantic -Werror -ansi -std=c89"
LDFLAGS="-static -nostdlib"

SRC=`ls *.c`

STARTTIME=`date +%s`

case $1 in
	'' | debug)
		run cc -o $PROJECT -O0 -g $CFLAGS $LDFLAGS $SRC
		;;
	clean)
		run rm -f $PROJECT *.o
		;;
	disas | disasm)
		for file in $SRC; do
			run cc -o $file.disas -O0 -S $CFLAGS $LDFLAGS $file
		done
		cat *.disas >$PROJECT.s
		rm -f *.disas
		;;
	fmt)
		if which cb >/dev/null; then
			for file in $SRC; do
				run cb -s $file >reformatted
				run cp reformatted $file
			done
		fi
		;;
	 release)
		run cc -o $PROJECT -O3 $CFLAGS $LDFLAGS $SRC
		;;
esac

ENDTIME=`date +%s`

echo Done $1 in $((ENDTIME-STARTTIME))s
