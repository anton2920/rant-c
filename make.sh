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
	prof)
		run cc -o $PROJECT -O3 -mavx2 -fno-omit-frame-pointer $CFLAGS $LDFLAGS $SRC
		;;
	release)
		run cc -o $PROJECT -O3 -mavx2 $CFLAGS $LDFLAGS $SRC
		;;
	vet)
		if which clang-tidy >/dev/null; then
			CHECKS='-*,clang-analyzer-*,-clang-analyzer-cplusplus*,-clang-analyzer-security.insecureAPI*,performance*,-performance-no-int-to-ptr -header-filter=.*'
			echo -n $CFLAGS $LDFLAGS | sed 's/ /\n/g' >compile_flags.txt
			run clang-tidy -warnings-as-errors=$CHECKS -system-headers *.c
			rm -f compile_flags.txt
		fi
		;;
esac

ENDTIME=`date +%s`

echo Done $1 in $((ENDTIME-STARTTIME))s
