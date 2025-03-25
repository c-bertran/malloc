#!/bin/bash

rm -rf out gnl

gnl_test () {
	clang -Wall -Werror -Wextra -D BUFFER_SIZE=$1 gnl.c main.c -I ./ -o gnl || exit 100;
	# REAL =====================================================================
	./gnl files/41_no_nl >> out
	./gnl files/41_with_nl >> out
	./gnl files/42_no_nl >> out
	./gnl files/42_with_nl >> out
	./gnl files/43_no_nl >> out
	./gnl files/43_with_nl >> out
	./gnl files/alternate_line_nl_no_nl >> out
	./gnl files/alternate_line_nl_with_nl >> out
	./gnl files/big_line_no_nl >> out
	./gnl files/big_line_with_nl >> out
	./gnl files/empty >> out
	./gnl files/multiple_line_no_nl >> out
	./gnl files/multiple_line_with_nl >> out
	./gnl files/multiple_nlx5 >> out
	./gnl files/nl >> out
}

gnl_test -1 $1
rm -rf gnl

gnl_test 0 $1
rm -rf gnl

gnl_test 1 $1
rm -rf gnl

gnl_test 215600 $1
rm -rf gnl
